#include "D3D11RenderEngine.h"
#include "D3D11CommonTypes.h"

SG::D3D11RenderEngine::D3D11RenderEngine(const SGRenderSettings & settings) : SGRenderEngine(settings)
{
	this->CreateDeviceAndContext(settings);
	bufferHandler = new D3D11BufferHandler(device);
	//samplerHandler;
	shaderManager = new D3D11ShaderManager(device);
	//stateHandler;
	this->textureHandler = new D3D11TextureHandler(device);
	this->pipelineManager = new D3D11PipelineManager(device);
	this->CreateSwapChain(settings);
}

SG::D3D11RenderEngine::~D3D11RenderEngine()
{
	engineActive = false;
	while (renderthreadActive)
	{
		// Spinwait
	}

	ReleaseCOM(device);
	ReleaseCOM(immediateContext);
	ReleaseCOM(swapChain);

	for (auto& context : defferedContexts)
		ReleaseCOM(context);

	delete bufferHandler;
	delete shaderManager;
	delete textureHandler;
	delete pipelineManager;
}

SG::D3D11BufferHandler * SG::D3D11RenderEngine::BufferHandler()
{
	return bufferHandler;
}

SG::D3D11ShaderManager * SG::D3D11RenderEngine::ShaderManager()
{
	return shaderManager;
}

SG::D3D11TextureHandler * SG::D3D11RenderEngine::TextureHandler()
{
	return textureHandler;
}

SG::D3D11PipelineManager * SG::D3D11RenderEngine::PipelineManager()
{
	return pipelineManager;
}

void SG::D3D11RenderEngine::CreateDeviceAndContext(const SGRenderSettings & settings)
{
	UINT flags = 0;

	if constexpr (DEBUG_VERSION)
		flags = D3D11_CREATE_DEVICE_DEBUG;

	//if (settings.nrOfContexts <= 1)
	//	flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

	if (FAILED(D3D11CreateDevice(settings.adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &device, NULL, &immediateContext)))
		throw std::runtime_error("Error creating device and immediate context");

	for (int i = 0; i < (settings.nrOfContexts >= 1 ? settings.nrOfContexts : 1); ++i)
	{
		ID3D11DeviceContext* defferedContext;
		if(FAILED(device->CreateDeferredContext(0, &defferedContext)))
			throw std::runtime_error("Error creating deffered context");

		defferedContexts.push_back(defferedContext);
	}
}

void SG::D3D11RenderEngine::CreateSwapChain(const SGRenderSettings & settings)
{
	DXGI_MODE_DESC bufferDesc;
	bufferDesc.Width = settings.backBufferSettings.width;
	bufferDesc.Height = settings.backBufferSettings.height;
	bufferDesc.RefreshRate.Numerator = settings.backBufferSettings.refreshRate;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = settings.backBufferSettings.format;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SAMPLE_DESC sampleDesc;
	sampleDesc.Count = settings.backBufferSettings.multiSampleCount;
	sampleDesc.Quality = settings.backBufferSettings.multiSampleQuality;

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	swapchainDesc.BufferDesc = bufferDesc;
	swapchainDesc.SampleDesc = sampleDesc;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = settings.backBufferSettings.nrOfBackBuffers;
	swapchainDesc.OutputWindow = settings.windowHandle;
	swapchainDesc.Windowed = settings.backBufferSettings.windowedMode;
	swapchainDesc.SwapEffect = settings.backBufferSettings.swapEffect;
	swapchainDesc.Flags = settings.backBufferSettings.flags;

	IDXGIDevice* dxgiDevice = 0;
	if(FAILED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice)))
		throw std::runtime_error("Error querying IDXGIDevice from device");

	IDXGIAdapter* dxgiAdapter = 0;
	if (FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter)))
		throw std::runtime_error("Error getting adapter from IDXGIDevice");

	IDXGIFactory* dxgiFactory = 0;
	if (FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
		throw std::runtime_error("Error getting factory from adapter");

	if (FAILED(dxgiFactory->CreateSwapChain(device, &swapchainDesc, &swapChain)))
		throw std::runtime_error("Error creating swap chain");

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	for (int i = 0; i < settings.backBufferSettings.nrOfBackBuffers - 1; ++i)
	{
		ID3D11Texture2D* texture;
		swapChain->GetBuffer(i, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
		textureHandler->AddTexture2D(SGGuid(std::string("SG_BACKBUFFER_") + std::to_string(i)), texture);
	}
}

void SG::D3D11RenderEngine::SwapUpdateBuffer()
{
	bufferHandler->SwapUpdateBuffer();
	textureHandler->SwapUpdateBuffer();
}

void SG::D3D11RenderEngine::SwapToWorkWithBuffer()
{
	bufferHandler->SwapToWorkWithBuffer();
	textureHandler->SwapToWorkWithBuffer();
}

void SG::D3D11RenderEngine::ExecuteJobs(const std::vector<SGPipelineJob>& jobs)
{
	//Simple way of doing the updates, can be improved
	bufferHandler->UpdateBuffers(immediateContext);
	//textureHandler->UpdateTextures(immediateContext);

	unsigned int nrOfContexts = static_cast<unsigned int>(defferedContexts.size());
	unsigned int jobsPerContext = static_cast<unsigned int>(jobs.size() / nrOfContexts);
	size_t threadsToUse = (jobs.size() < nrOfContexts ? jobs.size() - 1 : nrOfContexts - 1);
	std::vector<SG::FunctionStatus> statuses(threadsToUse);


	for (int i = 0; i < static_cast<int>(threadsToUse); ++i)
	{
		auto temp = std::bind(&D3D11RenderEngine::HandlePipelineJobs, this, jobs, i * jobsPerContext, (i + 1) * jobsPerContext, defferedContexts[i]);
		threadPool->EnqueFunction(&statuses[i], temp);
	}

	HandlePipelineJobs(jobs, nrOfContexts - 1, static_cast<int>(jobs.size()), defferedContexts[threadsToUse]);

	for (size_t i = 0; i < threadsToUse; ++i)
	{
		while (statuses[i] != FunctionStatus::FINISHED)
		{
			// Spinwait
		}

		ID3D11CommandList* cmdList;
		
		if(FAILED(defferedContexts[i]->FinishCommandList(false, &cmdList)))
			throw std::runtime_error("Error finishing command list");

		immediateContext->ExecuteCommandList(cmdList, false);
		ReleaseCOM(cmdList);
	}

	ID3D11CommandList* cmdList;

	if (FAILED(defferedContexts[threadsToUse]->FinishCommandList(false, &cmdList)))
		throw std::runtime_error("Error finishing command list");

	immediateContext->ExecuteCommandList(cmdList, false);
	ReleaseCOM(cmdList);

	swapChain->Present(0, 0);
}

void SG::D3D11RenderEngine::HandlePipelineJobs(const std::vector<SGPipelineJob>& jobs, int startPos, int endPos, ID3D11DeviceContext * context)
{
	for (int i = startPos; i < endPos; ++i)
	{
		SGPipeline pipeline = pipelineManager->GetPipeline(jobs[i].pipelineGuid);

		for (auto& job : pipeline.jobs)
		{
			switch (job.first)
			{
			case PipelineJobType::RENDER:
				HandleRenderJob(pipelineManager->GetRenderJob(job.second), jobs[i].entitiesToRender, context);
				break;
			case PipelineJobType::CLEAR_RENDER_TARGET:
				HandleClearRenderTargetJob(pipelineManager->GetClearRenderTargetJob(job.second), context);
				break;
			}
		}

	}
}

void SG::D3D11RenderEngine::HandleRenderJob(const SGRenderJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	SetShaders(job, context);

	if (job.association == Association::GLOBAL)
	{
		//HandleGlobalRenderJob(job, entities, context);
	}
	else if (job.association == Association::GROUP)
	{
		//HandleGroupRenderJob(job, entities, context);
	}
	else if (job.association == Association::ENTITY)
	{
		HandleEntityRenderJob(job, entities, context);
	}
}

D3D11_PRIMITIVE_TOPOLOGY SG::D3D11RenderEngine::TranslateTopology(const SGTopology & topology)
{
	switch (topology)
	{
	case SGTopology::POINTLIST:
		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case SGTopology::LINELIST:
		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case SGTopology::LINESTRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case SGTopology::TRIANGLELIST:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case SGTopology::TRIANGLESTRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	default:
		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

void SG::D3D11RenderEngine::SetShaders(const SGRenderJob & job, ID3D11DeviceContext * context)
{
	context->IASetPrimitiveTopology(TranslateTopology(job.topology));
	shaderManager->SetInputLayout(job.inputAssembly, context);
	shaderManager->SetVertexShader(job.vertexShader.shader, context);
	//hull
	//domain
	//geometry
	shaderManager->SetPixelShader(job.pixelShader.shader, context);
}

void SG::D3D11RenderEngine::HandleEntityRenderJob(const SGRenderJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	for (auto& entity : entities)
	{
		SetConstantBuffers(job, entity, context);
	}
}

void SG::D3D11RenderEngine::SetConstantBuffers(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	const RenderShader& vs = job.vertexShader;
	const RenderShader& hs = job.hullShader;
	const RenderShader& ds = job.domainShader;
	const RenderShader& gs = job.geometryShader;
	const RenderShader& ps = job.pixelShader;
	bufferHandler->SetConstantBuffers(vs.constantBuffers, hs.constantBuffers, ds.constantBuffers, gs.constantBuffers, ps.constantBuffers, context, graphicalEntities[entity].groupGuid, entity);
}

void SG::D3D11RenderEngine::SetVertexBuffers(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	const UINT arrSize = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
	ID3D11Buffer* bufferArr[arrSize] = {};
	UINT strideArr[arrSize] = {};
	UINT offsetArr[arrSize] = {};
	UINT counter = 0;
	for (auto& vBuffer : job.vertexBuffers)
	{
		switch (vBuffer.buffer.source)
		{
		case Association::GLOBAL:
			bufferArr[counter++] = bufferHandler->GetBuffer(vBuffer.buffer.resourceGuid, context);
			break;
		case Association::GROUP:
			entityMutex.lock();
			SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
			bufferArr[counter++] = bufferHandler->GetBuffer(vBuffer.buffer.resourceGuid, context, groupGuid);
			entityMutex.unlock();
			break;
		case Association::ENTITY:
			entityMutex.lock();
			bufferArr[counter++] = bufferHandler->GetBuffer(vBuffer.buffer.resourceGuid, context, entity);
			entityMutex.unlock();
			break;
		}

		// Bryt ut ovan till en egen funktion
		// Lägg till funktionalitet i pipelinemanager för att kunna skapa offsets och strides
		// Hämta offsets och strides i egna funktioner
		// Sätt buffrarna
	}
}

void SG::D3D11RenderEngine::HandleClearRenderTargetJob(const SGClearRenderTargetJob & job, ID3D11DeviceContext * context)
{
	ID3D11RenderTargetView* rtv = textureHandler->GetRTV(job.toClear);
	context->ClearRenderTargetView(rtv, job.color);
}

#include "D3D11RenderEngine.h"
#include "D3D11CommonTypes.h"

SG::D3D11RenderEngine::D3D11RenderEngine(const SGRenderSettings & settings) : SGRenderEngine(settings)
{
	this->CreateDeviceAndContext(settings);
	bufferHandler = new D3D11BufferHandler(device);
	samplerHandler = new D3D11SamplerHandler(device);
	shaderManager = new D3D11ShaderManager(device);
	this->stateHandler = new D3D11StateHandler(device);
	this->textureHandler = new D3D11TextureHandler(device);
	this->pipelineManager = new D3D11PipelineManager(device);
	this->drawCallHandler = new D3D11DrawCallHandler(device);
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
	delete samplerHandler;
	delete shaderManager;
	delete stateHandler;
	delete textureHandler;
	delete pipelineManager;
	delete drawCallHandler;
}

SG::D3D11BufferHandler * SG::D3D11RenderEngine::BufferHandler()
{
	return bufferHandler;
}

SG::D3D11SamplerHandler * SG::D3D11RenderEngine::SamplerHandler()
{
	return samplerHandler;
}

SG::D3D11ShaderManager * SG::D3D11RenderEngine::ShaderManager()
{
	return shaderManager;
}

SG::D3D11StateHandler * SG::D3D11RenderEngine::StateHandler()
{
	return stateHandler;
}

SG::D3D11TextureHandler * SG::D3D11RenderEngine::TextureHandler()
{
	return textureHandler;
}

SG::D3D11PipelineManager * SG::D3D11RenderEngine::PipelineManager()
{
	return pipelineManager;
}

SG::D3D11DrawCallHandler * SG::D3D11RenderEngine::DrawCallHandler()
{
	return drawCallHandler;
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
	swapchainDesc.BufferUsage = settings.backBufferSettings.usage;
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
	samplerHandler->SwapUpdateBuffer();
	stateHandler->SwapUpdateBuffer();
	textureHandler->SwapUpdateBuffer();
	drawCallHandler->SwapUpdateBuffer();
}

void SG::D3D11RenderEngine::SwapToWorkWithBuffer()
{
	bufferHandler->SwapToWorkWithBuffer();
	samplerHandler->SwapUpdateBuffer();
	stateHandler->SwapToWorkWithBuffer();
	textureHandler->SwapToWorkWithBuffer();
	drawCallHandler->SwapToWorkWithBuffer();
}

void SG::D3D11RenderEngine::ExecuteJobs(const std::vector<SGGraphicsJob>& jobs)
{
	unsigned int nrOfContexts = static_cast<unsigned int>(jobs.size() < defferedContexts.size() ? jobs.size() : defferedContexts.size());
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

void SG::D3D11RenderEngine::HandlePipelineJobs(const std::vector<SGGraphicsJob>& jobs, int startPos, int endPos, ID3D11DeviceContext * context)
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
			case PipelineJobType::COMPUTE:
				HandleComputeJob(pipelineManager->GetComputeJob(job.second), jobs[i].entitiesToRender, context);
				break;
			case PipelineJobType::CLEAR_RENDER_TARGET:
				HandleClearRenderTargetJob(pipelineManager->GetClearRenderTargetJob(job.second), context);
				break;
			case PipelineJobType::CLEAR_DEPTH_STENCIL:
				HandleClearDepthStencilJob(pipelineManager->GetClearDepthStencilJob(job.second), context);
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
		HandleGroupRenderJob(job, entities, context);
	}
	else if (job.association == Association::ENTITY)
	{
		HandleEntityRenderJob(job, entities, context);
	}

	ClearNecessaryResources(job, context);
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

void SG::D3D11RenderEngine::HandleGlobalRenderJob(const SGRenderJob & job, ID3D11DeviceContext * context)
{
	RenderPipelineState currentState{};
	SGGraphicalEntityID dummy; // Ugly workaround
	SetVertexBuffers(job, currentState.vertexBuffers, dummy, context);
	SetIndexBuffer(job, currentState.indexBuffer, dummy, context);
	SetConstantBuffers(job, currentState, dummy, context);
	SetShaderResourceViews(job, currentState, dummy, context);
	SetSamplerStates(job, currentState, dummy, context);
	SetViewports(job, currentState.viewports, dummy, context);
	SetStates(job, currentState, dummy, context);
	SetOMViews(job, currentState, dummy, context);
	ExecuteDrawCall(job, dummy, context);
}

void SG::D3D11RenderEngine::HandleGroupRenderJob(const SGRenderJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	entityMutex.lock();
	SG::SGGuid currentGroupGuid = graphicalEntities[entities[0]].groupGuid;
	entityMutex.unlock();
	unsigned int nrInGroup = 1;
	RenderPipelineState currentState{};

	for (auto it = entities.begin() + 1; it != entities.end(); ++it)
	{
		entityMutex.lock();

		while (it != entities.end() && graphicalEntities[*it].groupGuid == currentGroupGuid)
		{
			++nrInGroup;
			++it;
		}

		entityMutex.unlock();

		SG::SGGraphicalEntityID entity = *(it - 1);

		SetVertexBuffers(job, currentState.vertexBuffers, entity, context);
		SetIndexBuffer(job, currentState.indexBuffer, entity, context);
		SetConstantBuffers(job, currentState, entity, context);
		SetShaderResourceViews(job, currentState, entity, context);
		SetSamplerStates(job, currentState, entity, context);
		SetViewports(job, currentState.viewports, entity, context);
		SetStates(job, currentState, entity, context);
		SetOMViews(job, currentState, entity, context);
		ExecuteDrawCall(job, entity, nrInGroup, context);

		if (it != entities.end())
		{
			nrInGroup = 1;
			currentGroupGuid = graphicalEntities[entity].groupGuid;
		}
		else
		{
			break; // We cannot iterate past end
		}
	}
}

void SG::D3D11RenderEngine::HandleEntityRenderJob(const SGRenderJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	RenderPipelineState currentState{};

	for (auto& entity : entities)
	{
		SetVertexBuffers(job, currentState.vertexBuffers, entity, context);
		SetIndexBuffer(job, currentState.indexBuffer, entity, context);
		SetConstantBuffers(job, currentState, entity, context);
		SetShaderResourceViews(job, currentState, entity, context);
		SetSamplerStates(job, currentState, entity, context);
		SetViewports(job, currentState.viewports, entity, context);
		SetStates(job, currentState, entity, context);
		SetOMViews(job, currentState, entity, context);
		ExecuteDrawCall(job, entity, context);
	}
}

void SG::D3D11RenderEngine::HandleComputeJob(const SGComputeJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	(void)entities;
	shaderManager->SetComputeShader(job.shader, context);

	if (job.association == Association::GLOBAL)
	{
		HandleGlobalComputeJob(job, context);
	}
	else if (job.association == Association::GROUP)
	{
		//HandleGroupComputeJob(job, entities, context);
	}
	else if (job.association == Association::ENTITY)
	{
		//HandleEntityComputeJob(job, entities, context);
	}

	ClearNecessaryResources(job, context); 
}

void SG::D3D11RenderEngine::HandleGlobalComputeJob(const SGComputeJob & job, ID3D11DeviceContext * context)
{
	ComputePipelineState currentState{};
	SGGraphicalEntityID dummy; // Ugly workaround
	SetConstantBuffersForShader(job.constantBuffers, currentState.constantBuffers, dummy, context, 
		&ID3D11DeviceContext::CSSetConstantBuffers);
	SetShaderResourceViewsForShader(job.shaderResourceViews, currentState.shaderResourceViews, dummy, context,
		&ID3D11DeviceContext::CSSetShaderResources);
	SetSamplerStatesForShader(job.samplers, currentState.samplers,
		dummy, context, &ID3D11DeviceContext::CSSetSamplers);
	
	const int maximumUAVs = 8;
	ID3D11UnorderedAccessView* uavs[maximumUAVs] = {};
	UINT nrOfUAVS = 0;

	for (auto& uav : job.unorderedAccessViews)
		uavs[nrOfUAVS++] = GetUAV(uav, dummy);

	context->CSSetUnorderedAccessViews(0, nrOfUAVS, uavs, nullptr);

	SG::D3D11DrawCallHandler::DispatchCall dispatchCall = GetDispatchCall(job.dispatchCall, dummy);
	if (dispatchCall.indirect)
		context->DispatchIndirect(bufferHandler->GetBuffer(dispatchCall.data.dispatchIndirect.bufferForArgs, context), dispatchCall.data.dispatchIndirect.alignedByteOffsetForArgs);
	else
		context->Dispatch(dispatchCall.data.dispatch.threadGroupCountX, dispatchCall.data.dispatch.threadGroupCountY, dispatchCall.data.dispatch.threadGroupCountZ);
}

void SG::D3D11RenderEngine::ClearNecessaryResources(const SGRenderJob& job, ID3D11DeviceContext* context)
{
	ClearVertexBuffers(job.vertexBuffers, context);

	if (job.indexBuffer.clearAtEnd)
	{
		DXGI_FORMAT format = job.indexBuffer.format == IndexBufferFormat::IB_32_BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		context->IASetIndexBuffer(nullptr, format, 0);
	}

	ClearConstantBuffers(job, context);
	ClearShaderResourceViews(job, context);
	ClearOMViews(job, context);
}

void SG::D3D11RenderEngine::ClearNecessaryResources(const SGComputeJob& job, ID3D11DeviceContext* context)
{
	ClearConstantBuffersForShader(job.constantBuffers, context, &ID3D11DeviceContext::CSSetConstantBuffers);
	ClearShaderResourceViewsForShader(job.shaderResourceViews, context, &ID3D11DeviceContext::CSSetShaderResources);

	ID3D11UnorderedAccessView* uavArr[8] = {};
	for (unsigned int i = 0; i < job.unorderedAccessViews.size(); ++i)
	{
		if (job.unorderedAccessViews[i].clearAtEnd)
		{
			int nrToClear = 1;

			while (i + nrToClear < job.unorderedAccessViews.size() && job.unorderedAccessViews[i + nrToClear].clearAtEnd)
			{
				++nrToClear;
			}

			context->CSSetUnorderedAccessViews(i, nrToClear, uavArr, nullptr);
			i += nrToClear - 1;
		}
	}
}

void SG::D3D11RenderEngine::ClearVertexBuffers(const std::vector<SGVertexBuffer>& vertexBuffers, ID3D11DeviceContext* context)
{
	const UINT arrSize = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
	ID3D11Buffer* bufferArr[arrSize] = {};
	UINT strideArr[arrSize] = {};
	UINT offsetArr[arrSize] = {};

	for (unsigned int i = 0; i < vertexBuffers.size(); ++i)
	{
		if (vertexBuffers[i].clearAtEnd)
		{
			int startPos = i;
			int nrToClear = 1;

			while (i + nrToClear < vertexBuffers.size() && vertexBuffers[i + nrToClear].clearAtEnd)
			{
				++nrToClear;
			}

			context->IASetVertexBuffers(startPos, nrToClear, bufferArr, strideArr, offsetArr);
			i += nrToClear - 1;
		}
	}
}

void SG::D3D11RenderEngine::ClearConstantBuffers(const SGRenderJob& job, ID3D11DeviceContext* context)
{
	ClearConstantBuffersForShader(job.vertexShader.constantBuffers, context, &ID3D11DeviceContext::VSSetConstantBuffers);
	ClearConstantBuffersForShader(job.hullShader.constantBuffers, context, &ID3D11DeviceContext::HSSetConstantBuffers);
	ClearConstantBuffersForShader(job.domainShader.constantBuffers, context, &ID3D11DeviceContext::DSSetConstantBuffers);
	ClearConstantBuffersForShader(job.geometryShader.constantBuffers, context, &ID3D11DeviceContext::GSSetConstantBuffers);
	ClearConstantBuffersForShader(job.pixelShader.constantBuffers, context, &ID3D11DeviceContext::PSSetConstantBuffers);
}

void SG::D3D11RenderEngine::ClearConstantBuffersForShader(const std::vector<ConstantBuffer>& constantBuffers, ID3D11DeviceContext* context, void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11Buffer* const*))
{
	const UINT arrSize = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	ID3D11Buffer* bufferArr[arrSize] = {};

	for (unsigned int i = 0; i < constantBuffers.size(); ++i)
	{
		if (constantBuffers[i].clearAtEnd)
		{
			int nrToClear = 1;

			while (i + nrToClear < constantBuffers.size() && constantBuffers[i + nrToClear].clearAtEnd)
			{
				++nrToClear;
			}

			(context->*func)(i, nrToClear, bufferArr);
			i += nrToClear - 1;
		}
	}
}

void SG::D3D11RenderEngine::ClearShaderResourceViews(const SGRenderJob& job, ID3D11DeviceContext* context)
{
	ClearShaderResourceViewsForShader(job.vertexShader.shaderResourceViews, context, &ID3D11DeviceContext::VSSetShaderResources);
	ClearShaderResourceViewsForShader(job.hullShader.shaderResourceViews, context, &ID3D11DeviceContext::HSSetShaderResources);
	ClearShaderResourceViewsForShader(job.domainShader.shaderResourceViews, context, &ID3D11DeviceContext::DSSetShaderResources);
	ClearShaderResourceViewsForShader(job.geometryShader.shaderResourceViews, context, &ID3D11DeviceContext::GSSetShaderResources);
	ClearShaderResourceViewsForShader(job.pixelShader.shaderResourceViews, context, &ID3D11DeviceContext::PSSetShaderResources);
}

void SG::D3D11RenderEngine::ClearShaderResourceViewsForShader(const std::vector<ResourceView>& srvs, ID3D11DeviceContext* context, void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11ShaderResourceView* const*))
{
	const UINT arrSize = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	ID3D11ShaderResourceView* srvArr[arrSize] = {};

	for (unsigned int i = 0; i < srvs.size(); ++i)
	{
		if (srvs[i].clearAtEnd)
		{
			int nrToClear = 1;

			while (i + nrToClear < srvs.size() && srvs[i + nrToClear].clearAtEnd)
			{
				++nrToClear;
			}

			(context->*func)(i, nrToClear, srvArr);
			i += nrToClear - 1;
		}
	}
}

void SG::D3D11RenderEngine::ClearOMViews(const SGRenderJob& job, ID3D11DeviceContext* context)
{
	const int maximumRTVsAndUAVs = 8;
	ID3D11RenderTargetView* rtvs[maximumRTVsAndUAVs] = {};
	ID3D11UnorderedAccessView* uavs[maximumRTVsAndUAVs] = {};
	ID3D11DepthStencilView* dsv = nullptr;
	int rtvsToClear[maximumRTVsAndUAVs];
	int nrOfRTVsToClear = 0;
	int uavsToClear[maximumRTVsAndUAVs];
	int nrOfUAVsToClear = 0;
	UINT nrOfRTVs = static_cast<UINT>(job.rtvs.size());
	UINT nrOfUAVs = static_cast<UINT>(job.uavs.size());

	for (unsigned int i = 0; i < job.rtvs.size(); ++i)
		if (job.rtvs[i].clearAtEnd)
			rtvsToClear[nrOfRTVsToClear++] = i;

	for (unsigned int i = 0; i < job.uavs.size(); ++i)
		if (job.uavs[i].clearAtEnd)
			uavsToClear[nrOfUAVsToClear++] = i;

	if (job.dsv.clearAtEnd || nrOfRTVsToClear != 0 || nrOfUAVsToClear != 0)
	{
		context->OMGetRenderTargetsAndUnorderedAccessViews(nrOfRTVs, rtvs, job.dsv.clearAtEnd ? &dsv : nullptr, nrOfRTVs, nrOfUAVs, uavs);

		for (int i = 0; i < nrOfRTVsToClear; ++i)
		{
			rtvs[rtvsToClear[i]]->Release();
			rtvs[rtvsToClear[i]] = nullptr;
		}

		for (int i = 0; i < nrOfUAVsToClear; ++i)
		{
			uavs[uavsToClear[i]]->Release();
			uavs[uavsToClear[i]] = nullptr;
		}

		if (job.dsv.clearAtEnd)
		{
			dsv->Release();
			dsv = nullptr;
		}

		context->OMSetRenderTargetsAndUnorderedAccessViews(nrOfRTVs, rtvs, dsv, nrOfRTVs, nrOfUAVs, uavs, nullptr);

		for (int i = 0; i < nrOfRTVsToClear; ++i)
			if(rtvs[rtvsToClear[i]] != nullptr)
				rtvs[rtvsToClear[i]]->Release();

		for (int i = 0; i < nrOfUAVsToClear; ++i)
			if(uavs[uavsToClear[i]] != nullptr)
				uavs[uavsToClear[i]]->Release();
			
	}
}

void SG::D3D11RenderEngine::SetConstantBuffers(const SGRenderJob & job, RenderPipelineState& previousFrame, const SGGraphicalEntityID & entity,
	ID3D11DeviceContext * context)
{
	if(job.vertexShader.constantBuffers.size())
		SetConstantBuffersForShader(job.vertexShader.constantBuffers, previousFrame.vertexShader.constantBuffers,
			entity, context, &ID3D11DeviceContext::VSSetConstantBuffers);

	if(job.hullShader.constantBuffers.size())
		SetConstantBuffersForShader(job.hullShader.constantBuffers, previousFrame.hullShader.constantBuffers,
			entity, context, &ID3D11DeviceContext::HSSetConstantBuffers);

	if(job.domainShader.constantBuffers.size())
		SetConstantBuffersForShader(job.domainShader.constantBuffers, previousFrame.domainShader.constantBuffers,
			entity, context, &ID3D11DeviceContext::DSSetConstantBuffers);

	if(job.geometryShader.constantBuffers.size())
		SetConstantBuffersForShader(job.geometryShader.constantBuffers, previousFrame.geometryShader.constantBuffers,
			entity, context, &ID3D11DeviceContext::GSSetConstantBuffers);

	if(job.pixelShader.constantBuffers.size())
		SetConstantBuffersForShader(job.pixelShader.constantBuffers, previousFrame.pixelShader.constantBuffers,
			entity, context, &ID3D11DeviceContext::PSSetConstantBuffers);
}

void SG::D3D11RenderEngine::SetShaderResourceViews(const SGRenderJob & job, RenderPipelineState& previousFrame, const SGGraphicalEntityID & entity,
	ID3D11DeviceContext * context)
{
	if(job.vertexShader.shaderResourceViews.size())
		SetShaderResourceViewsForShader(job.vertexShader.shaderResourceViews, previousFrame.vertexShader.shaderResourceViews,
			entity, context, &ID3D11DeviceContext::VSSetShaderResources);

	if(job.hullShader.shaderResourceViews.size())
		SetShaderResourceViewsForShader(job.hullShader.shaderResourceViews, previousFrame.hullShader.shaderResourceViews,
			entity, context, &ID3D11DeviceContext::HSSetShaderResources);

	if(job.domainShader.shaderResourceViews.size())
		SetShaderResourceViewsForShader(job.domainShader.shaderResourceViews, previousFrame.domainShader.shaderResourceViews,
			entity, context, &ID3D11DeviceContext::DSSetShaderResources);

	if(job.geometryShader.shaderResourceViews.size())
		SetShaderResourceViewsForShader(job.geometryShader.shaderResourceViews, previousFrame.geometryShader.shaderResourceViews,
			entity, context, &ID3D11DeviceContext::GSSetShaderResources);
	
	if(job.pixelShader.shaderResourceViews.size())
		SetShaderResourceViewsForShader(job.pixelShader.shaderResourceViews, previousFrame.pixelShader.shaderResourceViews,
			entity, context, &ID3D11DeviceContext::PSSetShaderResources);
}

void SG::D3D11RenderEngine::SetSamplerStates(const SGRenderJob & job, RenderPipelineState& previousFrame, const SGGraphicalEntityID & entity,
	ID3D11DeviceContext * context)
{
	if(job.vertexShader.samplers.size())
		SetSamplerStatesForShader(job.vertexShader.samplers, previousFrame.vertexShader.samplers,
			entity, context, &ID3D11DeviceContext::VSSetSamplers);

	if(job.hullShader.samplers.size())
		SetSamplerStatesForShader(job.hullShader.samplers, previousFrame.hullShader.samplers,
			entity, context, &ID3D11DeviceContext::HSSetSamplers);

	if(job.domainShader.samplers.size())
		SetSamplerStatesForShader(job.domainShader.samplers, previousFrame.domainShader.samplers,
			entity, context, &ID3D11DeviceContext::DSSetSamplers);

	if(job.geometryShader.samplers.size())
		SetSamplerStatesForShader(job.geometryShader.samplers, previousFrame.geometryShader.samplers,
			entity, context, &ID3D11DeviceContext::GSSetSamplers);

	if(job.pixelShader.samplers.size())
		SetSamplerStatesForShader(job.pixelShader.samplers, previousFrame.pixelShader.samplers,
			entity, context, &ID3D11DeviceContext::PSSetSamplers);
}

void SG::D3D11RenderEngine::SetVertexBuffers(const SGRenderJob & job, VertexBufferState currentState[], const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	const UINT arrSize = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
	ID3D11Buffer* bufferArr[arrSize] = {};
	UINT strideArr[arrSize] = {};
	UINT offsetArr[arrSize] = {};
	UINT counter = 0;
	for (auto& vBuffer : job.vertexBuffers)
	{
		bufferArr[counter] = GetBuffer(vBuffer.buffer, entity, context);

		if (vBuffer.offset.resourceGuid != SGGuid())
			offsetArr[counter] = GetOffset(vBuffer.offset, entity);

		if (vBuffer.stride.resourceGuid != SGGuid())
			strideArr[counter] = GetStride(vBuffer.stride, entity);
		else
			strideArr[counter] = GetStrideFromVB(vBuffer.buffer, entity);

		++counter;
	}

	UINT startOfNewData = static_cast<UINT>(-1);
	for (UINT i = 0; i < counter && startOfNewData == static_cast<UINT>(-1); ++i)
	{
		auto& prevData = currentState[i];
		if (bufferArr[i] != prevData.buffer || offsetArr[i] != prevData.offset || strideArr[i] != prevData.stride)
			startOfNewData = i;
	}

	if (startOfNewData != static_cast<UINT>(-1))
	{
		context->IASetVertexBuffers(startOfNewData, counter - startOfNewData,
			bufferArr + startOfNewData, strideArr + startOfNewData, offsetArr + startOfNewData);

		for (unsigned int i = startOfNewData; i < counter; ++i)
		{
			currentState[i].buffer = bufferArr[i];
			currentState[i].offset = offsetArr[i];
			currentState[i].stride = strideArr[i];
		}
	}
}

void SG::D3D11RenderEngine::SetIndexBuffer(const SGRenderJob & job, IndexBufferState& currentState, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	ID3D11Buffer* buffer;
	UINT offset = 0;

	buffer = GetBuffer(job.indexBuffer.buffer, entity, context);

	if (job.indexBuffer.offset.resourceGuid != SGGuid())
		offset = GetOffset(job.indexBuffer.offset, entity);

	DXGI_FORMAT format = job.indexBuffer.format == IndexBufferFormat::IB_32_BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

	if (currentState.buffer != buffer || currentState.offset != offset)
	{
		context->IASetIndexBuffer(buffer, format, offset);
		currentState.buffer = buffer;
		currentState.offset = offset;
	}
}

void SG::D3D11RenderEngine::SetOMViews(const SGRenderJob & job, RenderPipelineState& currentState, 
	const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	const int maximumRTVsAndUAVs = 8;
	ID3D11RenderTargetView* rtvs[maximumRTVsAndUAVs] = {};
	ID3D11UnorderedAccessView* uavs[maximumRTVsAndUAVs] = {};
	UINT nrOfRTVs = 0;
	UINT nrOfUAVS = 0;
	ID3D11DepthStencilView* dsv = nullptr;

	for (auto& rtv : job.rtvs)
		rtvs[nrOfRTVs++] = GetRTV(rtv, entity);

	if(job.dsv.component.resourceGuid != SGGuid())
		dsv = GetDSV(job.dsv, entity);

	for (auto& uav : job.uavs)
		uavs[nrOfUAVS++] = GetUAV(uav, entity);

	UINT startOfNewRTVData = static_cast<UINT>(-1);
	for (unsigned int i = 0; i < nrOfRTVs; ++i)
	{
		if (currentState.rtvs[i] != rtvs[i])
		{
			startOfNewRTVData = (startOfNewRTVData == static_cast<UINT>(-1)) ? i : startOfNewRTVData;
			currentState.rtvs[i] = rtvs[i];
		}
	}

	UINT startOfNewUAVData = static_cast<UINT>(-1);
	for (unsigned int i = 0; i < nrOfUAVS; ++i)
	{
		if (currentState.uavs[i] != uavs[i])
		{
			startOfNewUAVData = (startOfNewUAVData == static_cast<UINT>(-1)) ? i : startOfNewUAVData;
			currentState.uavs[i] = uavs[i];
		}
	}
	
	if (startOfNewRTVData != static_cast<UINT>(-1) || startOfNewUAVData != static_cast<UINT>(-1) || dsv != currentState.dsv)
	{
		startOfNewRTVData = (startOfNewRTVData == static_cast<UINT>(-1)) ? 0 : startOfNewRTVData;
		startOfNewUAVData = (startOfNewUAVData == static_cast<UINT>(-1)) ? 0 : startOfNewUAVData;
		context->OMSetRenderTargetsAndUnorderedAccessViews(nrOfRTVs - startOfNewRTVData, rtvs + startOfNewRTVData, dsv,
			nrOfRTVs, nrOfUAVS - startOfNewUAVData, uavs + startOfNewUAVData, nullptr);
		currentState.dsv = dsv;
	}
}

void SG::D3D11RenderEngine::SetViewports(const SGRenderJob & job, D3D11_VIEWPORT currentState[],
	const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT nrOfViewports = 0;

	for (auto& vp : job.viewports)
		viewports[nrOfViewports++] = GetViewport(vp, entity);

	UINT startOfNewViewPortData = static_cast<UINT>(-1);
	for (unsigned int i = 0; i < nrOfViewports; ++i)
	{
		if (currentState[i] != viewports[i])
		{
			startOfNewViewPortData = (startOfNewViewPortData == static_cast<UINT>(-1)) ? i : startOfNewViewPortData;
			currentState[i] = viewports[i];
		}
	}

	if(startOfNewViewPortData != static_cast<UINT>(-1))
		context->RSSetViewports(nrOfViewports - startOfNewViewPortData, viewports + startOfNewViewPortData);
}

void SG::D3D11RenderEngine::SetStates(const SGRenderJob & job, RenderPipelineState& currentState,
	const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	ID3D11RasterizerState* rs = GetRasterizerState(job.rasterizerState, entity);

	if (currentState.rasterizerState != rs)
	{
		context->RSSetState(GetRasterizerState(job.rasterizerState, entity));
		currentState.rasterizerState = rs;
	}

	//blendstate
	//depthstencilstate
}

void SG::D3D11RenderEngine::ExecuteDrawCall(const SGRenderJob & job, const SGGraphicalEntityID& entity, unsigned int nrInGroup, ID3D11DeviceContext * context)
{
	SG::D3D11DrawCallHandler::DrawCall drawCall = GetDrawCall(job.drawCall, entity);

	switch (drawCall.type)
	{
	case SG::D3D11DrawCallHandler::DrawType::DRAW:
		context->Draw(GetVertexCount(job, drawCall.data.draw.vertexCount, entity), drawCall.data.draw.startVertexLocation);
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INDEXED:
		context->DrawIndexed(GetIndexCount(job, drawCall.data.draw.vertexCount, entity), drawCall.data.drawIndexed.startIndexLocation,
			drawCall.data.drawIndexed.baseVertexLocation);
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INSTANCED:
		SG::D3D11DrawCallHandler::DrawInstancedData drawInstanced = drawCall.data.drawInstanced;
		context->DrawInstanced(GetVertexCount(job, drawInstanced.vertexCountPerInstance, entity),
							   drawInstanced.instanceCount == 0 ? nrInGroup : drawInstanced.instanceCount,
							   drawInstanced.startVertexLocation, drawInstanced.startInstanceLocation);
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INDEXED_INSTANCED:
		SG::D3D11DrawCallHandler::DrawIndexedInstanced drawIndexedInstanced = drawCall.data.drawIndexedInstanced;
		context->DrawIndexedInstanced(GetIndexCount(job, drawIndexedInstanced.indexCountPerInstance, entity),
									  drawIndexedInstanced.instanceCount == 0 ? nrInGroup : drawIndexedInstanced.instanceCount,
									  drawIndexedInstanced.startIndexLocation, drawIndexedInstanced.baseVertexLocation,
									  drawIndexedInstanced.startInstanceLocation);
		break;
	}

}

void SG::D3D11RenderEngine::ExecuteDrawCall(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	SG::D3D11DrawCallHandler::DrawCall drawCall = GetDrawCall(job.drawCall, entity);

	switch (drawCall.type)
	{
	case SG::D3D11DrawCallHandler::DrawType::DRAW:
		context->Draw(GetVertexCount(job, drawCall.data.draw.vertexCount, entity), drawCall.data.draw.startVertexLocation);
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INDEXED:
		context->DrawIndexed(GetIndexCount(job, drawCall.data.draw.vertexCount, entity), drawCall.data.drawIndexed.startIndexLocation, drawCall.data.drawIndexed.baseVertexLocation);
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INSTANCED:
		SG::D3D11DrawCallHandler::DrawInstancedData drawInstanced = drawCall.data.drawInstanced;
		context->DrawInstanced(GetVertexCount(job, drawInstanced.vertexCountPerInstance, entity),
			drawInstanced.instanceCount, drawInstanced.startVertexLocation, drawInstanced.startInstanceLocation);
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INDEXED_INSTANCED:
		SG::D3D11DrawCallHandler::DrawIndexedInstanced drawIndexedInstanced = drawCall.data.drawIndexedInstanced;
		context->DrawIndexedInstanced(GetIndexCount(job, drawIndexedInstanced.indexCountPerInstance, entity),
			drawIndexedInstanced.instanceCount, drawIndexedInstanced.startIndexLocation,
			drawIndexedInstanced.baseVertexLocation, drawIndexedInstanced.startInstanceLocation);
		break;
	}

}

void SG::D3D11RenderEngine::SetConstantBuffersForShader(const std::vector<ConstantBuffer>& buffers, ID3D11Buffer** currentState, 
	const SGGraphicalEntityID & entity, ID3D11DeviceContext * context,
	void(_stdcall ID3D11DeviceContext::*func)(UINT, UINT, ID3D11Buffer * const *))
{
	const UINT arrSize = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	ID3D11Buffer* bufferArr[arrSize] = {};
	UINT counter = 0;
	for (auto& cBuffer : buffers)
		bufferArr[counter++] = GetBuffer(cBuffer.component, entity, context);

	UINT startOfNewConstantBufferData = static_cast<UINT>(-1);
	for (unsigned int i = 0; i < counter; ++i)
	{
		if (currentState[i] != bufferArr[i])
		{
			startOfNewConstantBufferData = (startOfNewConstantBufferData == static_cast<UINT>(-1)) ? i : startOfNewConstantBufferData;
			currentState[i] = bufferArr[i];
		}
	}

	if(startOfNewConstantBufferData != static_cast<UINT>(-1))
		(context->*func)(startOfNewConstantBufferData, arrSize - startOfNewConstantBufferData, bufferArr + startOfNewConstantBufferData);
}

void SG::D3D11RenderEngine::SetShaderResourceViewsForShader(const std::vector<ResourceView>& srvs, ID3D11ShaderResourceView** currentState, 
	const SGGraphicalEntityID & entity, ID3D11DeviceContext * context,
	void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11ShaderResourceView * const *))
{
	const UINT arrSize = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	ID3D11ShaderResourceView* srvArr[arrSize] = {};
	UINT counter = 0;
	for (auto& srv : srvs)
		srvArr[counter++] = GetSRV(srv, entity);

	UINT startOfNewSRVData = static_cast<UINT>(-1);
	for (unsigned int i = 0; i < counter; ++i)
	{
		if (currentState[i] != srvArr[i])
		{
			startOfNewSRVData = (startOfNewSRVData == static_cast<UINT>(-1)) ? i : startOfNewSRVData;
			currentState[i] = srvArr[i];
		}
	}

	if (startOfNewSRVData != static_cast<UINT>(-1))
		(context->*func)(startOfNewSRVData, arrSize - startOfNewSRVData, srvArr + startOfNewSRVData);
}

void SG::D3D11RenderEngine::SetSamplerStatesForShader(const std::vector<PipelineComponent>& samplers, ID3D11SamplerState** currentState, 
	const SGGraphicalEntityID & entity, ID3D11DeviceContext * context, 
	void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11SamplerState * const *))
{
	const UINT arrSize = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
	ID3D11SamplerState* samplerArr[arrSize] = {};
	UINT counter = 0;
	for (auto& sampler : samplers)
		samplerArr[counter++] = GetSamplerState(sampler, entity);

	UINT startOfNewSamplerData = static_cast<UINT>(-1);
	for (unsigned int i = 0; i < counter; ++i)
	{
		if (currentState[i] != samplerArr[i])
		{
			startOfNewSamplerData = (startOfNewSamplerData == static_cast<UINT>(-1)) ? i : startOfNewSamplerData;
			currentState[i] = samplerArr[i];
		}
	}

	if(startOfNewSamplerData != -1)
		(context->*func)(startOfNewSamplerData, arrSize - startOfNewSamplerData, samplerArr + startOfNewSamplerData);
}

ID3D11Buffer * SG::D3D11RenderEngine::GetBuffer(const PipelineComponent & component, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	ID3D11Buffer* toReturn = nullptr;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = bufferHandler->GetBuffer(component.resourceGuid, context);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = bufferHandler->GetBuffer(component.resourceGuid, context, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = bufferHandler->GetBuffer(component.resourceGuid, context, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11SamplerState * SG::D3D11RenderEngine::GetSamplerState(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	ID3D11SamplerState * toReturn = nullptr;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = samplerHandler->GetSamplerState(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = samplerHandler->GetSamplerState(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = samplerHandler->GetSamplerState(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

UINT SG::D3D11RenderEngine::GetOffset(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	UINT toReturn = 0;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = bufferHandler->GetOffset(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = bufferHandler->GetOffset(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = bufferHandler->GetOffset(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

UINT SG::D3D11RenderEngine::GetStride(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	UINT toReturn = 0;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = bufferHandler->GetStride(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = bufferHandler->GetStride(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = bufferHandler->GetStride(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

UINT SG::D3D11RenderEngine::GetStrideFromVB(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	UINT toReturn = 0;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = bufferHandler->GetVBElementSize(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = bufferHandler->GetVBElementSize(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = bufferHandler->GetVBElementSize(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11ShaderResourceView * SG::D3D11RenderEngine::GetSRV(const ResourceView & view, const SGGraphicalEntityID & entity)
{
	ID3D11ShaderResourceView* toReturn = nullptr;
	switch (view.component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetSRV(view.component.resourceGuid) : bufferHandler->GetSRV(view.component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetSRV(view.component.resourceGuid, groupGuid) : bufferHandler->GetSRV(view.component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetSRV(view.component.resourceGuid, entity) : bufferHandler->GetSRV(view.component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11RenderTargetView* SG::D3D11RenderEngine::GetRTV(const ResourceView & view, const SGGraphicalEntityID & entity)
{
	ID3D11RenderTargetView* toReturn = nullptr;
	switch (view.component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetRTV(view.component.resourceGuid) : nullptr;
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetRTV(view.component.resourceGuid, groupGuid) : nullptr;
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetRTV(view.component.resourceGuid, entity) : nullptr;
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11DepthStencilView * SG::D3D11RenderEngine::GetDSV(const ResourceView & view, const SGGraphicalEntityID & entity)
{
	ID3D11DepthStencilView* toReturn = nullptr;
	switch (view.component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetDSV(view.component.resourceGuid) : nullptr;
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetDSV(view.component.resourceGuid, groupGuid) : nullptr;
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetDSV(view.component.resourceGuid, entity) : nullptr;
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11UnorderedAccessView * SG::D3D11RenderEngine::GetUAV(const ResourceView & view, const SGGraphicalEntityID & entity)
{
	ID3D11UnorderedAccessView* toReturn = nullptr;
	switch (view.component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetUAV(view.component.resourceGuid) : bufferHandler->GetUAV(view.component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetUAV(view.component.resourceGuid, groupGuid) : bufferHandler->GetUAV(view.component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = view.type == ResourceView::ResourceType::TEXTURE ? textureHandler->GetUAV(view.component.resourceGuid, entity) : bufferHandler->GetUAV(view.component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11RasterizerState * SG::D3D11RenderEngine::GetRasterizerState(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	ID3D11RasterizerState* toReturn = nullptr;

	if (component.resourceGuid == SGGuid())
		return toReturn;

	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = stateHandler->GetRazterizerState(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = stateHandler->GetRazterizerState(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = stateHandler->GetRazterizerState(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11RenderEngine::GetDrawCall(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	SG::D3D11DrawCallHandler::DrawCall toReturn;

	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = drawCallHandler->GetDrawCall(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = drawCallHandler->GetDrawCall(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = drawCallHandler->GetDrawCall(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	default:
		throw std::runtime_error("Error, this error should never happen, because it implies that an association that does not exist has been used for a draw call");
	}

	return toReturn;
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11RenderEngine::GetDispatchCall(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	SG::D3D11DrawCallHandler::DispatchCall toReturn;

	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = drawCallHandler->GetDispatchCall(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = drawCallHandler->GetDispatchCall(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = drawCallHandler->GetDispatchCall(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	default:
		throw std::runtime_error("Error, this error should never happen, because it implies that an association that does not exist has been used for a dispatch call");
	}

	return toReturn;
}

D3D11_VIEWPORT SG::D3D11RenderEngine::GetViewport(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	D3D11_VIEWPORT toReturn = D3D11_VIEWPORT();

	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = stateHandler->GetViewport(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = stateHandler->GetViewport(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = stateHandler->GetViewport(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

UINT SG::D3D11RenderEngine::GetVertexCount(const SGRenderJob & job, UINT vertexCount, const SGGraphicalEntityID & entity)
{
	if (vertexCount < DrawCallVertexBufferToFetchFrom::BUFFER14)
		return vertexCount;

	UINT index = (vertexCount - DrawCallVertexBufferToFetchFrom::BUFFER0);

	UINT toReturn = 0;
	const PipelineComponent& buffer = job.vertexBuffers[index].buffer;

	switch (buffer.source)
	{
	case Association::GLOBAL:
	{
		toReturn = bufferHandler->GetElementCount(buffer.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = bufferHandler->GetElementCount(buffer.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = bufferHandler->GetElementCount(buffer.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

UINT SG::D3D11RenderEngine::GetIndexCount(const SGRenderJob & job, UINT indexCount, const SGGraphicalEntityID & entity)
{
	if (indexCount != 0)
		return indexCount;

	UINT toReturn = 0;
	const PipelineComponent& buffer = job.indexBuffer.buffer;

	switch (buffer.source)
	{
	case Association::GLOBAL:
	{
		toReturn = bufferHandler->GetElementCount(buffer.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = bufferHandler->GetElementCount(buffer.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = bufferHandler->GetElementCount(buffer.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

void SG::D3D11RenderEngine::HandleClearRenderTargetJob(const SGClearRenderTargetJob & job, ID3D11DeviceContext * context)
{
	ID3D11RenderTargetView* rtv = textureHandler->GetRTV(job.toClear);
	context->ClearRenderTargetView(rtv, job.color);
}

void SG::D3D11RenderEngine::HandleClearDepthStencilJob(const SGClearDepthStencilJob & job, ID3D11DeviceContext * context)
{
	ID3D11DepthStencilView* dsv = textureHandler->GetDSV(job.toClear);
	UINT clearFlags = 0 | (job.clearDepth ? D3D11_CLEAR_DEPTH : 0) | (job.clearStencil ? D3D11_CLEAR_STENCIL : 0);
	context->ClearDepthStencilView(dsv, clearFlags, job.depthClearValue, job.stencilClearValue);
}

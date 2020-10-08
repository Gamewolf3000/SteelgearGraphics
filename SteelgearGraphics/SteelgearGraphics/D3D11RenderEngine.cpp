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

void SG::D3D11RenderEngine::HandleGroupRenderJob(const SGRenderJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	
	SG::SGGuid currentGroupGuid = SG::SGGuid();

	for (auto& entity : entities)
	{
		if (graphicalEntities[entity].groupGuid == currentGroupGuid)
			continue;

		currentGroupGuid = graphicalEntities[entity].groupGuid;
		SetVertexBuffers(job, entity, context);
		SetIndexBuffer(job, entity, context);
		SetConstantBuffers(job, entity, context);
		SetShaderResourceViews(job, entity, context);
		SetSamplerStates(job, entity, context);
		SetViewports(job, entity, context);
		SetStates(job, entity, context);
		SetOMViews(job, entity, context);
		//ExecuteDrawCall(job, entity, context);
	}
}

void SG::D3D11RenderEngine::HandleEntityRenderJob(const SGRenderJob & job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext * context)
{
	for (auto& entity : entities)
	{
		SetVertexBuffers(job, entity, context);
		SetIndexBuffer(job, entity, context);
		SetConstantBuffers(job, entity, context);
		SetShaderResourceViews(job, entity, context);
		SetSamplerStates(job, entity, context);
		SetViewports(job, entity, context);
		SetStates(job, entity, context);
		SetOMViews(job, entity, context);
		ExecuteDrawCall(job, entity, context);
	}
}

void SG::D3D11RenderEngine::SetConstantBuffers(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	SetConstantBuffersForShader(job.vertexShader.constantBuffers, entity, context, &ID3D11DeviceContext::VSSetConstantBuffers);
	SetConstantBuffersForShader(job.hullShader.constantBuffers, entity, context, &ID3D11DeviceContext::HSSetConstantBuffers);
	SetConstantBuffersForShader(job.domainShader.constantBuffers, entity, context, &ID3D11DeviceContext::DSSetConstantBuffers);
	SetConstantBuffersForShader(job.geometryShader.constantBuffers, entity, context, &ID3D11DeviceContext::GSSetConstantBuffers);
	SetConstantBuffersForShader(job.pixelShader.constantBuffers, entity, context, &ID3D11DeviceContext::PSSetConstantBuffers);
}

void SG::D3D11RenderEngine::SetShaderResourceViews(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	SetShaderResourceViewsForShader(job.vertexShader.shaderResourceViews, entity, context, &ID3D11DeviceContext::VSSetShaderResources);
	SetShaderResourceViewsForShader(job.hullShader.shaderResourceViews, entity, context, &ID3D11DeviceContext::HSSetShaderResources);
	SetShaderResourceViewsForShader(job.domainShader.shaderResourceViews, entity, context, &ID3D11DeviceContext::DSSetShaderResources);
	SetShaderResourceViewsForShader(job.geometryShader.shaderResourceViews, entity, context, &ID3D11DeviceContext::GSSetShaderResources);
	SetShaderResourceViewsForShader(job.pixelShader.shaderResourceViews, entity, context, &ID3D11DeviceContext::PSSetShaderResources);
}

void SG::D3D11RenderEngine::SetSamplerStates(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	SetSamplerStatesForShader(job.vertexShader.samplers, entity, context, &ID3D11DeviceContext::VSSetSamplers);
	SetSamplerStatesForShader(job.hullShader.samplers, entity, context, &ID3D11DeviceContext::HSSetSamplers);
	SetSamplerStatesForShader(job.domainShader.samplers, entity, context, &ID3D11DeviceContext::DSSetSamplers);
	SetSamplerStatesForShader(job.geometryShader.samplers, entity, context, &ID3D11DeviceContext::GSSetSamplers);
	SetSamplerStatesForShader(job.pixelShader.samplers, entity, context, &ID3D11DeviceContext::PSSetSamplers);
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
		bufferArr[counter] = GetBuffer(vBuffer.buffer, entity, context);

		if (vBuffer.offset.resourceGuid != SGGuid())
			offsetArr[counter] = GetOffset(vBuffer.offset, entity);

		if (vBuffer.stride.resourceGuid != SGGuid())
			strideArr[counter] = GetStride(vBuffer.stride, entity);
		else
			strideArr[counter] = GetStrideFromVB(vBuffer.buffer, entity);

		++counter;
	}

	context->IASetVertexBuffers(0, arrSize, bufferArr, strideArr, offsetArr);
}

void SG::D3D11RenderEngine::SetIndexBuffer(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	ID3D11Buffer* buffer;
	UINT offset = 0;

	buffer = GetBuffer(job.indexBuffer.buffer, entity, context);

	if (job.indexBuffer.offset.resourceGuid != SGGuid())
		offset = GetOffset(job.indexBuffer.offset, entity);

	DXGI_FORMAT format = job.indexBuffer.format == IndexBufferFormat::IB_32_BIT ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

	context->IASetIndexBuffer(buffer, format, offset);
}

void SG::D3D11RenderEngine::SetOMViews(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	const int maximumRTVsAndUAVs = 8;
	ID3D11RenderTargetView* rtvs[maximumRTVsAndUAVs] = {};
	ID3D11UnorderedAccessView* uavs[maximumRTVsAndUAVs] = {};
	UINT nrOfRTVs = 0;
	UINT nrOfUAVS = 0;
	ID3D11DepthStencilView* dsv = nullptr;

	for (auto& rtv : job.rtvs)
		rtvs[nrOfRTVs++] = GetRTV(rtv, entity);

	if(job.dsv.resourceGuid != SGGuid())
		dsv = GetDSV(job.dsv, entity);

	//UAVs

	context->OMSetRenderTargetsAndUnorderedAccessViews(nrOfRTVs, rtvs, dsv, nrOfRTVs, nrOfUAVS, uavs, nullptr);
}

void SG::D3D11RenderEngine::SetViewports(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT nrOfViewports = 0;

	for (auto& vp : job.viewports)
		viewports[nrOfViewports++] = GetViewport(vp, entity);

	context->RSSetViewports(nrOfViewports, viewports);
}

void SG::D3D11RenderEngine::SetStates(const SGRenderJob & job, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context)
{
	context->RSSetState(GetRasterizerState(job.rasterizerState, entity));
	//blendstate
	//depthstencilstate
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
		// FIX LATER
		break;
	case SG::D3D11DrawCallHandler::DrawType::DRAW_INDEXED_INSTANCED:
		// FIX LATER
		break;
	}

}

void SG::D3D11RenderEngine::SetConstantBuffersForShader(const std::vector<PipelineComponent>& buffers, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context, void(_stdcall ID3D11DeviceContext::*func)(UINT, UINT, ID3D11Buffer * const *))
{
	const UINT arrSize = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	ID3D11Buffer* bufferArr[arrSize] = {};
	UINT counter = 0;
	for (auto& cBuffer : buffers)
		bufferArr[counter++] = GetBuffer(cBuffer, entity, context);

	(context->*func)(0, arrSize, bufferArr);
}

void SG::D3D11RenderEngine::SetShaderResourceViewsForShader(const std::vector<PipelineComponent>& srvs, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context, void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11ShaderResourceView * const *))
{
	const UINT arrSize = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	ID3D11ShaderResourceView* srvArr[arrSize] = {};
	UINT counter = 0;
	for (auto& srv : srvs)
		srvArr[counter++] = GetSRV(srv, entity);

	(context->*func)(0, arrSize, srvArr);
}

void SG::D3D11RenderEngine::SetSamplerStatesForShader(const std::vector<PipelineComponent>& samplers, const SGGraphicalEntityID & entity, ID3D11DeviceContext * context, void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11SamplerState * const *))
{
	const UINT arrSize = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
	ID3D11SamplerState* samplerArr[arrSize] = {};
	UINT counter = 0;
	for (auto& sampler : samplers)
		samplerArr[counter++] = GetSamplerState(sampler, entity);

	(context->*func)(0, arrSize, samplerArr);
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

ID3D11ShaderResourceView * SG::D3D11RenderEngine::GetSRV(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	ID3D11ShaderResourceView* toReturn = nullptr;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = textureHandler->GetSRV(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = textureHandler->GetSRV(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = textureHandler->GetSRV(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11RenderTargetView* SG::D3D11RenderEngine::GetRTV(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	ID3D11RenderTargetView* toReturn = nullptr;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = textureHandler->GetRTV(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = textureHandler->GetRTV(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = textureHandler->GetRTV(component.resourceGuid, entity);
		entityMutex.unlock();
	}
	break;
	}

	return toReturn;
}

ID3D11DepthStencilView * SG::D3D11RenderEngine::GetDSV(const PipelineComponent & component, const SGGraphicalEntityID & entity)
{
	ID3D11DepthStencilView* toReturn = nullptr;
	switch (component.source)
	{
	case Association::GLOBAL:
	{
		toReturn = textureHandler->GetDSV(component.resourceGuid);
	}
	break;
	case Association::GROUP:
	{
		entityMutex.lock();
		SGGuid& groupGuid = graphicalEntities[entity].groupGuid;
		entityMutex.unlock();
		toReturn = textureHandler->GetDSV(component.resourceGuid, groupGuid);
	}
	break;
	case Association::ENTITY:
	{
		entityMutex.lock();
		toReturn = textureHandler->GetDSV(component.resourceGuid, entity);
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
		throw std::runtime_error("Error, this error should never happen, because it implies that an association that does not exist has been used for a drawcall");
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

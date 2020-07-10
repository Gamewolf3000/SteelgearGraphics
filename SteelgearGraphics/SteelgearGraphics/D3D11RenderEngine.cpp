#include "D3D11RenderEngine.h"
#include "D3D11CommonTypes.h"

SG::D3D11RenderEngine::D3D11RenderEngine(const SGRenderSettings & settings) : SGRenderEngine(settings)
{
	this->CreateDeviceAndContext(settings);
	this->CreateSwapChain(settings);
	//bufferHandler;
	//drawDataHandler;
	//samplerHandler;
	//shaderManager;
	//stateHandler;
	this->textureHandler = new D3D11TextureHandler(device);
	//pipelineHandler;
}

SG::D3D11RenderEngine::~D3D11RenderEngine()
{
	ReleaseCOM(device);
	ReleaseCOM(immediateContext);
	ReleaseCOM(swapChain);

	for (auto& context : defferedContexts)
		ReleaseCOM(context);
}

void SG::D3D11RenderEngine::CreateDeviceAndContext(const SGRenderSettings & settings)
{
	UINT flags = 0;

	if constexpr (DEBUG_VERSION)
		flags = D3D11_CREATE_DEVICE_DEBUG;

	if (settings.nrOfContexts <= 1)
		flags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

	if (FAILED(D3D11CreateDevice(settings.adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &device, NULL, &immediateContext)))
		throw std::runtime_error("Error creating device and immediate context");
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

	for (int i = 0; i < settings.backBufferSettings.nrOfBackBuffers; ++i)
	{
		ID3D11Texture2D* texture;
		swapChain->GetBuffer(i, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
		textureHandler->AddTexture2D(SGGuid(std::string("SG_BACKBUFFER_") + std::to_string(i)), texture);
	}
}

void SG::D3D11RenderEngine::SwapUpdateBuffer()
{
	//bufferHandler->SwapUpdateBuffer();
	textureHandler->SwapUpdateBuffer();
}

void SG::D3D11RenderEngine::SwapToWorkWithBuffer()
{
	//bufferHandler->SwapToWorkWithBuffer();
	textureHandler->SwapToWorkWithBuffer();
}

void SG::D3D11RenderEngine::HandleRenderJob(const std::vector<SGPipelineJob>& jobs)
{
}

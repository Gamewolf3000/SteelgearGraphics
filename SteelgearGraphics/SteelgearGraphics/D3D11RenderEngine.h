#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>

#include "SGRenderEngine.h"

#include "D3D11BufferHandler.h"

namespace SG
{
	class D3D11RenderEngine : public SGRenderEngine
	{
	public:
		D3D11RenderEngine(const SGRenderSettings& settings);
		~D3D11RenderEngine();


	private:
		ID3D11Device* device;
		ID3D11DeviceContext* immediateContext;
		std::vector<ID3D11DeviceContext> defferedContexts;

		D3D11BufferHandler* bufferHandler;
	};
}
#pragma once

#include <d3d11_4.h>

namespace SG
{
	enum class StateType
	{
		RASTERIZER,
		DEPTH_STENCIL,
		BLEND
	};

	struct D3D11StateData
	{
		StateType type;
		union
		{
			ID3D11RasterizerState* rasterizer;
			ID3D11DepthStencilState* depthStencil;
			ID3D11BlendState* blend;
		} state;

		D3D11StateData() = default;
		~D3D11StateData();

		D3D11StateData(D3D11StateData&& other);
		D3D11StateData& operator=(D3D11StateData&& other);
	};
}
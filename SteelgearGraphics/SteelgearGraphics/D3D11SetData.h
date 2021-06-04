#pragma once

#include <d3d11_4.h>

namespace SG
{
	struct DepthStencilSetData
	{
		UINT stencilRef;
	};

	struct BlendSetData
	{
		FLOAT blendFactor[4];
		UINT sampleMask;
	};

	struct D3D11SetData
	{
		union
		{
			DepthStencilSetData depthStencil;
			BlendSetData blend;
		};

		D3D11SetData() = default;
		~D3D11SetData() = default;

		D3D11SetData(D3D11SetData&& other) = default;
		D3D11SetData& operator=(D3D11SetData&& other) = default;
	};
}
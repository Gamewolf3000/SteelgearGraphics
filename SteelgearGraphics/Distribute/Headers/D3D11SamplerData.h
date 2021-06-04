#pragma once

#include <d3d11_4.h>

namespace SG
{
	struct D3D11SamplerData
	{
		ID3D11SamplerState* sampler = nullptr;

		D3D11SamplerData() = default;
		~D3D11SamplerData();

		D3D11SamplerData(D3D11SamplerData&& other);
		D3D11SamplerData& operator=(D3D11SamplerData&& other);
	};
}
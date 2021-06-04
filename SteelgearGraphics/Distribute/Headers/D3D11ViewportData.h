#pragma once

#include <d3d11_4.h>

namespace SG
{
	struct D3D11ViewportData
	{
		D3D11_VIEWPORT viewport;

		D3D11ViewportData() = default;
		~D3D11ViewportData() = default;

		D3D11ViewportData(D3D11ViewportData&& other) = default;
		D3D11ViewportData& operator=(D3D11ViewportData&& other) = default;
	};
}
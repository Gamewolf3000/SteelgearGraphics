#pragma once

#include <d3d11_4.h>

namespace SG
{
	struct D3D11InputLayoutData
	{
		ID3D11InputLayout* inputLayout = nullptr;

		D3D11InputLayoutData() = default;
		~D3D11InputLayoutData();

		D3D11InputLayoutData(D3D11InputLayoutData&& other);
		D3D11InputLayoutData& operator=(D3D11InputLayoutData&& other);
	};
}
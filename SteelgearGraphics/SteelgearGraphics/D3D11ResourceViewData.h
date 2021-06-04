#pragma once

#include <d3d11_4.h>

#include "SGGuid.h"

namespace SG
{
	enum class ResourceViewType
	{
		SRV,
		UAV,
		DSV,
		RTV
	};

	struct D3D11ResourceViewData
	{
		ResourceViewType type;
		union
		{
			ID3D11ShaderResourceView* srv;
			ID3D11UnorderedAccessView* uav;
			ID3D11RenderTargetView* rtv;
			ID3D11DepthStencilView* dsv;
		} view;
		SGGuid resourceGuid;

		D3D11ResourceViewData() = default;
		~D3D11ResourceViewData();

		D3D11ResourceViewData(D3D11ResourceViewData&& other);
		D3D11ResourceViewData& operator=(D3D11ResourceViewData&& other);
	};
}
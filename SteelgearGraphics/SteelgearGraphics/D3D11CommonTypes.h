#pragma once

#include "SGGuid.h"

#include <mutex>
#include <unordered_map>

#include <d3d11_4.h>
#include <dxgi1_6.h>

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
	};

	enum class UpdateStrategy
	{
		DISCARD,
		NO_OVERWRITE
	};

	struct UpdateData
	{
		void* data  = nullptr;
		size_t size = 0;
		UpdateStrategy strategy;
		UINT subresource;
	};

	enum class ComparisonFunction
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};

	template<class T>
	void ReleaseCOM(T* &comObject)
	{

		if (comObject != nullptr)
		{
			comObject->Release();
			comObject = nullptr;
		}
	}

	//https://stackoverflow.com/questions/40339138/convert-dxgi-format-to-a-bpp
	int GetFormatElementSize(DXGI_FORMAT format);

}
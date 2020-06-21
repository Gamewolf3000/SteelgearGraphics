#pragma once

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
		} view;
		SGGuid resourceGuid;
	};

	struct ResourceData
	{
		void* data  = nullptr;
		size_t size = 0;
	};

	enum class UpdateStrategy
	{
		DISCARD,
		NO_OVERWRITE
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
}
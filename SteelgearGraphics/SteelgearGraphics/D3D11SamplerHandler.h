#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"
#include "LockableUnorderedMap.h"


namespace SG
{
	enum class Filter
	{
		MIN_MAG_MIP_POINT,
		MIN_MAG_POINT_MIP_LINEAR,
		MIN_POINT_MAG_LINEAR_MIP_POINT,
		MIN_POINT_MAG_MIP_LINEAR,
		MIN_LINEAR_MAG_MIP_POINT,
		MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MIN_MAG_LINEAR_MIP_POINT,
		MIN_MAG_MIP_LINEAR,
		ANISOTROPIC,
		COMPARISON_MIN_MAG_MIP_POINT,
		COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
		COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
		COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
		COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
		COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		COMPARISON_MIN_MAG_MIP_LINEAR,
		COMPARISON_ANISOTROPIC,
		MINIMUM_MIN_MAG_MIP_POINT,
		MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
		MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
		MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
		MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
		MINIMUM_MIN_MAG_MIP_LINEAR,
		MINIMUM_ANISOTROPIC,
		MAXIMUM_MIN_MAG_MIP_POINT,
		MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
		MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
		MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
		MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
		MAXIMUM_MIN_MAG_MIP_LINEAR,
		MAXIMUM_ANISOTROPIC
	};

	enum class TextureAdressMode
	{
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE
	};

	class D3D11SamplerHandler : public SGGraphicsHandler
	{
	public:

		D3D11SamplerHandler(ID3D11Device* device);
		~D3D11SamplerHandler();

		SGResult CreateSampler(const SGGuid& guid, Filter filter, TextureAdressMode adressU, TextureAdressMode adressV, TextureAdressMode adressW, 
			FLOAT mipLODBias, UINT maxAnisotropy, ComparisonFunction comparisonFunc, FLOAT borderColor[4], FLOAT minLOD, FLOAT maxLOD);

		SGResult BindSamplerToEntity(const SGGraphicalEntityID& entity, const SGGuid& samplerGuid, const SGGuid& bindGuid);
		SGResult BindSamplerToGroup(const SGGuid& group, const SGGuid& samplerGuid, const SGGuid& bindGuid);

	private:

		friend class D3D11RenderEngine;

		struct D3D11SamplerData
		{
			ID3D11SamplerState* sampler = nullptr;
		};

		LockableUnorderedMap<SGGuid, D3D11SamplerData> samplers;

		ID3D11Device* device;

		D3D11_FILTER TranslateFilter(const Filter& filter);
		D3D11_TEXTURE_ADDRESS_MODE TranslateAdressMode(const TextureAdressMode& adressMode);
		D3D11_COMPARISON_FUNC TranslateComparisonFunction(const ComparisonFunction& compFunc);

		ID3D11SamplerState* GetSamplerState(const SGGuid& guid);
		ID3D11SamplerState* GetSamplerState(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11SamplerState* GetSamplerState(const SGGuid& guid, const SGGraphicalEntityID& entity);
	};
}
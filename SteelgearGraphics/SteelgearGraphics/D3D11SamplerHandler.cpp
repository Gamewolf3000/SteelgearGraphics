#include "D3D11SamplerHandler.h"

SG::D3D11SamplerHandler::D3D11SamplerHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11SamplerHandler::~D3D11SamplerHandler()
{
	for (auto& sampler : samplers)
		ReleaseCOM(sampler.second.sampler);
}

SG::SGResult SG::D3D11SamplerHandler::CreateSampler(const SGGuid & guid, Filter filter, TextureAdressMode adressU, TextureAdressMode adressV, TextureAdressMode adressW, FLOAT mipLODBias, UINT maxAnisotropy, ComparisonFunction comparisonFunc, FLOAT borderColor[4], FLOAT minLOD, FLOAT maxLOD)
{
	D3D11_SAMPLER_DESC desc;
	desc.Filter = TranslateFilter(filter);
	desc.AddressU = TranslateAdressMode(adressU);
	desc.AddressV = TranslateAdressMode(adressV);
	desc.AddressW = TranslateAdressMode(adressW);
	desc.MipLODBias = mipLODBias;
	desc.MaxAnisotropy = maxAnisotropy;
	desc.ComparisonFunc = TranslateComparisonFunction(comparisonFunc);
	memcpy(desc.BorderColor, borderColor, sizeof(FLOAT) * 4);
	desc.MinLOD = minLOD;
	desc.MaxLOD = maxLOD;

	D3D11SamplerData toStore;
	if(FAILED(device->CreateSamplerState(&desc, &toStore.sampler)))
		return SGResult::FAIL;

	samplers.lock();
	samplers[guid] = toStore;
	samplers.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11SamplerHandler::BindSamplerToEntity(const SGGraphicalEntityID & entity, const SGGuid & samplerGuid, const SGGuid & bindGuid)
{
	entityData.lock();
	entityData[entity][bindGuid] = samplerGuid;
	entityData.unlock();

	if constexpr (DEBUG_VERSION)
	{
		samplers.lock();
		if (samplers.find(samplerGuid) == samplers.end())
		{
			samplers.unlock();
			return SGResult::GUID_MISSING;
		}
		samplers.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11SamplerHandler::BindSamplerToGroup(const SGGuid & group, const SGGuid & samplerGuid, const SGGuid & bindGuid)
{
	groupData.lock();
	groupData[group][bindGuid] = samplerGuid;
	groupData.unlock();

	if constexpr (DEBUG_VERSION)
	{
		samplers.lock();
		if (samplers.find(samplerGuid) == samplers.end())
		{
			samplers.unlock();
			return SGResult::GUID_MISSING;
		}
		samplers.unlock();
	}

	return SGResult::OK;
}

D3D11_FILTER SG::D3D11SamplerHandler::TranslateFilter(const Filter & filter)
{
	switch (filter)
	{
	case SG::Filter::MIN_MAG_MIP_POINT:
		return D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case SG::Filter::MIN_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::MIN_POINT_MAG_MIP_LINEAR:
		return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		break;
	case SG::Filter::MIN_LINEAR_MAG_MIP_POINT:
		return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		break;
	case SG::Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::MIN_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::MIN_MAG_MIP_LINEAR:
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case SG::Filter::ANISOTROPIC:
		return D3D11_FILTER_ANISOTROPIC;
		break;
	case SG::Filter::COMPARISON_MIN_MAG_MIP_POINT:
		return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		break;
	case SG::Filter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
		return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		break;
	case SG::Filter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
		return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		break;
	case SG::Filter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
		return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		break;
	case SG::Filter::COMPARISON_ANISOTROPIC:
		return D3D11_FILTER_COMPARISON_ANISOTROPIC;
		break;
	case SG::Filter::MINIMUM_MIN_MAG_MIP_POINT:
		return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
		break;
	case SG::Filter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
		return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
		break;
	case SG::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
		return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
		break;
	case SG::Filter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
		return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
		break;
	case SG::Filter::MINIMUM_ANISOTROPIC:
		return D3D11_FILTER_MINIMUM_ANISOTROPIC;
		break;
	case SG::Filter::MAXIMUM_MIN_MAG_MIP_POINT:
		return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
		break;
	case SG::Filter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
		return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
		break;
	case SG::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
		return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
		break;
	case SG::Filter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		break;
	case SG::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
		return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case SG::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
		return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
		break;
	case SG::Filter::MAXIMUM_ANISOTROPIC:
		return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
		break;
	default:
		throw std::runtime_error("Error, unknown filter type");
		break;
	}
}

D3D11_TEXTURE_ADDRESS_MODE SG::D3D11SamplerHandler::TranslateAdressMode(const TextureAdressMode & adressMode)
{
	switch (adressMode)
	{
	case SG::TextureAdressMode::WRAP:
		return D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case SG::TextureAdressMode::MIRROR:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	case SG::TextureAdressMode::CLAMP:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case SG::TextureAdressMode::BORDER:
		return D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	case SG::TextureAdressMode::MIRROR_ONCE:
		return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		break;
	default:
		throw std::runtime_error("Error, unknown texture adress mode");
		break;
	}
}

D3D11_COMPARISON_FUNC SG::D3D11SamplerHandler::TranslateComparisonFunction(const ComparisonFunction & compFunc)
{
	switch (compFunc)
	{
	case SG::ComparisonFunction::NEVER:
		return D3D11_COMPARISON_NEVER;
		break;
	case SG::ComparisonFunction::LESS:
		return D3D11_COMPARISON_LESS;
		break;
	case SG::ComparisonFunction::EQUAL:
		return D3D11_COMPARISON_EQUAL;
		break;
	case SG::ComparisonFunction::LESS_EQUAL:
		return D3D11_COMPARISON_LESS_EQUAL;
		break;
	case SG::ComparisonFunction::GREATER:
		return D3D11_COMPARISON_GREATER;
		break;
	case SG::ComparisonFunction::NOT_EQUAL:
		return D3D11_COMPARISON_NOT_EQUAL;
		break;
	case SG::ComparisonFunction::GREATER_EQUAL:
		return D3D11_COMPARISON_GREATER_EQUAL;
		break;
	case SG::ComparisonFunction::ALWAYS:
		return D3D11_COMPARISON_ALWAYS;
		break;
	default:
		throw std::runtime_error("Error, unknown comparison function");
		break;
	}
}

ID3D11SamplerState * SG::D3D11SamplerHandler::GetSamplerState(const SGGuid & guid)
{
	samplers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (samplers.find(guid) == samplers.end())
		{
			samplers.unlock();
			throw std::runtime_error("Error, missing guid when fetching sampler state");
		}
	}

	ID3D11SamplerState* toReturn = samplers[guid].sampler;
	samplers.unlock();
	return toReturn;
}

ID3D11SamplerState * SG::D3D11SamplerHandler::GetSamplerState(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	samplers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (samplers.find(groupData[groupGuid][guid].GetActive()) == samplers.end())
		{
			samplers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching sampler state");
		}
	}

	ID3D11SamplerState* toReturn = samplers[groupData[groupGuid][guid].GetActive()].sampler;
	samplers.unlock();
	groupData.unlock();
	return toReturn;
}

ID3D11SamplerState * SG::D3D11SamplerHandler::GetSamplerState(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	samplers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (samplers.find(entityData[entity][guid].GetActive()) == samplers.end())
		{
			samplers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching sampler state");
		}
	}

	ID3D11SamplerState* toReturn = samplers[entityData[entity][guid].GetActive()].sampler;
	samplers.unlock();
	entityData.unlock();
	return toReturn;
}

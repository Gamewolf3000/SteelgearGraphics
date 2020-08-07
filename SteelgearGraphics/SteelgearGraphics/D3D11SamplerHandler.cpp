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
	// CONTINUE https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc

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
	}
}

D3D11_TEXTURE_ADDRESS_MODE SG::D3D11SamplerHandler::TranslateAdressMode(const TextureAdressMode & adressMode)
{
	switch (TextureAdressMode)
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
	}
}

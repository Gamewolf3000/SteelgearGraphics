#include "D3D11TextureHandler.h"

SG::D3D11TextureHandler::D3D11TextureHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11TextureHandler::~D3D11TextureHandler()
{
}

SG::SGResult SG::D3D11TextureHandler::CreateTexture2D(const SGGuid & guid, const SGTextureData & generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC & sampleDesc, bool texturecube)
{
	(void)texturecube; // FIX LATER

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = generalSettings.mipLevels;
	desc.ArraySize = arraySize;
	desc.Format = generalSettings.format;
	desc.SampleDesc = sampleDesc;
	SetUsageAndCPUAccessFlags(generalSettings, desc.Usage, desc.CPUAccessFlags);
	SetBindflags(generalSettings, desc.BindFlags);
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = generalSettings.data;
	data.SysMemPitch = width * GetFormatElementSize(generalSettings.format);

	ID3D11Texture2D* texture;

	device->CreateTexture2D(&desc, &data, &texture);

	D3D11TextureData toStore;
	toStore.type = arraySize <= 1 ? TextureType::TEXTURE_2D : TextureType::TEXTURE_ARRAY_2D;
	toStore.texture.texture2D = texture;
	textures.lock();
	textures[guid] = toStore;
	textures.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateRTV(const SGGuid & guid, const SGGuid & textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
		return SGResult::GUID_MISSING;

	textures.unlock();

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.Format = format;
	desc.ViewDimension = GetRTVDimension(storedData->second.type);

	switch (storedData->second.type)
	{
	case TextureType::TEXTURE_1D:
		desc.Texture1D.MipSlice = mipSlice;
		break;
	case TextureType::TEXTURE_2D:
		desc.Texture2D.MipSlice = mipSlice;
		break;
	case TextureType::TEXTURE_3D:
		desc.Texture3D.MipSlice = mipSlice;
		desc.Texture3D.FirstWSlice = firstWSlice;
		desc.Texture3D.WSize = wSize;
		break;
	case TextureType::TEXTURE_MULTISAMPLED_2D:
		// Do nothing
		break;
	default:
		throw std::runtime_error("CreateRTV called refering to texture of incorrect type");
		break;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::RTV;
	device->CreateRenderTargetView(storedData->second.texture.texture2D, &desc, &toStore.view.rtv);
	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateDSV(const SGGuid & guid, const SGGuid & textureGuid, DXGI_FORMAT format, bool readOnlyDepth, bool readOnlyStencil, UINT mipSlice)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
		return SGResult::GUID_MISSING;

	textures.unlock();


	D3D11_DEPTH_STENCIL_VIEW_DESC  desc;
	desc.Format = format;
	desc.ViewDimension = GetDSVDimension(storedData->second.type);
	desc.Flags = 0 | (readOnlyDepth ? D3D11_DSV_READ_ONLY_DEPTH : 0) | (readOnlyStencil ? D3D11_DSV_READ_ONLY_STENCIL : 0);

	switch (storedData->second.type)
	{
	case TextureType::TEXTURE_1D:
		desc.Texture1D.MipSlice = mipSlice;
		break;
	case TextureType::TEXTURE_2D:
		desc.Texture2D.MipSlice = mipSlice;
		break;
	case TextureType::TEXTURE_MULTISAMPLED_2D:
		// Do nothing
		break;
	default:
		throw std::runtime_error("CreateDSV called refering to texture of incorrect type");
		break;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::DSV;
	device->CreateDepthStencilView(storedData->second.texture.texture2D, &desc, &toStore.view.dsv);
	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

void SG::D3D11TextureHandler::AddTexture2D(const SGGuid & guid, ID3D11Texture2D * texture)
{
	D3D11TextureData toStore;
	toStore.type = TextureType::TEXTURE_2D;
	toStore.texture.texture2D = texture;
	textures.lock();
	textures[guid] = toStore;
	textures.unlock();
}

void SG::D3D11TextureHandler::SwapUpdateBuffer()
{
	std::swap(toUpdate, toUseNext);
}

void SG::D3D11TextureHandler::SwapToWorkWithBuffer()
{
	std::swap(toWorkWith, toUseNext);
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid)
{
	views.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (views.find(guid) == views.end())
			throw std::runtime_error("Error fetching rtv, guid does not exist");

		if (views[guid].type != ResourceViewType::RTV)
			throw std::runtime_error("Error fetching rtv, guid does not match an rtv");
	}

	auto toReturn = views[guid].view.rtv;
	views.unlock();

	return toReturn;
}

void SG::D3D11TextureHandler::SetUsageAndCPUAccessFlags(const SGTextureData & generalSettings, D3D11_USAGE & usage, UINT & cpuAccessFlags)
{
	if (!generalSettings.cpuWritable && !generalSettings.cpuReadable && !generalSettings.gpuWritable)
	{
		usage = D3D11_USAGE_IMMUTABLE;
		cpuAccessFlags = 0;
	}
	else if (generalSettings.cpuWritable && !generalSettings.cpuReadable && !generalSettings.gpuWritable)
	{
		usage = D3D11_USAGE_DYNAMIC;
		cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (!generalSettings.cpuWritable && !generalSettings.cpuReadable && generalSettings.gpuWritable)
	{
		usage = D3D11_USAGE_DEFAULT;
		cpuAccessFlags = 0;
	}
	else if (generalSettings.cpuWritable && generalSettings.cpuReadable && generalSettings.gpuWritable)
	{
		usage = D3D11_USAGE_STAGING;
		cpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}
	else
	{
		throw std::runtime_error("Combination of CPU and GPU write/read flags do not work");
	}
}

void SG::D3D11TextureHandler::SetBindflags(const SGTextureData & generalSettings, UINT & flags)
{
	flags = 0;

	for (auto& binding : generalSettings.textureBindings)
	{
		switch (binding)
		{
		case TextureBinding::SHADER_RESOURCE:
			flags |= D3D11_BIND_SHADER_RESOURCE;
			break;
		case TextureBinding::RENDER_TARGET:
			flags |= D3D11_BIND_RENDER_TARGET;
			break;
		case TextureBinding::DEPTH_STENCIL:
			flags |= D3D11_BIND_DEPTH_STENCIL;
			break;
		case TextureBinding::UNORDERED_ACCESS:
			flags |= D3D11_BIND_UNORDERED_ACCESS;
			break;
		default:
			break;
		}
	}
}

D3D11_RTV_DIMENSION SG::D3D11TextureHandler::GetRTVDimension(TextureType type)
{
	D3D11_RTV_DIMENSION toReturn = D3D11_RTV_DIMENSION_UNKNOWN;

	switch (type)
	{
	case SG::D3D11TextureHandler::TextureType::TEXTURE_1D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE1D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_MULTISAMPLED_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_3D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE3D;
		break;
	default:
		throw std::runtime_error("Trying to create a RTV with a texture of incompatible type");
		break;
	}

	return toReturn;
}

D3D11_DSV_DIMENSION SG::D3D11TextureHandler::GetDSVDimension(TextureType type)
{
	D3D11_DSV_DIMENSION toReturn = D3D11_DSV_DIMENSION_UNKNOWN;

	switch (type)
	{
	case SG::D3D11TextureHandler::TextureType::TEXTURE_1D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE1D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_MULTISAMPLED_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		break;
	default:
		throw std::runtime_error("Trying to create a DSV with a texture of incompatible type");
		break;
	}

	return toReturn;
}

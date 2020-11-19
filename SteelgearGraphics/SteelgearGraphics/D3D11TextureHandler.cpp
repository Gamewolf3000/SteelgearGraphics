#include "D3D11TextureHandler.h"

SG::D3D11TextureHandler::D3D11TextureHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11TextureHandler::~D3D11TextureHandler()
{
	for (auto& texture : textures)
	{
		switch (texture.second.type)
		{
		case TextureType::TEXTURE_1D:
			ReleaseCOM(texture.second.texture.texture1D);
			break;
		case TextureType::TEXTURE_2D:
			ReleaseCOM(texture.second.texture.texture2D);
			break;
		case TextureType::TEXTURE_3D:
			ReleaseCOM(texture.second.texture.texture3D);
			break;
		default:
			break;
		}
	}

	for (auto& view : views)
	{
		switch (view.second.type)
		{
		case ResourceViewType::RTV:
			ReleaseCOM(view.second.view.rtv);
			break;
		case ResourceViewType::DSV:
			ReleaseCOM(view.second.view.dsv);
			break;
		case ResourceViewType::SRV:
			ReleaseCOM(view.second.view.srv);
			break;
		case ResourceViewType::UAV:
			ReleaseCOM(view.second.view.uav);
			break;
		default:
			break;
		}
	}
}

SG::SGResult SG::D3D11TextureHandler::CreateTexture2D(const SGGuid & guid, const SGTextureData & generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC & sampleDesc, bool texturecube)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = generalSettings.mipLevels;
	desc.ArraySize = arraySize;
	desc.Format = generalSettings.format;
	desc.SampleDesc = sampleDesc;
	SetUsageAndCPUAccessFlags(generalSettings, desc.Usage, desc.CPUAccessFlags);
	SetBindflags(generalSettings, desc.BindFlags);
	desc.MiscFlags = 0 | (generalSettings.generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0) | (generalSettings.resourceClamp ? D3D11_RESOURCE_MISC_RESOURCE_CLAMP : 0);
	desc.MiscFlags |= (texturecube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0);

	ID3D11Texture2D* texture;

	if (generalSettings.data.size())
	{
		if (generalSettings.data.size() < arraySize * generalSettings.mipLevels)
			return SG::SGResult::FAIL;

		// does not take into account the mip maps but the stack cannot store an infinite amount
		D3D11_SUBRESOURCE_DATA data[D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION]; 
		int formatSize = GetFormatElementSize(generalSettings.format);

		for (unsigned int i = 0; i < arraySize; ++i)
		{
			int mipLevelDivision = 1;
			
			for (unsigned int j = 0; j < generalSettings.mipLevels; ++j)
			{
				data[i * generalSettings.mipLevels + j].pSysMem = generalSettings.data[i * generalSettings.mipLevels + j];
				data[i * generalSettings.mipLevels + j].SysMemPitch = (width / mipLevelDivision) * formatSize;
				data[i * generalSettings.mipLevels + j].SysMemSlicePitch = 0;
				mipLevelDivision *= 2;
			}
		}

		if (FAILED(device->CreateTexture2D(&desc, data, &texture)))
			return SGResult::FAIL;
	}
	else
	{
		if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture)))
			return SGResult::FAIL;
	}

	D3D11TextureData toStore;
	toStore.type = arraySize <= 1 ? TextureType::TEXTURE_2D : TextureType::TEXTURE_ARRAY_2D;
	toStore.texture.texture2D = texture;
	textures.lock();
	textures[guid] = std::move(toStore);
	textures.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateSRV(const SGGuid & guid, const SGGuid & textureGuid, DXGI_FORMAT format, UINT mostDetailedMip, UINT mipLevels)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
	{
		textures.unlock();
		return SGResult::GUID_MISSING;
	}

	textures.unlock();

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = format;
	desc.ViewDimension = GetSRVDimension(storedData->second.type);

	switch (storedData->second.type)
	{
	case TextureType::TEXTURE_1D:
		desc.Texture1D.MostDetailedMip = mostDetailedMip;
		desc.Texture1D.MipLevels = mipLevels;
		break;
	case TextureType::TEXTURE_2D:
		desc.Texture2D.MostDetailedMip = mostDetailedMip;
		desc.Texture2D.MipLevels = mipLevels;
		break;
	case TextureType::TEXTURE_3D:
		desc.Texture3D.MostDetailedMip = mostDetailedMip;
		desc.Texture3D.MipLevels = mipLevels;
		break;
	case TextureType::TEXTURE_MULTISAMPLED_2D:
		// Do nothing
		break;
	case TextureType::TEXTURE_CUBE:
		desc.TextureCube.MostDetailedMip = mostDetailedMip;
		desc.TextureCube.MipLevels = mipLevels;
		break;
	default:
		throw std::runtime_error("CreateSRV called refering to texture of incorrect type");
		break;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::SRV;

	if (FAILED(device->CreateShaderResourceView(storedData->second.texture.texture2D, &desc, &toStore.view.srv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateSRV(const SGGuid & guid, const SGGuid & textureGuid)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
	{
		textures.unlock();
		return SGResult::GUID_MISSING;
	}

	textures.unlock();

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::SRV;

	if (FAILED(device->CreateShaderResourceView(storedData->second.texture.texture2D, nullptr, &toStore.view.srv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateUAV(const SGGuid & guid, const SGGuid & textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
	{
		textures.unlock();
		return SGResult::GUID_MISSING;
	}

	textures.unlock();

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = format;
	desc.ViewDimension = GetUAVDimension(storedData->second.type);

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
	default:
		throw std::runtime_error("CreateUAV called refering to texture of incorrect type");
		break;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::UAV;

	if (FAILED(device->CreateUnorderedAccessView(storedData->second.texture.texture2D, &desc, &toStore.view.uav)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateUAV(const SGGuid & guid, const SGGuid & textureGuid)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
	{
		textures.unlock();
		return SGResult::GUID_MISSING;
	}

	textures.unlock();

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::UAV;

	if (FAILED(device->CreateUnorderedAccessView(storedData->second.texture.texture2D, nullptr, &toStore.view.uav)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateRTV(const SGGuid & guid, const SGGuid & textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice, UINT wSize)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
	{
		textures.unlock();
		return SGResult::GUID_MISSING;
	}

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

	if (FAILED(device->CreateRenderTargetView(storedData->second.texture.texture2D, &desc, &toStore.view.rtv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateRTV(const SGGuid & guid, const SGGuid & textureGuid)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
	{
		textures.unlock();
		return SGResult::GUID_MISSING;
	}

	textures.unlock();

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::RTV;

	if (FAILED(device->CreateRenderTargetView(storedData->second.texture.texture2D, nullptr, &toStore.view.rtv)))
		return SGResult::FAIL;

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


	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
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

	if (FAILED(device->CreateDepthStencilView(storedData->second.texture.texture2D, &desc, &toStore.view.dsv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateDSV(const SGGuid & guid, const SGGuid & textureGuid)
{
	textures.lock();
	auto storedData = textures.find(textureGuid);

	if (storedData == textures.end())
		return SGResult::GUID_MISSING;

	textures.unlock();

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::DSV;

	if (FAILED(device->CreateDepthStencilView(storedData->second.texture.texture2D, nullptr, &toStore.view.dsv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::BindViewToEntity(const SGGraphicalEntityID & entity, const SGGuid & viewGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, viewGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		views.lock();
		if (views.find(viewGuid) == views.end())
		{
			views.unlock();
			return SGResult::GUID_MISSING;
		}
		views.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::BindViewToGroup(const SGGuid & group, const SGGuid & viewGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, viewGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		views.lock();
		if (views.find(viewGuid) == views.end())
		{
			views.unlock();
			return SGResult::GUID_MISSING;
		}
		views.unlock();
	}

	return SGResult::OK;
}

void SG::D3D11TextureHandler::AddTexture2D(const SGGuid & guid, ID3D11Texture2D * texture)
{
	D3D11TextureData toStore;
	toStore.type = TextureType::TEXTURE_2D;
	toStore.texture.texture2D = texture;
	textures.lock();
	textures[guid] = std::move(toStore);
	textures.unlock();
}

void SG::D3D11TextureHandler::SwapUpdateBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapUpdateBuffer();
	
	for (auto& guid : updatedFrameBuffer)
		textures[guid].updatedData.SwitchUpdateBuffer();

	updatedTotalBuffer.insert(updatedTotalBuffer.end(), updatedFrameBuffer.begin(), updatedFrameBuffer.end());
	updatedFrameBuffer.clear();
}

void SG::D3D11TextureHandler::SwapToWorkWithBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapToWorkWithBuffer();
	
	for (auto& guid : updatedTotalBuffer)
		textures[guid].updatedData.SwitchActiveBuffer();

	updatedTotalBuffer.clear();
}

ID3D11ShaderResourceView * SG::D3D11TextureHandler::GetSRV(const SGGuid & guid)
{
	views.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (views.find(guid) == views.end())
		{
			views.unlock();
			throw std::runtime_error("Error fetching srv, guid does not exist");
		}

		if (views[guid].type != ResourceViewType::SRV)
		{
			views.unlock();
			throw std::runtime_error("Error fetching srv, guid does not match an srv");
		}
	}

	auto toReturn = views[guid].view.srv;
	views.unlock();

	return toReturn;
}

ID3D11ShaderResourceView * SG::D3D11TextureHandler::GetSRV(const SGGuid & guid, const SGGuid & groupGuid)
{
	views.lock();
	groupData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (groupData.find(groupGuid) == groupData.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching srv, group does not exist");
		}

		if (groupData[groupGuid].find(guid) == groupData[groupGuid].end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching srv, group does not have the guid");
		}

		if (views.find(groupData[groupGuid][guid].GetActive()) == views.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching srv, guid does not exist");
		}

		if (views[groupData[groupGuid][guid].GetActive()].type != ResourceViewType::SRV)
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching srv, guid does not match an srv");
		}
	}

	auto toReturn = views[groupData[groupGuid][guid].GetActive()].view.srv;
	groupData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11ShaderResourceView * SG::D3D11TextureHandler::GetSRV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	views.lock();
	entityData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (entityData.find(entity) == entityData.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching srv, entity does not exist");
		}

		if (entityData[entity].find(guid) == entityData[entity].end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching srv, entity does not have the guid");
		}

		if (views.find(entityData[entity][guid].GetActive()) == views.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching srv, guid does not exist");
		}

		if (views[entityData[entity][guid].GetActive()].type != ResourceViewType::SRV)
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching srv, guid does not match an srv");
		}
	}

	auto toReturn = views[entityData[entity][guid].GetActive()].view.srv;
	entityData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11UnorderedAccessView * SG::D3D11TextureHandler::GetUAV(const SGGuid & guid)
{
	views.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (views.find(guid) == views.end())
		{
			views.unlock();
			throw std::runtime_error("Error fetching uav, guid does not exist");
		}

		if (views[guid].type != ResourceViewType::UAV)
		{
			views.unlock();
			throw std::runtime_error("Error fetching uav, guid does not match an uav");
		}
	}

	auto toReturn = views[guid].view.uav;
	views.unlock();

	return toReturn;
}

ID3D11UnorderedAccessView * SG::D3D11TextureHandler::GetUAV(const SGGuid & guid, const SGGuid & groupGuid)
{
	views.lock();
	groupData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (groupData.find(groupGuid) == groupData.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching uav, group does not exist");
		}

		if (groupData[groupGuid].find(guid) == groupData[groupGuid].end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching uav, group does not have the guid");
		}

		if (views.find(groupData[groupGuid][guid].GetActive()) == views.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching uav, guid does not exist");
		}

		if (views[groupData[groupGuid][guid].GetActive()].type != ResourceViewType::UAV)
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching uav, guid does not match an uav");
		}
	}

	auto toReturn = views[groupData[groupGuid][guid].GetActive()].view.uav;
	groupData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11UnorderedAccessView * SG::D3D11TextureHandler::GetUAV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	views.lock();
	entityData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (entityData.find(entity) == entityData.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching uav, entity does not exist");
		}

		if (entityData[entity].find(guid) == entityData[entity].end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching uav, entity does not have the guid");
		}

		if (views.find(entityData[entity][guid].GetActive()) == views.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching uav, guid does not exist");
		}

		if (views[entityData[entity][guid].GetActive()].type != ResourceViewType::UAV)
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching uav, guid does not match an uav");
		}
	}

	auto toReturn = views[entityData[entity][guid].GetActive()].view.uav;
	entityData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid)
{
	views.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (views.find(guid) == views.end())
		{
			views.unlock();
			throw std::runtime_error("Error fetching rtv, guid does not exist");
		}

		if (views[guid].type != ResourceViewType::RTV)
		{
			views.unlock();
			throw std::runtime_error("Error fetching rtv, guid does not match an rtv");
		}
	}

	auto toReturn = views[guid].view.rtv;
	views.unlock();

	return toReturn;
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid, const SGGuid & groupGuid)
{
	views.lock();
	groupData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (groupData.find(groupGuid) == groupData.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rtv, group does not exist");
		}

		if (groupData[groupGuid].find(guid) == groupData[groupGuid].end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rtv, group does not have the guid");
		}

		if (views.find(groupData[groupGuid][guid].GetActive()) == views.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rtv, guid does not exist");
		}

		if (views[groupData[groupGuid][guid].GetActive()].type != ResourceViewType::RTV)
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rtv, guid does not match an rtv");
		}
	}

	auto toReturn = views[groupData[groupGuid][guid].GetActive()].view.rtv;
	groupData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	views.lock();
	entityData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (entityData.find(entity) == entityData.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rtv, entity does not exist");
		}

		if (entityData[entity].find(guid) == entityData[entity].end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rtv, entity does not have the guid");
		}

		if (views.find(entityData[entity][guid].GetActive()) == views.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rtv, guid does not exist");
		}

		if (views[entityData[entity][guid].GetActive()].type != ResourceViewType::RTV)
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rtv, guid does not match an rtv");
		}
	}

	auto toReturn = views[entityData[entity][guid].GetActive()].view.rtv;
	entityData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11DepthStencilView * SG::D3D11TextureHandler::GetDSV(const SGGuid & guid)
{
	views.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (views.find(guid) == views.end())
		{
			views.unlock();
			throw std::runtime_error("Error fetching dsv, guid does not exist");
		}

		if (views[guid].type != ResourceViewType::DSV)
		{
			views.unlock();
			throw std::runtime_error("Error fetching dsv, guid does not match a dsv");
		}
	}

	auto toReturn = views[guid].view.dsv;
	views.unlock();

	return toReturn;
}

ID3D11DepthStencilView * SG::D3D11TextureHandler::GetDSV(const SGGuid & guid, const SGGuid & groupGuid)
{
	views.lock();
	groupData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (groupData.find(groupGuid) == groupData.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching dsv, group does not exist");
		}

		if (groupData[groupGuid].find(guid) == groupData[groupGuid].end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching dsv, group does not have the guid");
		}

		if (views.find(groupData[groupGuid][guid].GetActive()) == views.end())
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching dsv, guid does not exist");
		}

		if (views[groupData[groupGuid][guid].GetActive()].type != ResourceViewType::RTV)
		{
			views.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching dsv, guid does not match a dsv");
		}
	}

	auto toReturn = views[groupData[groupGuid][guid].GetActive()].view.dsv;
	groupData.unlock();
	views.unlock();

	return toReturn;
}

ID3D11DepthStencilView * SG::D3D11TextureHandler::GetDSV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	views.lock();
	entityData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (entityData.find(entity) == entityData.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching dsv, entity does not exist");
		}

		if (entityData[entity].find(guid) == entityData[entity].end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching dsv, entity does not have the guid");
		}

		if (views.find(entityData[entity][guid].GetActive()) == views.end())
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching dsv, guid does not exist");
		}

		if (views[entityData[entity][guid].GetActive()].type != ResourceViewType::RTV)
		{
			views.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching dsv, guid does not match an dsv");
		}
	}

	auto toReturn = views[entityData[entity][guid].GetActive()].view.dsv;
	entityData.unlock();
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

D3D11_SRV_DIMENSION SG::D3D11TextureHandler::GetSRVDimension(TextureType type)
{
	D3D11_SRV_DIMENSION toReturn = D3D11_SRV_DIMENSION_UNKNOWN;

	switch (type)
	{
	case SG::D3D11TextureHandler::TextureType::TEXTURE_1D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE1D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_MULTISAMPLED_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_3D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE3D;
		break;
	case ::SG::D3D11TextureHandler::TextureType::TEXTURE_CUBE:
		toReturn = D3D11_SRV_DIMENSION_TEXTURECUBE;
		break;
	case ::SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_CUBE:
		toReturn = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
		break;
	default:
		throw std::runtime_error("Trying to create an SRV with a texture of incompatible type");
		break;
	}

	return toReturn;
}

D3D11_UAV_DIMENSION SG::D3D11TextureHandler::GetUAVDimension(TextureType type)
{
	D3D11_UAV_DIMENSION toReturn = D3D11_UAV_DIMENSION_UNKNOWN;

	switch (type)
	{
	case SG::D3D11TextureHandler::TextureType::TEXTURE_1D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE1D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_2D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE2D;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::D3D11TextureHandler::TextureType::TEXTURE_3D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE3D;
		break;
	default:
		throw std::runtime_error("Trying to create an UAV with a texture of incompatible type");
		break;
	}

	return toReturn;
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

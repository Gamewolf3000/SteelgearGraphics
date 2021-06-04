#include "D3D11TextureHandler.h"

SG::D3D11TextureHandler::D3D11TextureHandler(ID3D11Device * device)
{
	this->device = device;
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
	toStore.type = (arraySize > 1) ? (texturecube ? TextureType::TEXTURE_CUBE : TextureType::TEXTURE_ARRAY_2D) : TextureType::TEXTURE_2D;
	toStore.texture.texture2D = texture;
	textures.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11TextureHandler::RemoveTexture(const SGGuid& guid)
{
	textures.RemoveElement(guid);
}

SG::SGResult SG::D3D11TextureHandler::CreateSRV(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format, std::optional<TextureType> viewDimension,
	std::optional<UINT> mostDetailedMip, std::optional<UINT> mipLevels)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetSRVDimension(*viewDimension) : GetSRVDimension(textureData.type);

	switch (desc.ViewDimension)
	{
	case D3D11_SRV_DIMENSION_TEXTURE1D:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1D.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.Texture1D.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc1D.MipLevels;
		break;
	case D3D11_SRV_DIMENSION_TEXTURE2D:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2D.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.Texture2D.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc2D.MipLevels;
		break;
	case D3D11_SRV_DIMENSION_TEXTURE3D:
		desc.Format = format ? *format : resourceDesc.desc3D.Format;
		desc.Texture3D.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.Texture3D.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc3D.MipLevels;
		break;
	case D3D11_SRV_DIMENSION_TEXTURE2DMS:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		break;
	case D3D11_SRV_DIMENSION_TEXTURECUBE:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.TextureCube.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.TextureCube.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc2D.MipLevels;
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::SRV;

	if (FAILED(device->CreateShaderResourceView(textureData.texture.texture2D, &desc, &toStore.view.srv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateSRVTextureArray(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format, std::optional<TextureType> viewDimension, std::optional<UINT> mostDetailedMip,
	std::optional<UINT> mipLevels, std::optional<UINT> firstArraySlice, std::optional<UINT> arraySize)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetSRVDimension(*viewDimension) : GetSRVDimension(textureData.type);

	switch (desc.ViewDimension)
	{
	case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1DArray.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.Texture1DArray.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc1D.MipLevels;
		desc.Texture1DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture1DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc1D.ArraySize;
		break;
	case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DArray.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.Texture2DArray.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc2D.MipLevels;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DMSArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DMSArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.TextureCubeArray.MostDetailedMip = mostDetailedMip ? *mostDetailedMip : 0;
		desc.TextureCubeArray.MipLevels = mipLevels ? *mipLevels : resourceDesc.desc2D.MipLevels;
		desc.TextureCubeArray.First2DArrayFace = firstArraySlice ? *firstArraySlice : 0;
		desc.TextureCubeArray.NumCubes = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::SRV;

	if (FAILED(device->CreateShaderResourceView(textureData.texture.texture2D, &desc, &toStore.view.srv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateUAV(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format, std::optional<TextureType> viewDimension, std::optional<UINT> mipSlice,
	std::optional<UINT> firstWSlice, std::optional<UINT> wSize)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetUAVDimension(*viewDimension) : GetUAVDimension(textureData.type);

	switch (desc.ViewDimension)
	{
	case D3D11_UAV_DIMENSION_TEXTURE1D:
		desc.Format = format ? * format : resourceDesc.desc1D.Format;
		desc.Texture1D.MipSlice = mipSlice ? *mipSlice : 0;
		break;
	case D3D11_UAV_DIMENSION_TEXTURE2D:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2D.MipSlice = mipSlice ? *mipSlice : 0;
		break;
	case D3D11_UAV_DIMENSION_TEXTURE3D:
		desc.Format = format ? *format : resourceDesc.desc3D.Format;
		desc.Texture3D.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture3D.FirstWSlice = firstWSlice ? *firstWSlice : 0;
		desc.Texture3D.WSize = wSize ? *wSize : ((resourceDesc.desc3D.Depth / (desc.Texture3D.MipSlice + 1)) - desc.Texture3D.FirstWSlice);
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::UAV;

	if (FAILED(device->CreateUnorderedAccessView(textureData.texture.texture2D, &desc, &toStore.view.uav)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateUAVTextureArray(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format, std::optional<TextureType> viewDimension, std::optional<UINT> mipSlice,
	std::optional<UINT> firstArraySlice, std::optional<UINT> arraySize)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetUAVDimension(*viewDimension) : GetUAVDimension(textureData.type);

	switch (desc.ViewDimension)
	{
	case D3D11_UAV_DIMENSION_TEXTURE1DARRAY:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1DArray.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture1DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture1DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc1D.ArraySize;
		break;
	case D3D11_UAV_DIMENSION_TEXTURE2DARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DArray.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::UAV;

	if (FAILED(device->CreateUnorderedAccessView(textureData.texture.texture2D, &desc, &toStore.view.uav)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateRTV(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format, std::optional<TextureType> viewDimension,
	std::optional<UINT> mipSlice, std::optional<UINT> firstWSlice, std::optional<UINT> wSize)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetRTVDimension(*viewDimension) : GetRTVDimension(textureData.type);

	switch (desc.ViewDimension)
	{
	case D3D11_RTV_DIMENSION_TEXTURE1D:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1D.MipSlice = mipSlice ? *mipSlice : 0;
		break;
	case D3D11_RTV_DIMENSION_TEXTURE2D:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2D.MipSlice = mipSlice ? *mipSlice : 0;
		break;
	case D3D11_RTV_DIMENSION_TEXTURE3D:
		desc.Format = format ? *format : resourceDesc.desc3D.Format;
		desc.Texture3D.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture3D.FirstWSlice = firstWSlice ? *firstWSlice : 0;
		desc.Texture3D.WSize = wSize ? *wSize : ((resourceDesc.desc3D.Depth / (desc.Texture3D.MipSlice + 1)) - desc.Texture3D.FirstWSlice);
		break;
	case D3D11_RTV_DIMENSION_TEXTURE2DMS:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::RTV;

	if (FAILED(device->CreateRenderTargetView(textureData.texture.texture2D, &desc, &toStore.view.rtv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateRTVTextureArray(const SGGuid& guid, const SGGuid& textureGuid,
	std::optional<DXGI_FORMAT> format,
	std::optional<TextureType> viewDimension,
	std::optional<UINT> mipSlice,
	std::optional<UINT> firstArraySlice,
	std::optional<UINT> arraySize)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_RENDER_TARGET_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetRTVDimension(*viewDimension) : GetRTVDimension(textureData.type);

	switch (desc.ViewDimension)
	{
	case D3D11_RTV_DIMENSION_TEXTURE1DARRAY:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1DArray.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture1DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture1DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	case D3D11_RTV_DIMENSION_TEXTURE2DARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DArray.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	case D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DMSArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DMSArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::RTV;

	if (FAILED(device->CreateRenderTargetView(textureData.texture.texture2D, &desc, &toStore.view.rtv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateDSV(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format,
	std::optional<TextureType> viewDimension,
	std::optional<bool> readOnlyDepth,
	std::optional<bool> readOnlyStencil,
	std::optional<UINT> mipSlice)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetDSVDimension(*viewDimension) : GetDSVDimension(textureData.type);
	desc.Flags = 0;
	desc.Flags |= (readOnlyDepth && *readOnlyDepth ? D3D11_DSV_READ_ONLY_DEPTH : 0);
	desc.Flags |= (readOnlyStencil && *readOnlyStencil ? D3D11_DSV_READ_ONLY_STENCIL : 0);

	switch (desc.ViewDimension)
	{
	case D3D11_DSV_DIMENSION_TEXTURE1D:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1D.MipSlice = mipSlice ? *mipSlice : 0;
		break;
	case D3D11_DSV_DIMENSION_TEXTURE2D:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2D.MipSlice = mipSlice ? *mipSlice : 0;
		break;
	case D3D11_DSV_DIMENSION_TEXTURE2DMS:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		break;
	default:
		return SG::SGResult::FAIL;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::DSV;

	if (FAILED(device->CreateDepthStencilView(textureData.texture.texture2D, &desc, &toStore.view.dsv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11TextureHandler::CreateDSVTextureArray(const SGGuid & guid, const SGGuid & textureGuid,
	std::optional<DXGI_FORMAT> format,
	std::optional<TextureType> viewDimension,
	std::optional<bool> readOnlyDepth,
	std::optional<bool> readOnlyStencil,
	std::optional<UINT> mipSlice,
	std::optional<UINT> firstArraySlice,
	std::optional<UINT> arraySize)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!textures.Exists(textureGuid))
			return SG::SGResult::GUID_MISSING;
	}

	const D3D11TextureData& textureData = textures.GetElement(textureGuid);
	TextureDesc resourceDesc = GetDesc(textureData);

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	desc.ViewDimension = viewDimension ? GetDSVDimension(*viewDimension) : GetDSVDimension(textureData.type);
	desc.Flags = 0;
	desc.Flags |= (readOnlyDepth && *readOnlyDepth ? D3D11_DSV_READ_ONLY_DEPTH : 0);
	desc.Flags |= (readOnlyStencil && *readOnlyStencil ? D3D11_DSV_READ_ONLY_STENCIL : 0);

	switch (desc.ViewDimension)
	{
	case D3D11_DSV_DIMENSION_TEXTURE1DARRAY:
		desc.Format = format ? *format : resourceDesc.desc1D.Format;
		desc.Texture1DArray.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture1DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture1DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc1D.ArraySize;
		break;
	case D3D11_DSV_DIMENSION_TEXTURE2DARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DArray.MipSlice = mipSlice ? *mipSlice : 0;
		desc.Texture2DArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	case D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY:
		desc.Format = format ? *format : resourceDesc.desc2D.Format;
		desc.Texture2DMSArray.FirstArraySlice = firstArraySlice ? *firstArraySlice : 0;
		desc.Texture2DMSArray.ArraySize = arraySize ? *arraySize : resourceDesc.desc2D.ArraySize;
		break;
	default:
		return SG::SGResult::FAIL;
		break;
	}

	D3D11ResourceViewData toStore;
	toStore.type = ResourceViewType::DSV;

	if (FAILED(device->CreateDepthStencilView(textureData.texture.texture2D, &desc, &toStore.view.dsv)))
		return SGResult::FAIL;

	toStore.resourceGuid = textureGuid;
	views.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11TextureHandler::RemoveView(const SGGuid& guid)
{
	views.RemoveElement(guid);
}

SG::SGResult SG::D3D11TextureHandler::BindViewToEntity(const SGGraphicalEntityID & entity, const SGGuid & viewGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, viewGuid, bindGuid, views);
}

SG::SGResult SG::D3D11TextureHandler::BindViewToGroup(const SGGuid & group, const SGGuid & viewGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, viewGuid, bindGuid, views);
}

void SG::D3D11TextureHandler::AddTexture2D(const SGGuid & guid, ID3D11Texture2D * texture)
{
	D3D11TextureData toStore;
	toStore.type = TextureType::TEXTURE_2D;
	toStore.texture.texture2D = texture;
	textures.AddElement(guid, std::move(toStore));
}

void SG::D3D11TextureHandler::FinishFrame()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::FinishFrame();

	textures.FinishFrame();
	views.FinishFrame();
	
	for (auto& guid : updatedFrameBuffer)
		textures[guid].updatedData.SwitchUpdateBuffer();

	updatedTotalBuffer.insert(updatedTotalBuffer.end(), updatedFrameBuffer.begin(), updatedFrameBuffer.end());
	updatedFrameBuffer.clear();
}

void SG::D3D11TextureHandler::SwapFrame()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapFrame();

	textures.UpdateActive();
	views.UpdateActive();
	
	for (auto& guid : updatedTotalBuffer)
		textures[guid].updatedData.SwitchActiveBuffer();

	updatedTotalBuffer.clear();
}

ID3D11ShaderResourceView * SG::D3D11TextureHandler::GetSRV(const SGGuid & guid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGlobalResourceView(guid, ResourceViewType::SRV, views, "texture");
	return static_cast<ID3D11ShaderResourceView*>(toReturn);
}

ID3D11ShaderResourceView * SG::D3D11TextureHandler::GetSRV(const SGGuid & guid, const SGGuid & groupGuid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGroupResourceView(guid, groupGuid, ResourceViewType::SRV, views, "texture");
	return static_cast<ID3D11ShaderResourceView*>(toReturn);
}

ID3D11ShaderResourceView * SG::D3D11TextureHandler::GetSRV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetEntityResourceView(guid, entity, ResourceViewType::SRV, views, "texture");
	return static_cast<ID3D11ShaderResourceView*>(toReturn);
}

ID3D11UnorderedAccessView * SG::D3D11TextureHandler::GetUAV(const SGGuid & guid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGlobalResourceView(guid, ResourceViewType::UAV, views, "texture");
	return static_cast<ID3D11UnorderedAccessView*>(toReturn);
}

ID3D11UnorderedAccessView * SG::D3D11TextureHandler::GetUAV(const SGGuid & guid, const SGGuid & groupGuid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGroupResourceView(guid, groupGuid, ResourceViewType::UAV, views, "texture");
	return static_cast<ID3D11UnorderedAccessView*>(toReturn);
}

ID3D11UnorderedAccessView * SG::D3D11TextureHandler::GetUAV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetEntityResourceView(guid, entity, ResourceViewType::UAV, views, "texture");
	return static_cast<ID3D11UnorderedAccessView*>(toReturn);
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGlobalResourceView(guid, ResourceViewType::RTV, views, "texture");
	return static_cast<ID3D11RenderTargetView*>(toReturn);
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid, const SGGuid & groupGuid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGroupResourceView(guid, groupGuid, ResourceViewType::RTV, views, "texture");
	return static_cast<ID3D11RenderTargetView*>(toReturn);
}

ID3D11RenderTargetView * SG::D3D11TextureHandler::GetRTV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetEntityResourceView(guid, entity, ResourceViewType::RTV, views, "texture");
	return static_cast<ID3D11RenderTargetView*>(toReturn);
}

ID3D11DepthStencilView * SG::D3D11TextureHandler::GetDSV(const SGGuid & guid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGlobalResourceView(guid, ResourceViewType::DSV, views, "texture");
	return static_cast<ID3D11DepthStencilView*>(toReturn);
}

ID3D11DepthStencilView * SG::D3D11TextureHandler::GetDSV(const SGGuid & guid, const SGGuid & groupGuid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGroupResourceView(guid, groupGuid, ResourceViewType::DSV, views, "texture");
	return static_cast<ID3D11DepthStencilView*>(toReturn);
}

ID3D11DepthStencilView * SG::D3D11TextureHandler::GetDSV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetEntityResourceView(guid, entity, ResourceViewType::DSV, views, "texture");
	return static_cast<ID3D11DepthStencilView*>(toReturn);
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
	case SG::TextureType::TEXTURE_1D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE1D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::TextureType::TEXTURE_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_MULTISAMPLED_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		break;
	case SG::TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
		break;
	case SG::TextureType::TEXTURE_3D:
		toReturn = D3D11_SRV_DIMENSION_TEXTURE3D;
		break;
	case ::SG::TextureType::TEXTURE_CUBE:
		toReturn = D3D11_SRV_DIMENSION_TEXTURECUBE;
		break;
	case ::SG::TextureType::TEXTURE_ARRAY_CUBE:
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
	case SG::TextureType::TEXTURE_1D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE1D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::TextureType::TEXTURE_2D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE2D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_CUBE:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_ARRAY_CUBE:
		toReturn = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_3D:
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
	case SG::TextureType::TEXTURE_1D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE1D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::TextureType::TEXTURE_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_CUBE:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_ARRAY_CUBE:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_MULTISAMPLED_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DMS;
		break;
	case SG::TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		toReturn = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		break;
	case SG::TextureType::TEXTURE_3D:
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
	case SG::TextureType::TEXTURE_1D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE1D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_1D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
		break;
	case SG::TextureType::TEXTURE_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2D;
		break;
	case SG::TextureType::TEXTURE_ARRAY_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_CUBE:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_ARRAY_CUBE:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		break;
	case SG::TextureType::TEXTURE_MULTISAMPLED_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		break;
	case SG::TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		toReturn = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		break;
	default:
		throw std::runtime_error("Trying to create a DSV with a texture of incompatible type");
		break;
	}

	return toReturn;
}

SG::D3D11TextureHandler::TextureDesc SG::D3D11TextureHandler::GetDesc(const D3D11TextureData& storedData)
{
	TextureDesc toReturn;

	switch (storedData.type)
	{
	case TextureType::TEXTURE_1D:
		storedData.texture.texture1D->GetDesc(&toReturn.desc1D);
		break;
	case TextureType::TEXTURE_ARRAY_1D:
		storedData.texture.texture1D->GetDesc(&toReturn.desc1D);
		break;
	case TextureType::TEXTURE_2D:
		storedData.texture.texture2D->GetDesc(&toReturn.desc2D);
		break;
	case TextureType::TEXTURE_ARRAY_2D:
		storedData.texture.texture2D->GetDesc(&toReturn.desc2D);
		break;
	case TextureType::TEXTURE_MULTISAMPLED_2D:
		storedData.texture.texture2D->GetDesc(&toReturn.desc2D);
		break;
	case TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
		storedData.texture.texture2D->GetDesc(&toReturn.desc2D);
		break;
	case TextureType::TEXTURE_CUBE:
		storedData.texture.texture2D->GetDesc(&toReturn.desc2D);
		break;
	case TextureType::TEXTURE_ARRAY_CUBE:
		storedData.texture.texture2D->GetDesc(&toReturn.desc2D);
		break;
	case TextureType::TEXTURE_3D:
		storedData.texture.texture3D->GetDesc(&toReturn.desc3D);
		break;
	default:
		throw std::runtime_error("Trying to get description of a texture of incompatible type");
	}

	return toReturn;
}

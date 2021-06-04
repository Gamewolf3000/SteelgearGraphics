#include "D3D11TextureData.h"

SG::D3D11TextureData::~D3D11TextureData()
{
    if (texture.texture1D != nullptr) // Since only ptrs it does not matter which is used here
    {
		switch (type)
		{
		case TextureType::TEXTURE_1D:
			texture.texture1D->Release();
			break;
		case TextureType::TEXTURE_ARRAY_1D:
			texture.texture1D->Release();
			break;
		case TextureType::TEXTURE_2D:
			texture.texture2D->Release();
			break;
		case TextureType::TEXTURE_ARRAY_2D:
			texture.texture2D->Release();
			break;
		case TextureType::TEXTURE_MULTISAMPLED_2D:
			texture.texture2D->Release();
			break;
		case TextureType::TEXTURE_ARRAY_MULTISAMPLED_2D:
			texture.texture2D->Release();
			break;
		case TextureType::TEXTURE_CUBE:
			texture.texture2D->Release();
			break;
		case TextureType::TEXTURE_ARRAY_CUBE:
			texture.texture2D->Release();
			break;
		case TextureType::TEXTURE_3D:
			texture.texture3D->Release();
			break;
		default:
			break;
		}
    }
}

SG::D3D11TextureData::D3D11TextureData(D3D11TextureData&& other) : type(other.type), texture(other.texture), 
																	updatedData(std::move(other.updatedData))
{
	other.texture.texture1D = nullptr; // Since only ptrs it does not matter which is used here
}

SG::D3D11TextureData& SG::D3D11TextureData::operator=(D3D11TextureData&& other)
{
	if (this != &other)
	{
		type = other.type;
		texture = other.texture;
		other.texture.texture1D = nullptr;
		updatedData = std::move(other.updatedData);
	}

	return *this;
}

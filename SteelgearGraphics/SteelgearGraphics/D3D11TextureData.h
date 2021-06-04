#pragma once

#include <d3d11_4.h>

#include "D3D11CommonTypes.h"
#include "TripleBufferedData.h"

namespace SG
{
	enum class TextureType
	{
		TEXTURE_1D,
		TEXTURE_ARRAY_1D,
		TEXTURE_2D,
		TEXTURE_ARRAY_2D,
		TEXTURE_MULTISAMPLED_2D,
		TEXTURE_ARRAY_MULTISAMPLED_2D,
		TEXTURE_3D,
		TEXTURE_CUBE,
		TEXTURE_ARRAY_CUBE
	};

	struct D3D11TextureData
	{
		TextureType type;
		union
		{
			ID3D11Texture1D* texture1D;
			ID3D11Texture2D* texture2D;
			ID3D11Texture3D* texture3D;
		} texture;
		TripleBufferedData<UpdateData> updatedData;

		D3D11TextureData() = default;
		~D3D11TextureData();

		D3D11TextureData(D3D11TextureData&& other);
		D3D11TextureData& operator=(D3D11TextureData&& other);
	};
}
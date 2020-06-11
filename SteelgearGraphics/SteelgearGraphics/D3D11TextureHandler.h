#pragma once

#include <d3d11_4.h>
#include <vector>
#include <unordered_map>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	/*
	enum class TextureUsage
	{
		SHADER_RESOURCE,
		RENDER_TARGET,
		DEPTH_STENCIL,
		UNORDERED_ACCESS
	};
	*/

	struct SGTextureData
	{
		UINT mipLevels;
		DXGI_FORMAT format;
		bool gpuWritable;
		bool cpuWritable;
		bool cpuReadable;
		//TextureUsage usage;
		bool generateMips;
		bool resourceClamp;
		void* data;
		UINT bytesPerRow;
		UINT rowsPerSlice;
	};

	class D3D11TextureHandler
	{
	public:


		D3D11TextureHandler(ID3D11Device* device);
		~D3D11TextureHandler();

		SGResult CreateTexture1D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT arraySize);
		SGResult CreateTexture2D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC& sampleDesc, bool texturecube);
		SGResult CreateTexture3D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT depth);

		SGResult CreateSRV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mostDetailedMip, UINT mipLevels);
		SGResult CreateSRVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mostDetailedMip, UINT mipLevels, UINT firstArraySlice, UINT arraySize);

		SGResult CreateUAV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipLevels, UINT firstWSlice = 0, UINT wSize = 0);
		SGResult CreateUAVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice = 0, UINT arraySize = 0);

		SGResult CreateRTV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipLevels, UINT firstWSlice = 0, UINT wSize = 0);
		SGResult CreateRTVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize);

		SGResult CreateRTV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, bool readOnlyDepth, bool readOnlyStencil, UINT mipSlice);
		SGResult CreateRTVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, bool readOnlyDepth, bool readOnlyStencil, UINT mipSlice, UINT firstArraySlice, UINT arraySize);



	private:
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
			bool updated = false;
			union
			{
				ID3D11Texture1D* texture1D;
				ID3D11Texture2D* texture2D;
				ID3D11Texture3D* texture3D;
			} texture;
			void* UpdatedData[3] = { nullptr, nullptr, nullptr };
		};


		std::unordered_map<SGGuid, D3D11TextureData> textures;
		std::unordered_map<SGGuid, D3D11ResourceViewData> views;

		std::vector<std::unordered_map<SGGuid, SGGuid>> entityData; // the entity leads to a position in the vector, and the guid is used to retrieve the guid of the actual texture/view

		ID3D11Device* device;

	};
}
#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	
	enum class TextureBinding
	{
		SHADER_RESOURCE,
		RENDER_TARGET,
		DEPTH_STENCIL,
		UNORDERED_ACCESS
	};
	

	struct SGTextureData
	{
		UINT mipLevels;
		DXGI_FORMAT format;
		bool gpuWritable;
		bool cpuWritable;
		bool cpuReadable;
		std::vector<TextureBinding> textureBindings;
		bool generateMips;
		bool resourceClamp;
		void* data;
		UINT bytesPerRow;
		UINT rowsPerSlice;
	};

	class D3D11TextureHandler : public SGGraphicsHandler
	{
	public:


		D3D11TextureHandler(ID3D11Device* device);
		~D3D11TextureHandler();

		SGResult CreateTexture1D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT arraySize);
		SGResult CreateTexture2D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC& sampleDesc, bool texturecube);
		SGResult CreateTexture3D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT depth);

		SGResult CreateSRV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mostDetailedMip, UINT mipLevels);
		SGResult CreateSRVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mostDetailedMip, UINT mipLevels, UINT firstArraySlice, UINT arraySize);

		SGResult CreateUAV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice = 0, UINT wSize = 0);
		SGResult CreateUAVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice = 0, UINT arraySize = 0);

		SGResult CreateRTV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstWSlice = 0, UINT wSize = 0);
		SGResult CreateRTVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, UINT mipSlice, UINT firstArraySlice, UINT arraySize);

		SGResult CreateDSV(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, bool readOnlyDepth, bool readOnlyStencil, UINT mipSlice);
		SGResult CreateDSVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, DXGI_FORMAT format, bool readOnlyDepth, bool readOnlyStencil, UINT mipSlice, UINT firstArraySlice, UINT arraySize);



		void AddTexture2D(const SGGuid& guid, ID3D11Texture2D* texture);

		void SwapUpdateBuffer();
		void SwapToWorkWithBuffer();


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
			std::pair<ResourceData, UpdateStrategy> UpdatedData[3];
		};


		LockableUnorderedMap<SGGuid, D3D11TextureData> textures;
		LockableUnorderedMap<SGGuid, D3D11ResourceViewData> views;

		int toWorkWith = 0;
		int toUseNext = 1;
		int toUpdate = 2;

		ID3D11Device* device;

		void SetUsageAndCPUAccessFlags(const SGTextureData & generalSettings, D3D11_USAGE& usage, UINT& cpuAccessFlags);
		void SetBindflags(const SGTextureData & generalSettings, UINT& flags);
		D3D11_RTV_DIMENSION GetRTVDimension(TextureType type);
		D3D11_DSV_DIMENSION GetDSVDimension(TextureType type);
	};
}
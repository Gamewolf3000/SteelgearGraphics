#pragma once

#include <d3d11_4.h>
#include <vector>
#include <unordered_map>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"


namespace SG
{
	enum class TextureType
	{
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D

	};

	enum class TextureUsage
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
		TextureUsage usage;
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

		SGResult CreateTexture1D(const SGGraphicalEntityID& entity, const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT arraySize);
		SGResult CreateGlobalTexture1D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT arraySize);

		SGResult CreateTexture2D(const SGGraphicalEntityID& entity, const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC& sampleDesc, bool texturecube);
		SGResult CreateGlobalTexture2D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC& sampleDesc, bool texturecube);

		SGResult CreateTexture3D(const SGGraphicalEntityID& entity, const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT depth);
		SGResult CreateGlobalTexture3D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT depth);




	private:

		struct D3D11TextureData
		{
			TextureType type;
			bool gpuWritable = false;
			bool updated = false;
			void* texture = nullptr;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11UnorderedAccessView* uav = nullptr;
			void* UpdatedData[3] = { nullptr, nullptr, nullptr };
		};

		std::vector<std::unordered_map<SGGuid, D3D11TextureData>> entityTextureData;
		std::unordered_map<SGGuid, D3D11TextureData> globalTextureData;

		ID3D11Device* device;

	};
}
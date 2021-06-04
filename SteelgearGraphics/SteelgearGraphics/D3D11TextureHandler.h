#pragma once

#include <optional>

#include <d3d11_4.h>

#include "D3D11GraphicsHandler.h"

#include "D3D11CommonTypes.h"
#include "D3D11TextureData.h"

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
		std::vector<void*> data;
		//UINT bytesPerRow;
		//UINT rowsPerSlice;
	};

	class D3D11TextureHandler : public D3D11GraphicsHandler
	{
	public:


		D3D11TextureHandler(ID3D11Device* device);
		~D3D11TextureHandler() = default;

		SGResult CreateTexture1D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT arraySize);
		SGResult CreateTexture2D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT arraySize, const DXGI_SAMPLE_DESC& sampleDesc, bool texturecube);
		SGResult CreateTexture3D(const SGGuid& guid, const SGTextureData& generalSettings, UINT width, UINT height, UINT depth);
		void RemoveTexture(const SGGuid& guid);

		SGResult CreateSRV(const SGGuid& guid, const SGGuid& textureGuid,
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<UINT> mostDetailedMip = std::nullopt,
			std::optional<UINT> mipLevels = std::nullopt);
		SGResult CreateSRVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, 
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<UINT> mostDetailedMip = std::nullopt,
			std::optional<UINT> mipLevels = std::nullopt,
			std::optional<UINT> firstArraySlice = std::nullopt,
			std::optional<UINT> arraySize = std::nullopt);

		SGResult CreateUAV(const SGGuid& guid, const SGGuid& textureGuid,
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<UINT> mipSlice = std::nullopt,
			std::optional<UINT> firstWSlice = std::nullopt,
			std::optional<UINT> wSize = std::nullopt);
		SGResult CreateUAVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, 
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<UINT> mipSlice = std::nullopt,
			std::optional<UINT> firstArraySlice = std::nullopt,
			std::optional<UINT> arraySize = std::nullopt);

		SGResult CreateRTV(const SGGuid& guid, const SGGuid& textureGuid, 
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<UINT> mipSlice = std::nullopt,
			std::optional<UINT> firstWSlice = std::nullopt,
			std::optional<UINT> wSize = std::nullopt);
		SGResult CreateRTVTextureArray(const SGGuid& guid, const SGGuid& textureGuid,
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<UINT> mipSlice = std::nullopt,
			std::optional<UINT> firstArraySlice = std::nullopt,
			std::optional<UINT> arraySize = std::nullopt);

		SGResult CreateDSV(const SGGuid& guid, const SGGuid& textureGuid, 
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<bool> readOnlyDepth = std::nullopt,
			std::optional<bool> readOnlyStencil = std::nullopt,
			std::optional<UINT> mipSlice = std::nullopt);
		SGResult CreateDSVTextureArray(const SGGuid& guid, const SGGuid& textureGuid, 
			std::optional<DXGI_FORMAT> format = std::nullopt,
			std::optional<TextureType> viewDimension = std::nullopt,
			std::optional<bool> readOnlyDepth = std::nullopt,
			std::optional<bool> readOnlyStencil = std::nullopt,
			std::optional<UINT> mipSlice = std::nullopt,
			std::optional<UINT> firstArraySlice = std::nullopt,
			std::optional<UINT> arraySize = std::nullopt);

		void RemoveView(const SGGuid& guid);

		SGResult BindViewToEntity(const SGGraphicalEntityID& entity, const SGGuid& viewGuid, const SGGuid& bindGuid);
		SGResult BindViewToGroup(const SGGuid& group, const SGGuid& viewGuid, const SGGuid& bindGuid);

	private:

		friend class D3D11RenderEngine;

		union TextureDesc
		{
			D3D11_TEXTURE1D_DESC desc1D;
			D3D11_TEXTURE2D_DESC desc2D;
			D3D11_TEXTURE3D_DESC desc3D;

			TextureDesc& operator=(const TextureDesc& other)
			{
				memcpy(this, &other, sizeof(TextureDesc));
				return *this;
			}
		};

		FrameMap<SGGuid, D3D11TextureData> textures;
		FrameMap<SGGuid, D3D11ResourceViewData> views;

		std::vector<SGGuid> updatedFrameBuffer;
		std::vector<SGGuid> updatedTotalBuffer;

		ID3D11Device* device;

		void SetUsageAndCPUAccessFlags(const SGTextureData & generalSettings, D3D11_USAGE& usage, UINT& cpuAccessFlags);
		void SetBindflags(const SGTextureData & generalSettings, UINT& flags);
		D3D11_SRV_DIMENSION GetSRVDimension(TextureType type);
		D3D11_UAV_DIMENSION GetUAVDimension(TextureType type);
		D3D11_RTV_DIMENSION GetRTVDimension(TextureType type);
		D3D11_DSV_DIMENSION GetDSVDimension(TextureType type);

		TextureDesc GetDesc(const D3D11TextureData& storedData);

		void AddTexture2D(const SGGuid& guid, ID3D11Texture2D* texture);

		void FinishFrame() override;
		void SwapFrame() override;

		ID3D11ShaderResourceView* GetSRV(const SGGuid& guid);
		ID3D11ShaderResourceView* GetSRV(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11ShaderResourceView* GetSRV(const SGGuid& guid, const SGGraphicalEntityID& entity);

		ID3D11UnorderedAccessView* GetUAV(const SGGuid& guid);
		ID3D11UnorderedAccessView* GetUAV(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11UnorderedAccessView* GetUAV(const SGGuid& guid, const SGGraphicalEntityID& entity);

		ID3D11RenderTargetView* GetRTV(const SGGuid& guid);
		ID3D11RenderTargetView* GetRTV(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11RenderTargetView* GetRTV(const SGGuid& guid, const SGGraphicalEntityID& entity);

		ID3D11DepthStencilView* GetDSV(const SGGuid& guid);
		ID3D11DepthStencilView* GetDSV(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11DepthStencilView* GetDSV(const SGGuid& guid, const SGGraphicalEntityID& entity);
	};
}
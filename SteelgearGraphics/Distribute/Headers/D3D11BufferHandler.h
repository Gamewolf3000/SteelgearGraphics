#pragma once

#include <d3d11_4.h>

#include "D3D11GraphicsHandler.h"
#include "D3D11BufferData.h"

namespace SG
{
	class D3D11BufferHandler : public D3D11GraphicsHandler
	{
	public:

		D3D11BufferHandler(ID3D11Device* device);
		~D3D11BufferHandler() = default;

		SGResult CreateVertexBuffer(const SGGuid& guid, UINT size, UINT nrOfVertices, bool dynamic, bool streamOut, const void* const data);
		SGResult CreateIndexBuffer(const SGGuid& guid, UINT size, UINT nrOfIndices, bool dynamic, const void* const data);
		SGResult CreateConstantBuffer(const SGGuid& guid, UINT size, bool dynamic, bool cpuUpdate, const void * const data);
		SGResult CreateStructuredBuffer(const SGGuid& guid, UINT count, UINT structSize, bool cpuWritable, bool gpuWritable, void* data);
		SGResult CreateAppendConsumeBuffer(const SGGuid& guid, UINT size, UINT structsize, void* data);
		SGResult CreateByteAdressBuffer(const SGGuid& guid, UINT size, bool gpuWritable, void* data);
		SGResult CreateIndirectArgsBuffer(const SGGuid& guid, UINT size, void* data);
		void RemoveBuffer(const SGGuid& guid);

		SGResult CreateBufferOffset(const SGGuid& guid, UINT value);
		void RemoveBufferOffset(const SGGuid& guid);

		SGResult CreateBufferStride(const SGGuid& guid, UINT value);
		void RemoveBufferStride(const SGGuid& guid);

		/**
			bufferGuid is the BufferData to create a SRV for
		*/
		SGResult CreateSRV(const SGGuid& guid, const SGGuid& bufferGuid, DXGI_FORMAT format, UINT elementOffset, UINT elementWidth);

		/**
			bufferGuid is the BufferData to create a UAV for
		*/
		SGResult CreateUAV(const SGGuid& guid, const SGGuid& bufferGuid, DXGI_FORMAT format, UINT firstElement, UINT numberOfElements, bool counter, bool append, bool raw);

		void RemoveView(const SGGuid& guid);

		SGResult BindBufferToEntity(const SGGraphicalEntityID& entity, const SGGuid& bufferGuid, const SGGuid& bindGuid);
		SGResult BindBufferToGroup(const SGGuid& group, const SGGuid& bufferGuid, const SGGuid& bindGuid);

		SGResult BindOffsetToEntity(const SGGraphicalEntityID& entity, const SGGuid& offsetGuid, const SGGuid& bindGuid);
		SGResult BindOffsetToGroup(const SGGuid& group, const SGGuid& offsetGuid, const SGGuid& bindGuid);
		SGResult BindStrideToEntity(const SGGraphicalEntityID& entity, const SGGuid& strideGuid, const SGGuid& bindGuid);
		SGResult BindStrideToGroup(const SGGuid& group, const SGGuid& strideGuid, const SGGuid& bindGuid);

		SGResult BindViewToEntity(const SGGraphicalEntityID& entity, const SGGuid& viewGuid, const SGGuid& bindGuid);
		SGResult BindViewToGroup(const SGGuid& group, const SGGuid& viewGuid, const SGGuid& bindGuid);

		void UpdateBuffer(const SGGuid& guid, const UpdateStrategy& updateStrategy, void* data, UINT subresource = 0);

	private:

		friend class D3D11RenderEngine;

		FrameMap<SGGuid, D3D11BufferData> buffers;
		FrameMap<SGGuid, D3D11ResourceViewData> views;
		FrameMap<SGGuid, UINT> bufferOffsets;
		FrameMap<SGGuid, UINT> bufferStrides;

		std::mutex frameBufferMutex;
		std::vector<SGGuid> updatedFrameBuffer;
		std::vector<SGGuid> updatedTotalBuffer;

		ID3D11Device* device;

		void FinishFrame() override;
		void SwapFrame() override;

		void UpdateBufferGPU(D3D11BufferData& toUpdate, ID3D11DeviceContext* context);

		ID3D11Buffer* GetBuffer(const SGGuid& guid, ID3D11DeviceContext* context);
		ID3D11Buffer* GetBuffer(const SGGuid& guid, ID3D11DeviceContext* context, const SGGuid& groupGuid);
		ID3D11Buffer* GetBuffer(const SGGuid& guid, ID3D11DeviceContext* context, const SGGraphicalEntityID& entity);

		ID3D11ShaderResourceView* GetSRV(const SGGuid& guid);
		ID3D11ShaderResourceView* GetSRV(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11ShaderResourceView* GetSRV(const SGGuid& guid, const SGGraphicalEntityID& entity);

		ID3D11UnorderedAccessView* GetUAV(const SGGuid& guid);
		ID3D11UnorderedAccessView* GetUAV(const SGGuid& guid, const SGGuid& groupGuid);
		ID3D11UnorderedAccessView* GetUAV(const SGGuid& guid, const SGGraphicalEntityID& entity);

		UINT GetOffset(const SGGuid& guid);
		UINT GetOffset(const SGGuid& guid, const SGGuid& groupGuid);
		UINT GetOffset(const SGGuid& guid, const SGGraphicalEntityID& entity);

		UINT GetStride(const SGGuid& guid);
		UINT GetStride(const SGGuid& guid, const SGGuid& groupGuid);
		UINT GetStride(const SGGuid& guid, const SGGraphicalEntityID& entity);

		UINT GetElementCount(const SGGuid& guid);
		UINT GetElementCount(const SGGuid& guid, const SGGuid& groupGuid);
		UINT GetElementCount(const SGGuid& guid, const SGGraphicalEntityID& entity);

		UINT GetVBElementSize(const SGGuid& guid);
		UINT GetVBElementSize(const SGGuid& guid, const SGGuid& groupGuid);
		UINT GetVBElementSize(const SGGuid& guid, const SGGraphicalEntityID& entity);
	};
}
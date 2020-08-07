#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"
#include "TripleBufferedData.h"


namespace SG
{
	class D3D11BufferHandler : public SGGraphicsHandler
	{
	public:

		D3D11BufferHandler(ID3D11Device* device);
		~D3D11BufferHandler();

		SGResult CreateVertexBuffer(const SGGuid& guid, UINT size, UINT nrOfVertices, bool dynamic, bool streamOut, void* data);
		SGResult CreateIndexBuffer(const SGGuid& guid, UINT size, UINT nrOfIndices, bool dynamic, void* data);
		SGResult CreateConstantBuffer(const SGGuid& guid, UINT size, bool dynamic, bool cpuUpdate, void* data);
		SGResult CreateStructuredBuffer(const SGGuid& guid, UINT count, UINT structSize, bool cpuWritable, bool gpuWritable, void* data);
		SGResult CreateAppendConsumeBuffer(const SGGuid& guid, UINT size, UINT structsize, void* data);
		SGResult CreateByteAdressBuffer(const SGGuid& guid, UINT size, bool gpuWritable, void* data);
		SGResult CreateIndirectArgsBuffer(const SGGuid& guid, UINT size, void* data);

		SGResult CreateBufferOffset(const SGGuid& guid, UINT value);
		SGResult CreateBufferStride(const SGGuid& guid, UINT value);


		/**
			bufferGuid is the BufferData to create a SRV for
		*/
		SGResult CreateSRV(const SGGuid& guid, const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);

		/**
			bufferGuid is the BufferData to create a UAV for
		*/
		SGResult CreateUAV(const SGGuid& guid, const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);

		SGResult BindBufferToEntity(const SGGraphicalEntityID& entity, const SGGuid& bufferGuid, const SGGuid& bindGuid);
		SGResult BindBufferToGroup(const SGGuid& group, const SGGuid& bufferGuid, const SGGuid& bindGuid);

		SGResult BindOffsetToEntity(const SGGraphicalEntityID& entity, const SGGuid& offsetGuid, const SGGuid& bindGuid);
		SGResult BindOffsetToGroup(const SGGuid& group, const SGGuid& offsetGuid, const SGGuid& bindGuid);
		SGResult BindStrideToEntity(const SGGraphicalEntityID& entity, const SGGuid& strideGuid, const SGGuid& bindGuid);
		SGResult BindStrideToGroup(const SGGuid& group, const SGGuid& strideGuid, const SGGuid& bindGuid);

		void UpdateBuffer(const SGGuid& guid, const UpdateStrategy& updateStrategy, void* data, UINT subresource = 0);


	private:

		friend class D3D11RenderEngine;

		enum class BufferType
		{
			VERTEX_BUFFER,
			INDEX_BUFFER,
			CONSTANT_BUFFER,
			STRUCTURED_BUFFER,
			APPEND_CONSUME_BUFFER,
			BYTE_ADRESS_BUFFER,
			INDIRECT_ARGS_BUFFER
		};

		struct VertexBufferData
		{
			UINT nrOfVertices;
			UINT vertexSize;
		};

		struct IndexBufferData
		{
			UINT nrOfIndices;
		};

		struct D3D11BufferData
		{
			BufferType type;
			//bool updated = false;
			union
			{
				VertexBufferData vb;
				IndexBufferData ib;
			} specificData;
			ID3D11Buffer* buffer = nullptr;
			TripleBufferedData<UpdateData> updatedData;
		};

		LockableUnorderedMap<SGGuid, D3D11BufferData> buffers;
		LockableUnorderedMap<SGGuid, D3D11ResourceViewData> views;
		LockableUnorderedMap<SGGuid, UINT> bufferOffsets;
		LockableUnorderedMap<SGGuid, UINT> bufferStrides;

		std::mutex frameBufferMutex;
		std::vector<SGGuid> updatedFrameBuffer;
		std::vector<SGGuid> updatedTotalBuffer;

		ID3D11Device* device;

		void SwapUpdateBuffer() override;
		void SwapToWorkWithBuffer() override;

		//void SetConstantBuffers(const std::vector<PipelineComponent>& vs, const std::vector<PipelineComponent>& hs, const std::vector<PipelineComponent>& ds, const std::vector<PipelineComponent>& gs, const std::vector<PipelineComponent>& ps, ID3D11DeviceContext* context, const SGGuid& groupGuid = SGGuid(), const SGGraphicalEntityID& entity = SGGraphicalEntityID());
		//void FillBufferArray(const std::vector<PipelineComponent>& resources, ID3D11Buffer ** bufferArr, unsigned int arrSize, ID3D11DeviceContext* context, const SGGuid& groupGuid, const SGGraphicalEntityID& entity);
		void UpdateBufferGPU(D3D11BufferData& toUpdate, ID3D11DeviceContext* context);

		ID3D11Buffer* GetBuffer(const SGGuid& guid, ID3D11DeviceContext* context);
		ID3D11Buffer* GetBuffer(const SGGuid& guid, ID3D11DeviceContext* context, const SGGuid& groupGuid);
		ID3D11Buffer* GetBuffer(const SGGuid& guid, ID3D11DeviceContext* context, const SGGraphicalEntityID& entity);

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
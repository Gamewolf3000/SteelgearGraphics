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

		std::mutex frameBufferMutex;
		std::vector<SGGuid> updatedFrameBuffer;
		std::vector<SGGuid> updatedTotalBuffer;

		ID3D11Device* device;

		void SwapUpdateBuffer();
		void SwapToWorkWithBuffer();
		void UpdateBuffers(ID3D11DeviceContext* context);

		void SetShaderResources(const std::vector<ShaderResource>& toSet, ID3D11DeviceContext* context);
		void SetShaderResources(const std::vector<ShaderResource>& toSet, ID3D11DeviceContext* context, const SGGuid& groupGuid);
		void SetShaderResources(const std::vector<ShaderResource>& toSet, ID3D11DeviceContext* context, const SGGraphicalEntityID& entity);
	};
}
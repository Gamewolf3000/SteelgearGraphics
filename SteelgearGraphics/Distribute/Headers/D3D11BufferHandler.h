#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"


namespace SG
{
	class D3D11BufferHandler : public SGGraphicsHandler
	{
	public:

		D3D11BufferHandler(ID3D11Device* device);
		~D3D11BufferHandler();

		SGResult CreateVertexBuffer(const SGGuid& guid, UINT size, bool dynamic, bool streamOut, void* data);
		SGResult CreateIndexBuffer(const SGGuid& guid, UINT size, bool dynamic, void* data);
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


		void UpdateBuffer(const SGGuid& guid, UpdateStrategy updateStrategy, void* data, size_t byteSize, UINT subresource = 0);




		void SwapUpdateBuffer();
		void SwapToWorkWithBuffer();


	private:
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
			bool updated = false;
			union
			{
				VertexBufferData vb;
				IndexBufferData ib;
			} specificData;
			ID3D11Buffer* buffer = nullptr;
			std::pair<ResourceData, UpdateStrategy> UpdatedData[3];
		};

		std::unordered_map<SGGuid, D3D11BufferData> buffers;
		std::unordered_map<SGGuid, D3D11ResourceViewData> views;

		int toWorkWith = 0;
		int toUseNext = 1;
		int toUpdate = 2;

		ID3D11Device* device;
	};
}
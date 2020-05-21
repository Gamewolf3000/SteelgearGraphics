#pragma once

#include <d3d11_4.h>
#include <vector>
#include <unordered_map>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"


namespace SG
{
	class D3D11BufferHandler
	{
	public:
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

		D3D11BufferHandler(ID3D11Device* device);
		~D3D11BufferHandler();

		SGResult CreateVertexBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, bool streamOut, void* data);
		SGResult CreateGlobalVertexBuffer(const SGGuid& guid, UINT size, bool dynamic, bool streamOut, void* data);
		SGResult CreateIndexBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, void* data);
		SGResult CreateGlobalIndexBuffer(const SGGuid& guid, UINT size, bool dynamic, void* data);
		SGResult CreateConstantBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, bool cpuUpdate, void* data);
		SGResult CreateGlobalConstantBuffer(const SGGuid& guid, UINT size, bool dynamic, bool cpuUpdate, void* data);
		SGResult CreateStructuredBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT count, UINT structSize, bool cpuWritable, bool gpuWritable, void* data);
		SGResult CreateGlobalStructuredBuffer(const SGGuid& guid, UINT count, UINT structSize, bool cpuWritable, bool gpuWritable, void* data);
		SGResult CreateAppendConsumeBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, UINT structsize, void* data);
		SGResult CreateGlobaAppendConsumeBuffer(const SGGuid& guid, UINT size, UINT structsize, void* data);
		SGResult CreateByteAdressBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool gpuWritable, void* data);
		SGResult CreateGlobalByteAdressBuffer(const SGGuid& guid, UINT size, bool gpuWritable, void* data);
		SGResult CreateGlobalIndirectArgsBuffer(const SGGuid& guid, UINT size, void* data);




		SGResult CreateBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid);

		/**
			entityGuid is the BufferData of the entity where the SRV is to be stored
			bufferGuid is the BufferData to create a SRV for. First look in the entity, then if not found, look among global buffers
		*/
		SGResult CreateSRV(const SGGraphicalEntityID& entity, const SGGuid& entityGuid, const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);
		/**
			Creates a SRV for a global buffer. It is not connected to any entity
		*/
		SGResult CreateGlobalSRV(const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);

		/**
			entityGuid is the BufferData of the entity where the UAV is to be stored
			bufferGuid is the BufferData to create a UAV for. First look in the entity, then if not found, look among global buffers
*/
		SGResult CreateUAV(const SGGraphicalEntityID& entity, const SGGuid& entityGuid, const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);
		/**
			Creates a UAV for a global buffer. It is not connected to any entity
		*/
		SGResult CreateGlobalUAV(const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);




	private:

		struct D3D11BufferData
		{
			BufferType type;
			bool gpuWritable = false;
			bool updated = false;
			ID3D11Buffer* buffer = nullptr;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11UnorderedAccessView* uav = nullptr;
			void* UpdatedData[3] = { nullptr, nullptr, nullptr };
		};

		std::vector<std::unordered_map<SGGuid, D3D11BufferData>> entityBufferData;
		std::unordered_map<SGGuid, D3D11BufferData> globalBufferData;

		ID3D11Device* device;

	};
}
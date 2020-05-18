#pragma once

#include <d3d11_4.h>
#include <vector>
#include <unordered_map>

#include "SGError.h"
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

		SGError CreateVertexBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, bool streamOut, void* data);
		SGError CreateGlobalVertexBuffer(const SGGuid& guid, UINT size, bool dynamic, bool streamOut, void* data);
		SGError CreateIndexBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, void* data);
		SGError CreateGlobalIndexBuffer(const SGGuid& guid, UINT size, bool dynamic, void* data);
		SGError CreateConstantBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, bool cpuUpdate, void* data);
		SGError CreateGlobalConstantBuffer(const SGGuid& guid, UINT size, bool dynamic, bool cpuUpdate, void* data);
		SGError CreateStructuredBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT count, UINT structSize, bool cpuWritable, bool gpuWritable, void* data);
		SGError CreateGlobalStructuredBuffer(const SGGuid& guid, UINT count, UINT structSize, bool cpuWritable, bool gpuWritable, void* data);



		SGError CreateBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid);

		/**
			entityGuid is the BufferData of the entity where the srv is to be stored
			bufferGuid is the BufferData to create a SRV for. First look in the entity, then if not found, look among global buffers
		*/
		SGError CreateSRV(const SGGraphicalEntityID& entity, const SGGuid& entityGuid, const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);
		/**
			Creates a srv for a global buffer. It is not connected to any entity
		*/
		SGError CreateGlobalSRV(const SGGuid& bufferGuid, UINT firstElement, UINT numberOfElements);




	private:

		struct BufferData
		{
			BufferType type;
			bool updated = false;
			ID3D11Buffer* buffer = nullptr;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11UnorderedAccessView* uav = nullptr;
			void* UpdatedData[3] = { nullptr, nullptr, nullptr };
		};

		std::vector<std::unordered_map<SGGuid, BufferData>> entityBufferData;
		std::unordered_map<SGGuid, BufferData> globalBufferData;

		ID3D11Device* device;

	};
}
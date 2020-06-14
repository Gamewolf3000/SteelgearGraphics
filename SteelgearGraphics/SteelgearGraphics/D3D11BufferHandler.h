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
	class D3D11BufferHandler
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

		struct D3D11BufferData
		{
			BufferType type;
			//uint count = -1; // needed for keeping count of vertex/index count?
			bool updated = false;
			ID3D11Buffer* buffer = nullptr;
			void* UpdatedData[3] = { nullptr, nullptr, nullptr };
		};



		std::unordered_map<SGGuid, D3D11BufferData> buffers;
		std::unordered_map<SGGuid, D3D11ResourceViewData> views;

		std::vector<std::unordered_map<SGGuid, SGGuid>> entityData; // the entity leads to a position in the vector, and the guid is used to retrieve the guid of the actual buffer/view

		ID3D11Device* device;

	};
}
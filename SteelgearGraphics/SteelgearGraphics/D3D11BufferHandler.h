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

		SGError CreateVertexBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, bool streamout, void* data);
		SGError CreateGlobalVertexBuffer(const SGGuid& guid, UINT size, bool dynamic, bool streamout, void* data);
		SGError CreateIndexBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid, UINT size, bool dynamic, void* data);
		SGError CreateGlobalIndexBuffer(const SGGuid& guid, UINT size, bool dynamic, void* data);

		SGError CreateBuffer(const SGGraphicalEntityID& entity, const SGGuid& guid);


	private:

		struct BufferData
		{
			BufferType type;
			ID3D11Buffer* buffer = nullptr;
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11UnorderedAccessView* uav = nullptr;
		};

		std::vector<std::unordered_map<SGGuid, BufferData>> entityBufferData;
		std::unordered_map<SGGuid, BufferData> globalBufferData;

		ID3D11Device* device;

	};
}
#pragma once

#include <d3d11_4.h>

#include "TripleBufferedData.h"
#include "D3D11CommonTypes.h"

namespace SG
{

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

		union
		{
			VertexBufferData vb;
			IndexBufferData ib;
		} specificData;
		ID3D11Buffer* buffer = nullptr;
		TripleBufferedData<UpdateData> updatedData;

		D3D11BufferData() = default;
		~D3D11BufferData();

		D3D11BufferData(D3D11BufferData&& other);
		const D3D11BufferData& operator=(D3D11BufferData&& other);
	};

}
#pragma once

#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	class D3D11DrawDataHandler : public SGGraphicsHandler
	{
	public:

		D3D11DrawDataHandler();
		~D3D11DrawDataHandler();

		SGResult CreateDrawCall(const SGGuid& guid, UINT startVertexLocation);
		SGResult CreateDrawIndexedCall(const SGGuid& guid, UINT startIndexLocation, INT baseVertexLocation);
		SGResult CreateDrawInstancedCall(const SGGuid& guid, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation);
		SGResult CreateDrawIndexInstancedCall(const SGGuid& guid, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation);


	private:

		enum class DrawType
		{
			DRAW,
			DRAW_INDEXED,
			DRAW_INSTANCED,
			DRAW_INDEXED_INSTANCED
		};

		struct DrawCallData
		{
			UINT startVertexLocation;
		};

		struct DrawIndexedCallData
		{
			UINT startIndexLocation;
			INT baseVertexLocation;
		};

		struct DrawInstancedCallData
		{
			UINT instanceCount;
			UINT startVertexLocation;
			UINT startInstanceLocation;
		};

		struct DrawIndexedInstancedCallData
		{
			UINT instanceCount;
			UINT startIndexLocation;
			INT baseVertexLocation;
			UINT startInstanceLocation;
		};

		struct D3D11DrawData
		{
			DrawType drawType;
			union
			{
				DrawCallData draw;
				DrawIndexedCallData drawIndexed;
				DrawInstancedCallData drawInstanced;
				DrawIndexedInstancedCallData drawIndexedInstanced;
			} data;
		};


		std::unordered_map<SGGuid, D3D11DrawData> drawCalls;

		ID3D11Device* device;

	};
}
#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	enum DrawCallVertexBufferToFetchFrom : UINT
	{
		BUFFER0 = static_cast<UINT>(-1),
		BUFFER1 = static_cast<UINT>(-2),
		BUFFER2 = static_cast<UINT>(-3),
		BUFFER3 = static_cast<UINT>(-4),
		BUFFER4 = static_cast<UINT>(-5),
		BUFFER5 = static_cast<UINT>(-6),
		BUFFER6 = static_cast<UINT>(-7),
		BUFFER7 = static_cast<UINT>(-8),
		BUFFER8 = static_cast<UINT>(-9),
		BUFFER9 = static_cast<UINT>(-10),
		BUFFER10 = static_cast<UINT>(-11),
		BUFFER11 = static_cast<UINT>(-12),
		BUFFER12 = static_cast<UINT>(-13),
		BUFFER13 = static_cast<UINT>(-14),
		BUFFER14 = static_cast<UINT>(-15)
	};

	class D3D11DrawCallHandler : public SGGraphicsHandler
	{
	public:

		D3D11DrawCallHandler(ID3D11Device* device);
		~D3D11DrawCallHandler();

		SGResult CreateDrawCall(const SGGuid& guid, UINT vertexCount = BUFFER0, UINT startVertexLocation = 0);
		SGResult CreateDrawIndexedCall(const SGGuid& guid, UINT indexCount = 0, UINT startIndexLocation = 0, INT baseVertexLocation = 0);
		SGResult CreateDrawInstancedCall(const SGGuid& guid, UINT vertexCountPerInstance = BUFFER0, UINT instanceCount = 0, UINT startVertexLocation = 0, UINT startInstanceLocation = 0);
		SGResult CreateDrawIndexedInstancedCall(const SGGuid& guid, UINT indexCountPerInstance = 0, UINT instanceCount = 0, UINT startIndexLocation = 0, INT baseVertexLocation = 0, UINT startInstanceLocation = 0);

		SGResult BindBufferToEntity(const SGGraphicalEntityID& entity, const SGGuid& callGuid, const SGGuid& bindGuid);
		SGResult BindBufferToGroup(const SGGuid& group, const SGGuid& callGuid, const SGGuid& bindGuid);

	private:

		friend class D3D11RenderEngine;

		enum class DrawType
		{
			DRAW,
			DRAW_INDEXED,
			DRAW_INSTANCED,
			DRAW_INDEXED_INSTANCED
		};

		struct DrawData
		{
			UINT vertexCount;
			UINT startVertexLocation;
		};

		struct DrawIndexedData
		{
			UINT indexCount;
			UINT startIndexLocation;
			INT baseVertexLocation;
		};

		struct DrawInstancedData
		{
			UINT vertexCountPerInstance;
			UINT instanceCount;
			UINT startVertexLocation;
			UINT startInstanceLocation;
		};

		struct DrawIndexedInstanced
		{
			UINT indexCountPerInstance;
			UINT instanceCount;
			UINT startIndexLocation;
			INT  baseVertexLocation;
			UINT startInstanceLocation;
		};

		struct DrawCall
		{
			DrawType type;
			union
			{
				DrawData draw;
				DrawIndexedData drawIndexed;
				DrawInstancedData drawInstanced;
				DrawIndexedInstanced drawIndexedInstanced;
			} data;
		};

		ID3D11Device* device;

		LockableUnorderedMap<SGGuid, DrawCall> drawCalls;

		DrawCall GetDrawCall(const SGGuid& guid);
		DrawCall GetDrawCall(const SGGuid& guid, const SGGuid& groupGuid);
		DrawCall GetDrawCall(const SGGuid& guid, const SGGraphicalEntityID& entity);
	};
}
#include "D3D11DrawCallHandler.h"

SG::D3D11DrawCallHandler::D3D11DrawCallHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11DrawCallHandler::~D3D11DrawCallHandler()
{
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDrawCall(const SGGuid & guid, UINT vertexCount, UINT startVertexLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW;
	toStore.data.draw.vertexCount = vertexCount;
	toStore.data.draw.startVertexLocation = startVertexLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDrawIndexedCall(const SGGuid & guid, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INDEXED;
	toStore.data.drawIndexed.indexCount = indexCount;
	toStore.data.drawIndexed.startIndexLocation = startIndexLocation;
	toStore.data.drawIndexed.baseVertexLocation = baseVertexLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDrawInstancedCall(const SGGuid & guid, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INSTANCED;
	toStore.data.drawInstanced.vertexCountPerInstance = vertexCountPerInstance;
	toStore.data.drawInstanced.instanceCount = instanceCount;
	toStore.data.drawInstanced.startVertexLocation = startVertexLocation;
	toStore.data.drawInstanced.startInstanceLocation = startInstanceLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDrawIndexedInstancedCall(const SGGuid & guid, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INDEXED_INSTANCED;
	toStore.data.drawIndexedInstanced.indexCountPerInstance = indexCountPerInstance;
	toStore.data.drawIndexedInstanced.instanceCount = instanceCount;
	toStore.data.drawIndexedInstanced.startIndexLocation = startIndexLocation;
	toStore.data.drawIndexedInstanced.baseVertexLocation = baseVertexLocation;
	toStore.data.drawIndexedInstanced.startInstanceLocation = startInstanceLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDispatchCall(const SGGuid & guid, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
	DispatchCall toStore;
	toStore.indirect = false;
	toStore.data.dispatch.threadGroupCountX = threadGroupCountX;
	toStore.data.dispatch.threadGroupCountY = threadGroupCountY;
	toStore.data.dispatch.threadGroupCountZ = threadGroupCountZ;

	dispatchCalls.lock();
	dispatchCalls[guid] = toStore;
	dispatchCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDispatchIndirectCall(const SGGuid & guid, const SGGuid & bufferGuid, UINT alignedByteOffsetForArgs)
{
	DispatchCall toStore;
	toStore.indirect = true;
	toStore.data.dispatchIndirect.bufferForArgs = bufferGuid;
	toStore.data.dispatchIndirect.alignedByteOffsetForArgs = alignedByteOffsetForArgs;

	dispatchCalls.lock();
	dispatchCalls[guid] = toStore;
	dispatchCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::BindDrawCallToEntity(const SGGraphicalEntityID & entity, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, callGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		drawCalls.lock();
		if (drawCalls.find(callGuid) == drawCalls.end())
		{
			drawCalls.unlock();
			return SGResult::GUID_MISSING;
		}
		drawCalls.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::BindDrawCallToGroup(const SGGuid & group, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, callGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		drawCalls.lock();
		if (drawCalls.find(callGuid) == drawCalls.end())
		{
			drawCalls.unlock();
			return SGResult::GUID_MISSING;
		}
		drawCalls.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::BindDispatchCallToEntity(const SGGraphicalEntityID & entity, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, callGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		dispatchCalls.lock();
		if (dispatchCalls.find(callGuid) == dispatchCalls.end())
		{
			dispatchCalls.unlock();
			return SGResult::GUID_MISSING;
		}
		dispatchCalls.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::BindDispatchCallToGroup(const SGGuid & group, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, callGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		dispatchCalls.lock();
		if (dispatchCalls.find(callGuid) == dispatchCalls.end())
		{
			dispatchCalls.unlock();
			return SGResult::GUID_MISSING;
		}
		dispatchCalls.unlock();
	}

	return SGResult::OK;
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid)
{
	drawCalls.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (drawCalls.find(guid) == drawCalls.end())
		{
			drawCalls.unlock();
			throw std::runtime_error("Error, missing guid when fetching draw call");
		}
	}

	DrawCall& toReturn = drawCalls[guid];
	drawCalls.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	drawCalls.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (drawCalls.find(groupData[groupGuid][guid].GetActive()) == drawCalls.end())
		{
			drawCalls.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching draw call");
		}
	}

	DrawCall& toReturn = drawCalls[groupData[groupGuid][guid].GetActive()];
	drawCalls.unlock();
	groupData.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	drawCalls.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (drawCalls.find(entityData[entity][guid].GetActive()) == drawCalls.end())
		{
			drawCalls.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching draw call");
		}
	}

	DrawCall& toReturn = drawCalls[entityData[entity][guid].GetActive()];
	drawCalls.unlock();
	entityData.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11DrawCallHandler::GetDispatchCall(const SGGuid & guid)
{
	dispatchCalls.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (dispatchCalls.find(guid) == dispatchCalls.end())
		{
			dispatchCalls.unlock();
			throw std::runtime_error("Error, missing guid when fetching dispatch call");
		}
	}

	DispatchCall& toReturn = dispatchCalls[guid];
	dispatchCalls.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11DrawCallHandler::GetDispatchCall(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	dispatchCalls.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (dispatchCalls.find(groupData[groupGuid][guid].GetActive()) == dispatchCalls.end())
		{
			dispatchCalls.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching disptach call");
		}
	}

	DispatchCall& toReturn = dispatchCalls[groupData[groupGuid][guid].GetActive()];
	dispatchCalls.unlock();
	groupData.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11DrawCallHandler::GetDispatchCall(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	dispatchCalls.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (dispatchCalls.find(entityData[entity][guid].GetActive()) == dispatchCalls.end())
		{
			dispatchCalls.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching dispatch call");
		}
	}

	DispatchCall& toReturn = dispatchCalls[entityData[entity][guid].GetActive()];
	dispatchCalls.unlock();
	entityData.unlock();
	return toReturn;
}

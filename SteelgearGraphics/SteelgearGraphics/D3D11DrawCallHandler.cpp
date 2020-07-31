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

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		drawCalls.lock();
		if (drawCalls.find(guid) == drawCalls.end())
		{
			drawCalls.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
		drawCalls.unlock();
	}

	drawCalls.lock();
	DrawCall& toReturn = drawCalls[guid];
	drawCalls.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid, const SGGuid & groupGuid)
{
	if constexpr (DEBUG_VERSION)
	{
		groupData.lock();
		drawCalls.lock();
		if (drawCalls.find(groupData[groupGuid][guid].GetActive()) == drawCalls.end())
		{
			drawCalls.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
		drawCalls.unlock();
		groupData.unlock();
	}

	groupData.lock();
	drawCalls.lock();
	DrawCall& toReturn = drawCalls[groupData[groupGuid][guid].GetActive()];
	drawCalls.unlock();
	groupData.unlock();
	return toReturn;
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	if constexpr (DEBUG_VERSION)
	{
		entityData.lock();
		drawCalls.lock();
		if (drawCalls.find(entityData[entity][guid].GetActive()) == drawCalls.end())
		{
			drawCalls.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
		drawCalls.unlock();
		entityData.unlock();
	}

	entityData.lock();
	drawCalls.lock();
	DrawCall& toReturn = drawCalls[entityData[entity][guid].GetActive()];
	drawCalls.unlock();
	entityData.unlock();
	return toReturn;
}

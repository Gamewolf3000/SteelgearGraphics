#include "D3D11DrawCallHandler.h"

SG::D3D11DrawCallHandler::D3D11DrawCallHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDrawCall(const SGGuid & guid, UINT vertexCount, UINT startVertexLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW;
	toStore.data.draw.vertexCount = vertexCount;
	toStore.data.draw.startVertexLocation = startVertexLocation;
	drawCalls.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDrawIndexedCall(const SGGuid & guid, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INDEXED;
	toStore.data.drawIndexed.indexCount = indexCount;
	toStore.data.drawIndexed.startIndexLocation = startIndexLocation;
	toStore.data.drawIndexed.baseVertexLocation = baseVertexLocation;
	drawCalls.AddElement(guid, std::move(toStore));

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
	drawCalls.AddElement(guid, std::move(toStore));

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
	drawCalls.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11DrawCallHandler::RemoveDrawCall(const SGGuid& guid)
{
	drawCalls.RemoveElement(guid);
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDispatchCall(const SGGuid & guid, UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ)
{
	DispatchCall toStore;
	toStore.indirect = false;
	toStore.data.dispatch.threadGroupCountX = threadGroupCountX;
	toStore.data.dispatch.threadGroupCountY = threadGroupCountY;
	toStore.data.dispatch.threadGroupCountZ = threadGroupCountZ;
	dispatchCalls.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11DrawCallHandler::CreateDispatchIndirectCall(const SGGuid & guid, const SGGuid & bufferGuid, UINT alignedByteOffsetForArgs)
{
	DispatchCall toStore;
	toStore.indirect = true;
	toStore.data.dispatchIndirect.bufferForArgs = bufferGuid;
	toStore.data.dispatchIndirect.alignedByteOffsetForArgs = alignedByteOffsetForArgs;
	dispatchCalls.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11DrawCallHandler::RemoveDispatchCall(const SGGuid& guid)
{
	dispatchCalls.RemoveElement(guid);
}

SG::SGResult SG::D3D11DrawCallHandler::BindDrawCallToEntity(const SGGraphicalEntityID & entity, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, callGuid, bindGuid, drawCalls);
}

SG::SGResult SG::D3D11DrawCallHandler::BindDrawCallToGroup(const SGGuid & group, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, callGuid, bindGuid, drawCalls);
}

SG::SGResult SG::D3D11DrawCallHandler::BindDispatchCallToEntity(const SGGraphicalEntityID & entity, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, callGuid, bindGuid, dispatchCalls);
}

SG::SGResult SG::D3D11DrawCallHandler::BindDispatchCallToGroup(const SGGuid & group, const SGGuid & callGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, callGuid, bindGuid, dispatchCalls);
}

void SG::D3D11DrawCallHandler::FinishFrame()
{
	SG::SGGraphicsHandler::FinishFrame();

	drawCalls.FinishFrame();
	dispatchCalls.FinishFrame();
}

void SG::D3D11DrawCallHandler::SwapFrame()
{
	SG::SGGraphicsHandler::SwapFrame();

	drawCalls.UpdateActive();
	dispatchCalls.UpdateActive();
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid)
{
	return SG::SGGraphicsHandler::GetGlobalElement(guid, drawCalls, "draw call");
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid, const SGGuid & groupGuid)
{
	return SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, drawCalls, "draw call");
}

SG::D3D11DrawCallHandler::DrawCall SG::D3D11DrawCallHandler::GetDrawCall(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	return SG::SGGraphicsHandler::GetEntityElement(guid, entity, drawCalls, "draw call");
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11DrawCallHandler::GetDispatchCall(const SGGuid & guid)
{
	return SG::SGGraphicsHandler::GetGlobalElement(guid, dispatchCalls, "dispatch call");
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11DrawCallHandler::GetDispatchCall(const SGGuid & guid, const SGGuid & groupGuid)
{
	return SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, dispatchCalls, "dispatch call");
}

SG::D3D11DrawCallHandler::DispatchCall SG::D3D11DrawCallHandler::GetDispatchCall(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	return SG::SGGraphicsHandler::GetEntityElement(guid, entity, dispatchCalls, "dispatch call");
}

#include "SGGraphicsHandler.h"

void SG::SGGraphicsHandler::UpdateEntity(const SGGraphicalEntityID & entity, const SGGuid & resourceGuid, const SGGuid & bindGuid)
{
	entityData.AddElement(entity, bindGuid, resourceGuid);

	frameBufferEntityMutex.lock();
	updatedEntitiesFrameBuffer.push_back(std::make_pair(entity, bindGuid));
	frameBufferEntityMutex.unlock();
}

void SG::SGGraphicsHandler::UpdateGroup(const SGGuid & group, const SGGuid & resourceGuid, const SGGuid & bindGuid)
{
	groupData.AddElement(group, bindGuid, resourceGuid);

	frameBufferGroupMutex.lock();
	updatedGroupsFrameBuffer.push_back(std::make_pair(group, bindGuid));
	frameBufferGroupMutex.unlock();
}

void SG::SGGraphicsHandler::FinishFrame()
{
	// No need to lock since this function is called only by the render engine during certain conditions

	entityData.FinishFrame();

	for (auto& pair : updatedEntitiesFrameBuffer)
		entityData.GetElement(pair.first, pair.second).SwitchUpdateBuffer();

	updatedEntitiesTotalBuffer.insert(updatedEntitiesTotalBuffer.end(), updatedEntitiesFrameBuffer.begin(), updatedEntitiesFrameBuffer.end());
	updatedEntitiesFrameBuffer.clear();

	groupData.FinishFrame();

	for (auto& pair : updatedGroupsFrameBuffer)
		groupData.GetElement(pair.first, pair.second).SwitchUpdateBuffer();

	updatedGroupsTotalBuffer.insert(updatedGroupsTotalBuffer.end(), updatedGroupsFrameBuffer.begin(), updatedGroupsFrameBuffer.end());
	updatedGroupsFrameBuffer.clear();
}

void SG::SGGraphicsHandler::SwapFrame()
{
	// No need to lock since this function is called only by the render engine during certain conditions

	entityData.UpdateActive();

	for (auto& pair : updatedEntitiesTotalBuffer)
		entityData[pair.first][pair.second].SwitchActiveBuffer();

	updatedEntitiesTotalBuffer.clear();
	groupData.UpdateActive();

	for (auto& pair : updatedGroupsTotalBuffer)
		groupData[pair.first][pair.second].SwitchActiveBuffer();

	updatedGroupsTotalBuffer.clear();
}
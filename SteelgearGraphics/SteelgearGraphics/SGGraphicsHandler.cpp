#include "SGGraphicsHandler.h"

void SG::SGGraphicsHandler::UpdateEntity(const SGGraphicalEntityID & entity, const SGGuid & resourceGuid, const SGGuid & bindGuid)
{
	entityData.lock();
	entityData[entity][bindGuid] = resourceGuid;
	entityData.unlock();

	frameBufferEntityMutex.lock();
	updatedEntitiesFrameBuffer.push_back(std::make_pair(entity, bindGuid));
	frameBufferEntityMutex.unlock();
}

void SG::SGGraphicsHandler::UpdateGroup(const SGGuid & group, const SGGuid & resourceGuid, const SGGuid & bindGuid)
{
	groupData.lock();
	groupData[group][bindGuid] = resourceGuid;
	groupData.unlock();

	frameBufferGroupMutex.lock();
	updatedGroupsFrameBuffer.push_back(std::make_pair(group, bindGuid));
	frameBufferGroupMutex.unlock();
}

void SG::SGGraphicsHandler::SwapUpdateBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	for (auto& pair : updatedEntitiesFrameBuffer)
		entityData[pair.first][pair.second].SwitchUpdateBuffer();

	updatedEntitiesTotalBuffer.insert(updatedEntitiesTotalBuffer.end(), updatedEntitiesFrameBuffer.begin(), updatedEntitiesFrameBuffer.end());

	for (auto& pair : updatedGroupsFrameBuffer)
		groupData[pair.first][pair.second].SwitchUpdateBuffer();

	updatedGroupsTotalBuffer.insert(updatedGroupsTotalBuffer.end(), updatedGroupsFrameBuffer.begin(), updatedGroupsFrameBuffer.end());
}

void SG::SGGraphicsHandler::SwapToWorkWithBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	for (auto& pair : updatedEntitiesTotalBuffer)
		entityData[pair.first][pair.second].SwitchActiveBuffer();

	updatedEntitiesTotalBuffer.clear();

	for (auto& pair : updatedGroupsTotalBuffer)
		groupData[pair.first][pair.second].SwitchActiveBuffer();

	updatedGroupsTotalBuffer.clear();
}

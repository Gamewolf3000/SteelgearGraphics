#pragma once

#include <mutex>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"
#include "LockableUnorderedMap.h"
#include "TripleBufferedData.h"

namespace SG
{
	class SGGraphicsHandler
	{
	public:
		SGGraphicsHandler() {};
		virtual ~SGGraphicsHandler() {};

	protected:

		LockableUnorderedMap<SGGraphicalEntityID, std::unordered_map<SGGuid, TripleBufferedData<SGGuid>>> entityData; // the entity and a guid leads to another guid, and that guid is used to retrieve the guid of the actual resource
		LockableUnorderedMap<SGGuid, std::unordered_map<SGGuid, TripleBufferedData<SGGuid>>> groupData; // the group and a guid leads to another guid, and that guid is used to retrieve the guid of the actual resource

		std::mutex frameBufferEntityMutex;
		std::vector<std::pair<SGGraphicalEntityID, SGGuid>> updatedEntitiesFrameBuffer;
		std::vector<std::pair<SGGraphicalEntityID, SGGuid>> updatedEntitiesTotalBuffer;

		std::mutex frameBufferGroupMutex;
		std::vector<std::pair<SGGuid, SGGuid>> updatedGroupsFrameBuffer;
		std::vector<std::pair<SGGuid, SGGuid>> updatedGroupsTotalBuffer;

		void UpdateEntity(const SGGraphicalEntityID & entity, const SGGuid & resourceGuid, const SGGuid & bindGuid);
		void UpdateGroup(const SGGuid & group, const SGGuid & resourceGuid, const SGGuid & bindGuid);

		virtual void SwapUpdateBuffer();
		virtual void SwapToWorkWithBuffer();
	};
}
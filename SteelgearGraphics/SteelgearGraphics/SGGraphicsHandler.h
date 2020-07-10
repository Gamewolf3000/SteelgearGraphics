#pragma once

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"
#include "LockableUnorderedMap.h"

namespace SG
{
	class SGGraphicsHandler
	{
	public:
		SGGraphicsHandler() {};
		virtual ~SGGraphicsHandler() {};

	protected:

		LockableUnorderedMap<SGGraphicalEntityID, std::unordered_map<SGGuid, SGGuid>> entityData; // the entity and a guid leads to another guid, and that guid is used to retrieve the guid of the actual resource
		LockableUnorderedMap<SGGraphicalEntityGroupID, std::unordered_map<SGGuid, SGGuid>> groupData; // the group and a guid leads to another guid, and that guid is used to retrieve the guid of the actual resource


	};
}
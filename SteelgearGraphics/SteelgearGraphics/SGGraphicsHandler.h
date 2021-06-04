#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <functional>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"
#include "LayeredFrameMap.h"
#include "TripleBufferedData.h"

namespace SG
{
	class SGGraphicsHandler
	{
	public:
		SGGraphicsHandler() = default;
		virtual ~SGGraphicsHandler() = default;

	protected:

		LayeredFrameMap<SGGraphicalEntityID, SGGuid, TripleBufferedData<SGGuid>> entityData; // the entity and a guid leads to another guid, and that guid is used to retrieve the guid of the actual resource
		LayeredFrameMap<SGGuid, SGGuid, TripleBufferedData<SGGuid>> groupData; // the group and a guid leads to another guid, and that guid is used to retrieve the guid of the actual resource

		std::mutex frameBufferEntityMutex;
		std::vector<std::pair<SGGraphicalEntityID, SGGuid>> updatedEntitiesFrameBuffer;
		std::vector<std::pair<SGGraphicalEntityID, SGGuid>> updatedEntitiesTotalBuffer;

		std::mutex frameBufferGroupMutex;
		std::vector<std::pair<SGGuid, SGGuid>> updatedGroupsFrameBuffer;
		std::vector<std::pair<SGGuid, SGGuid>> updatedGroupsTotalBuffer;

		void UpdateEntity(const SGGraphicalEntityID & entity, const SGGuid & resourceGuid, const SGGuid & bindGuid);
		void UpdateGroup(const SGGuid & group, const SGGuid & resourceGuid, const SGGuid & bindGuid);

		template<typename T>
		SG::SGResult BindElementToEntity(const SGGraphicalEntityID& entity, const SGGuid& elementGuid, const SGGuid& bindGuid,
										 FrameMap<SGGuid, T>& elementMap);
		template<typename T>
		SG::SGResult BindElementToGroup(const SGGuid& group, const SGGuid& elementGuid, const SGGuid& bindGuid,
										FrameMap<SGGuid, T>& elementMap);

		virtual void FinishFrame();
		virtual void SwapFrame();

		template<typename T>
		T& GetGlobalElement(const SGGuid& guid, FrameMap<SGGuid, T>& elementMap, const std::string& resourceName, 
							const std::vector<std::function<void(const T&)>>& checks = std::vector<std::function<void(const T&)>>());
		template<typename T>
		T& GetGroupElement(const SGGuid& guid, const SGGuid& groupGuid, FrameMap<SGGuid, T>& elementMap, const std::string& resourceName, 
						   const std::vector<std::function<void(const T&)>>& checks = std::vector<std::function<void(const T&)>>());
		template<typename T>
		T& GetEntityElement(const SGGuid& guid, const SGGraphicalEntityID& entity, FrameMap<SGGuid, T>& elementMap,
							const std::string& resourceName, 
							const std::vector<std::function<void(const T&)>>& checks = std::vector<std::function<void(const T&)>>());
	};

	template<typename T>
	inline SG::SGResult SGGraphicsHandler::BindElementToEntity(const SGGraphicalEntityID& entity, const SGGuid& elementGuid,
															   const SGGuid& bindGuid, FrameMap<SGGuid, T>& elementMap)
	{
		this->UpdateEntity(entity, elementGuid, bindGuid);

		if constexpr (DEBUG_VERSION)
		{
			if (!elementMap.Exists(elementGuid))
				return SGResult::GUID_MISSING;
		}


		return SGResult::OK;
	}

	template<typename T>
	inline SG::SGResult SGGraphicsHandler::BindElementToGroup(const SGGuid& group, const SGGuid& elementGuid, const SGGuid& bindGuid,
															  FrameMap<SGGuid, T>& elementMap)
	{
		UpdateGroup(group, elementGuid, bindGuid);

		if constexpr (DEBUG_VERSION)
		{
			if (!elementMap.Exists(elementGuid))
				return SGResult::GUID_MISSING;
		}

		return SGResult::OK;
	}

	template<typename T>
	inline T& SGGraphicsHandler::GetGlobalElement(const SGGuid& guid, FrameMap<SGGuid, T>& elementMap, const std::string& resourceName,
												  const std::vector<std::function<void(const T&)>>& checks)
	{
		(void)resourceName; // Ugly solution for now. Since resourcename and checks is only used in debug mode the parameter is unused in release builds.
		(void)checks;
		if constexpr (DEBUG_VERSION)
		{
			if (!elementMap.HasElement(guid))
				throw std::runtime_error("Error, missing guid when fetching " + resourceName);

			for (auto& check : checks)
				check(elementMap[guid]);
		}

		return elementMap[guid];
	}

	template<typename T>
	inline T& SGGraphicsHandler::GetGroupElement(const SGGuid& guid, const SGGuid& groupGuid, FrameMap<SGGuid, T>& elementMap, 
												 const std::string& resourceName, const std::vector<std::function<void(const T&)>>& checks)
	{
		(void)resourceName; // Ugly solution for now. Since resourcename and checks is only used in debug mode the parameter is unused in release builds.
		(void)checks;
		if constexpr (DEBUG_VERSION)
		{
			if (!groupData.HasElement(groupGuid))
				throw std::runtime_error("Error, group with guid not found when fetching " + resourceName);

			if (!groupData[groupGuid].HasElement(guid))
				throw std::runtime_error("Error, guid not found in group when fetching " + resourceName);

			if (!elementMap.HasElement(groupData[groupGuid][guid].GetActive()))
				throw std::runtime_error("Error, missing guid when fetching " + resourceName);

			for (auto& check : checks)
				check(elementMap[guid]);
		}

		return elementMap[groupData[groupGuid][guid].GetActive()];
	}

	template<typename T>
	inline T& SGGraphicsHandler::GetEntityElement(const SGGuid& guid, const SGGraphicalEntityID& entity, FrameMap<SGGuid, T>& elementMap,
												  const std::string& resourceName, const std::vector<std::function<void(const T&)>>& checks)
	{
		(void)resourceName; // Ugly solution for now. Since resourcename and checks is only used in debug mode the parameter is unused in release builds.
		(void)checks;
		if constexpr (DEBUG_VERSION)
		{
			if (!entityData.HasElement(entity))
				throw std::runtime_error("Error, entity with guid not found when fetching " + resourceName);

			if (!entityData[entity].HasElement(guid))
				throw std::runtime_error("Error, guid not found in entity when fetching " + resourceName);

			if (!elementMap.HasElement(entityData[entity][guid].GetActive()))
				throw std::runtime_error("Error, missing guid when fetching " + resourceName);

			for (auto& check : checks)
				check(elementMap[guid]);
		}

		return elementMap[entityData[entity][guid].GetActive()];
	}
}
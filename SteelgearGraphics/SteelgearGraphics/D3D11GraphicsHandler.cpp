#include "D3D11GraphicsHandler.h"

void* SG::D3D11GraphicsHandler::GetGlobalResourceView(const SGGuid& guid, const ResourceViewType& resourceViewType,
												FrameMap<SGGuid, D3D11ResourceViewData>& views, 
												const std::string& associatedResourceName)
{
	(void)associatedResourceName; // Ugly solution for now. Since resourcename is only used in debug mode the parameter is unused in release builds.
	if constexpr (DEBUG_VERSION)
	{
		std::string viewTypeString = TranslateViewToString(resourceViewType);
		if (!views.HasElement(guid))
			throw std::runtime_error("Error fetching " + viewTypeString + 
									 ", guid does not exist. Associated resource was: " + associatedResourceName);

		if (views[guid].type != resourceViewType)
			throw std::runtime_error("Error fetching " + viewTypeString + ", guid does not match an the expected view type."
									 + " Got: " + TranslateViewToString(views[guid].type) + "."
									 + " Associated resource was: " + associatedResourceName);
	}

	return views[guid].view.srv;
	// Since all are pointers it does not actually matter what pointer is returned here as long
	// as it actually points to the correct type of view and is received as the correct type
}

void* SG::D3D11GraphicsHandler::GetGroupResourceView(const SGGuid& guid, const SGGuid& groupGuid, const ResourceViewType& resourceViewType,
													 FrameMap<SGGuid, D3D11ResourceViewData>& views, 
													 const std::string& associatedResourceName)
{
	(void)associatedResourceName; // Ugly solution for now. Since resourcename is only used in debug mode the parameter is unused in release builds.
	if constexpr (DEBUG_VERSION)
	{
		std::string viewTypeString = TranslateViewToString(resourceViewType);

		if (!groupData.HasElement(groupGuid))
			throw std::runtime_error("Error, group with guid not found when fetching " + viewTypeString
									 + " Associated resource was: " + associatedResourceName);

		if (!groupData[groupGuid].HasElement(guid))
			throw std::runtime_error("Error, guid not found in group when fetching " + viewTypeString
				+ " Associated resource was: " + associatedResourceName);

		if (!views.HasElement(groupData[groupGuid][guid].GetActive()))
			throw std::runtime_error("Error fetching " + viewTypeString +
				", guid does not exist. Associated resource was: " + associatedResourceName);

		if (views[groupData[groupGuid][guid].GetActive()].type != resourceViewType)
			throw std::runtime_error("Error fetching " + viewTypeString + ", guid does not match an the expected view type."
				+ " Got: " + TranslateViewToString(views[guid].type) + "."
				+ " Associated resource was : " + associatedResourceName);
	}

	return views[groupData[groupGuid][guid].GetActive()].view.srv;
	// Since all are pointers it does not actually matter what pointer is returned here as long
	// as it actually points to the correct type of view and is received as the correct type
}

void* SG::D3D11GraphicsHandler::GetEntityResourceView(const SGGuid& guid, const SGGraphicalEntityID& entity, const ResourceViewType& resourceViewType, FrameMap<SGGuid, D3D11ResourceViewData>& views, const std::string& associatedResourceName)
{
	(void)associatedResourceName; // Ugly solution for now. Since resourcename is only used in debug mode the parameter is unused in release builds.
	if constexpr (DEBUG_VERSION)
	{
		std::string viewTypeString = TranslateViewToString(resourceViewType);

		if (!entityData.HasElement(entity))
			throw std::runtime_error("Error, entity with guid not found when fetching " + viewTypeString
				+ " Associated resource was: " + associatedResourceName);

		if (!entityData[entity].HasElement(guid))
			throw std::runtime_error("Error, guid not found in entity when fetching " + viewTypeString
				+ " Associated resource was: " + associatedResourceName);

		if (!views.HasElement(entityData[entity][guid].GetActive()))
			throw std::runtime_error("Error fetching " + viewTypeString +
				", guid does not exist. Associated resource was: " + associatedResourceName);

		if (views[entityData[entity][guid].GetActive()].type != resourceViewType)
			throw std::runtime_error("Error fetching " + viewTypeString + ", guid does not match an the expected view type."
				+ " Got: " + TranslateViewToString(views[guid].type) + "."
				+ " Associated resource was : " + associatedResourceName);
	}

	return views[entityData[entity][guid].GetActive()].view.srv;
	// Since all are pointers it does not actually matter what pointer is returned here as long
	// as it actually points to the correct type of view and is received as the correct type
}

std::string SG::D3D11GraphicsHandler::TranslateViewToString(const ResourceViewType& resourceViewType)
{
	switch (resourceViewType)
	{
	case ResourceViewType::SRV:
		return "srv";
		break;
	case ResourceViewType::UAV:
		return "uav";
		break;
	case ResourceViewType::DSV: 
		return "dsv";
		break;
	case ResourceViewType::RTV:
		return "rtv";
		break;
	default:
		return "ERROR VIEW";
		break;
	}
}

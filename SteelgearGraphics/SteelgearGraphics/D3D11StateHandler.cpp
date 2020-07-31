#include "D3D11StateHandler.h"

SG::D3D11StateHandler::D3D11StateHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11StateHandler::~D3D11StateHandler()
{
}

SG::SGResult SG::D3D11StateHandler::CreateViewport(const SGGuid & guid, FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height, FLOAT minDepth, FLOAT maxDepth)
{
	D3D11ViewportData toStore;

	toStore.viewport.TopLeftX = topLeftX;
	toStore.viewport.TopLeftY = topLeftY;
	toStore.viewport.Width = width;
	toStore.viewport.Height = height;
	toStore.viewport.MinDepth = minDepth;
	toStore.viewport.MaxDepth = maxDepth;

	viewports.lock();
	viewports[guid] = toStore;
	viewports.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11StateHandler::BindViewportToEntity(const SGGraphicalEntityID & entity, const SGGuid & viewportGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, viewportGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		viewports.lock();
		if (viewports.find(viewportGuid) == viewports.end())
		{
			viewports.unlock();
			return SGResult::GUID_MISSING;
		}
		viewports.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11StateHandler::BindViewportToGroup(const SGGuid & group, const SGGuid & viewportGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, viewportGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		viewports.lock();
		if (viewports.find(viewportGuid) == viewports.end())
		{
			viewports.unlock();
			return SGResult::GUID_MISSING;
		}
		viewports.unlock();
	}

	return SGResult::OK;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		viewports.lock();
		if (viewports.find(guid) == viewports.end())
		{
			viewports.unlock();
			throw std::runtime_error("Error, missing guid when fetching viewport");
		}
		viewports.unlock();
	}

	viewports.lock();
	const D3D11_VIEWPORT& toReturn = viewports[guid].viewport;
	viewports.unlock();
	return toReturn;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid, const SGGuid & groupGuid)
{
	if constexpr (DEBUG_VERSION)
	{
		groupData.lock();
		viewports.lock();
		if (viewports.find(groupData[groupGuid][guid].GetActive()) == viewports.end())
		{
			viewports.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching viewport");
		}
		viewports.unlock();
		groupData.unlock();
	}

	groupData.lock();
	viewports.lock();
	const D3D11_VIEWPORT& toReturn = viewports[groupData[groupGuid][guid].GetActive()].viewport;
	viewports.unlock();
	groupData.unlock();
	return toReturn;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	if constexpr (DEBUG_VERSION)
	{
		entityData.lock();
		viewports.lock();
		if (viewports.find(entityData[entity][guid].GetActive()) == viewports.end())
		{
			viewports.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching viewport");
		}
		viewports.unlock();
		entityData.unlock();
	}

	entityData.lock();
	viewports.lock();
	const D3D11_VIEWPORT& toReturn = viewports[entityData[entity][guid].GetActive()].viewport;
	viewports.unlock();
	entityData.unlock();
	return toReturn;
}

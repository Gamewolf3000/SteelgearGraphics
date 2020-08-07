#include "D3D11StateHandler.h"

SG::D3D11StateHandler::D3D11StateHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11StateHandler::~D3D11StateHandler()
{
	for (auto& state : states)
	{
		switch (state.second.type)
		{
		case StateType::RASTERIZER:
			ReleaseCOM(state.second.state.rasterizer);
			break;
		case StateType::DEPTH_STENCIL:
			ReleaseCOM(state.second.state.depthStencil);
			break;
		case StateType::BLEND:
			ReleaseCOM(state.second.state.blend);
			break;
		}
	}
}

SG::SGResult SG::D3D11StateHandler::CreateRasterizerState(const SGGuid & guid, FillMode fill, CullMode cull, BOOL frontCounterClockwise, INT depthBias, FLOAT depthBiasClamp, FLOAT slopeScaledDepthBias, BOOL depthClipEnable, BOOL scissorEnable, BOOL multisampleEnable, BOOL antialiasedEnable)
{
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = fill == FillMode::SOLID ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
	
	switch (cull)
	{
	case SG::CullMode::NONE:
		desc.CullMode = D3D11_CULL_NONE;
		break;
	case SG::CullMode::FRONT:
		desc.CullMode = D3D11_CULL_FRONT;
		break;
	case SG::CullMode::BACK:
		desc.CullMode = D3D11_CULL_BACK;
		break;
	default:
		break;
	}

	desc.FrontCounterClockwise = frontCounterClockwise;
	desc.DepthBias = depthBias;
	desc.DepthBiasClamp = depthBiasClamp;
	desc.SlopeScaledDepthBias = slopeScaledDepthBias;
	desc.DepthClipEnable = depthClipEnable;
	desc.ScissorEnable = scissorEnable;
	desc.MultisampleEnable = multisampleEnable;
	desc.AntialiasedLineEnable = antialiasedEnable;

	D3D11StateData toStore;
	toStore.type = StateType::RASTERIZER;
	if (FAILED(device->CreateRasterizerState(&desc, &toStore.state.rasterizer)))
		return SGResult::FAIL;

	states.lock();
	states[guid] = toStore;
	states.unlock();

	return SGResult::OK;
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

SG::SGResult SG::D3D11StateHandler::BindStateToEntity(const SGGraphicalEntityID & entity, const SGGuid & stateGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, stateGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		states.lock();
		if (states.find(stateGuid) == states.end())
		{
			states.unlock();
			return SGResult::GUID_MISSING;
		}
		states.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11StateHandler::BindStateToGroup(const SGGuid & group, const SGGuid & stateGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, stateGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		states.lock();
		if (states.find(stateGuid) == states.end())
		{
			states.unlock();
			return SGResult::GUID_MISSING;
		}
		states.unlock();
	}

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

ID3D11RasterizerState * SG::D3D11StateHandler::GetRazterizerState(const SGGuid & guid)
{
	states.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (states.find(guid) == states.end())
		{
			states.unlock();
			throw std::runtime_error("Error fetching rasterizer state, guid does not exist");
		}

		if (states[guid].type != StateType::RASTERIZER)
		{
			states.unlock();
			throw std::runtime_error("Error fetching rasterizer state, guid does not match a rasterizer state");
		}
	}

	auto toReturn = states[guid].state.rasterizer;
	states.unlock();

	return toReturn;
}

ID3D11RasterizerState * SG::D3D11StateHandler::GetRazterizerState(const SGGuid & guid, const SGGuid & groupGuid)
{
	states.lock();
	groupData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (groupData.find(groupGuid) == groupData.end())
		{
			states.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, group does not exist");
		}

		if (groupData[groupGuid].find(guid) == groupData[groupGuid].end())
		{
			states.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, group does not have the guid");
		}

		if (states.find(groupData[groupGuid][guid].GetActive()) == states.end())
		{
			states.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, guid does not exist");
		}

		if (states[groupData[groupGuid][guid].GetActive()].type != StateType::RASTERIZER)
		{
			states.unlock();
			groupData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, guid does not match a rasterizer state");
		}
	}

	auto toReturn = states[groupData[groupGuid][guid].GetActive()].state.rasterizer;
	groupData.unlock();
	states.unlock();

	return toReturn;
}

ID3D11RasterizerState * SG::D3D11StateHandler::GetRazterizerState(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	states.lock();
	entityData.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (entityData.find(entity) == entityData.end())
		{
			states.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, entity does not exist");
		}

		if (entityData[entity].find(guid) == entityData[entity].end())
		{
			states.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, entity does not have the guid");
		}

		if (states.find(entityData[entity][guid].GetActive()) == states.end())
		{
			states.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, guid does not exist");
		}

		if (states[entityData[entity][guid].GetActive()].type != StateType::RASTERIZER)
		{
			states.unlock();
			entityData.unlock();
			throw std::runtime_error("Error fetching rasterizer state, guid does not match an rasterizer state");
		}
	}

	auto toReturn = states[entityData[entity][guid].GetActive()].state.rasterizer;
	entityData.unlock();
	states.unlock();

	return toReturn;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid)
{
	viewports.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (viewports.find(guid) == viewports.end())
		{
			viewports.unlock();
			throw std::runtime_error("Error, missing guid when fetching viewport");
		}
	}

	const D3D11_VIEWPORT& toReturn = viewports[guid].viewport;
	viewports.unlock();
	return toReturn;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	viewports.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (viewports.find(groupData[groupGuid][guid].GetActive()) == viewports.end())
		{
			viewports.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching viewport");
		}
	}

	const D3D11_VIEWPORT& toReturn = viewports[groupData[groupGuid][guid].GetActive()].viewport;
	viewports.unlock();
	groupData.unlock();
	return toReturn;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	viewports.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (viewports.find(entityData[entity][guid].GetActive()) == viewports.end())
		{
			viewports.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching viewport");
		}
	}

	const D3D11_VIEWPORT& toReturn = viewports[entityData[entity][guid].GetActive()].viewport;
	viewports.unlock();
	entityData.unlock();
	return toReturn;
}

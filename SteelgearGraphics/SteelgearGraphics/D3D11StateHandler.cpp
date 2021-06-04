#include "D3D11StateHandler.h"

SG::D3D11StateHandler::D3D11StateHandler(ID3D11Device * device)
{
	this->device = device;
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

	states.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11StateHandler::RemoveState(const SGGuid& guid)
{
	states.RemoveElement(guid);
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

	viewports.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11StateHandler::RemoveViewport(const SGGuid& guid)
{
	viewports.RemoveElement(guid);
}

void SG::D3D11StateHandler::RemoveStateData(const SGGuid& guid)
{
	setData.RemoveElement(guid);
}

SG::SGResult SG::D3D11StateHandler::BindStateToEntity(const SGGraphicalEntityID & entity, const SGGuid & stateGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, stateGuid, bindGuid, states);
}

SG::SGResult SG::D3D11StateHandler::BindStateToGroup(const SGGuid & group, const SGGuid & stateGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, stateGuid, bindGuid, states);
}

SG::SGResult SG::D3D11StateHandler::BindViewportToEntity(const SGGraphicalEntityID & entity, const SGGuid & viewportGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, viewportGuid, bindGuid, viewports);
}

SG::SGResult SG::D3D11StateHandler::BindViewportToGroup(const SGGuid & group, const SGGuid & viewportGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, viewportGuid, bindGuid, viewports);
}

void SG::D3D11StateHandler::FinishFrame()
{
	SG::SGGraphicsHandler::FinishFrame();

	states.FinishFrame();
	setData.FinishFrame();
	viewports.FinishFrame();
}

void SG::D3D11StateHandler::SwapFrame()
{
	SG::SGGraphicsHandler::SwapFrame();

	states.UpdateActive();
	setData.UpdateActive();
	viewports.UpdateActive();
}

ID3D11RasterizerState * SG::D3D11StateHandler::GetRazterizerState(const SGGuid & guid)
{
	auto stateTypeCheck = [](const D3D11StateData& element)
	{
		if (element.type != StateType::RASTERIZER)
			throw std::runtime_error("Error fetching rasterizer state, guid does not match a rasterizer state");
	};

	return SG::SGGraphicsHandler::GetGlobalElement(guid, states, "rasterizer state", { stateTypeCheck }).state.rasterizer;
}

ID3D11RasterizerState * SG::D3D11StateHandler::GetRazterizerState(const SGGuid & guid, const SGGuid & groupGuid)
{
	auto stateTypeCheck = [](const D3D11StateData& element)
	{
		if (element.type != StateType::RASTERIZER)
			throw std::runtime_error("Error fetching rasterizer state, guid does not match a rasterizer state");
	};

	return SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, states, "rasterizer state", { stateTypeCheck }).state.rasterizer;
}

ID3D11RasterizerState * SG::D3D11StateHandler::GetRazterizerState(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	auto stateTypeCheck = [](const D3D11StateData& element)
	{
		if (element.type != StateType::RASTERIZER)
			throw std::runtime_error("Error fetching rasterizer state, guid does not match a rasterizer state");
	};

	return SG::SGGraphicsHandler::GetEntityElement(guid, entity, states, "rasterizer state", { stateTypeCheck }).state.rasterizer;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid)
{
	return SG::SGGraphicsHandler::GetGlobalElement(guid, viewports, "viewport").viewport;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid, const SGGuid & groupGuid)
{
	return SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, viewports, "viewport").viewport;
}

const D3D11_VIEWPORT& SG::D3D11StateHandler::GetViewport(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	return SG::SGGraphicsHandler::GetEntityElement(guid, entity, viewports, "viewport").viewport;
}

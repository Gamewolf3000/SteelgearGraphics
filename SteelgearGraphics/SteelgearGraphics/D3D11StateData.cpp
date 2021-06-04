#include "D3D11StateData.h"

SG::D3D11StateData::~D3D11StateData()
{
	if (state.rasterizer != nullptr) // Since only ptrs it does not matter which is used here
	{
		switch (type)
		{
		case StateType::RASTERIZER:
			state.rasterizer->Release();
			break;
		case StateType::DEPTH_STENCIL:
			state.depthStencil->Release();
			break;
		case StateType::BLEND:
			state.blend->Release();
			break;
		}
	}
}

SG::D3D11StateData::D3D11StateData(D3D11StateData&& other) : type(other.type), state(other.state)
{
	other.state.rasterizer = nullptr; // Since only ptrs it does not matter which is used here
}

SG::D3D11StateData& SG::D3D11StateData::operator=(D3D11StateData&& other)
{
	if (this != &other)
	{
		type = other.type;
		state.rasterizer = other.state.rasterizer; // Since only ptrs it does not matter which is used here
		other.state.rasterizer = nullptr;
	}

	return *this;
}

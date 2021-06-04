#include "D3D11ResourceViewData.h"

SG::D3D11ResourceViewData::~D3D11ResourceViewData()
{
	if (view.srv != nullptr) // Since only ptrs it does not matter which is used here
	{
		switch (type)
		{
		case ResourceViewType::SRV:
			view.srv->Release();
			break;
		case ResourceViewType::UAV:
			view.uav->Release();
			break;
		case ResourceViewType::DSV:
			view.dsv->Release();
			break;
		case ResourceViewType::RTV:
			view.rtv->Release();
			break;
		default:
			break;
		}
	}
}

SG::D3D11ResourceViewData::D3D11ResourceViewData(D3D11ResourceViewData&& other) : type(other.type), view(other.view),
resourceGuid(std::move(other.resourceGuid))
{
	other.view.srv = nullptr; // Since only ptrs it does not matter which is used here
}

SG::D3D11ResourceViewData& SG::D3D11ResourceViewData::operator=(D3D11ResourceViewData&& other)
{
	if (this != &other)
	{
		type = other.type;
		view.srv = other.view.srv; // Since only ptrs it does not matter which is used here
		other.view.srv = nullptr;
		resourceGuid = std::move(other.resourceGuid);
	}

	return *this;
}

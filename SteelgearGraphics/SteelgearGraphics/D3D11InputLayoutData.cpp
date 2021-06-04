#include "D3D11InputLayoutData.h"

SG::D3D11InputLayoutData::~D3D11InputLayoutData()
{
    if (inputLayout != nullptr)
        inputLayout->Release();
}

SG::D3D11InputLayoutData::D3D11InputLayoutData(D3D11InputLayoutData&& other) : inputLayout(other.inputLayout)
{
    other.inputLayout = nullptr;
}

SG::D3D11InputLayoutData& SG::D3D11InputLayoutData::operator=(D3D11InputLayoutData&& other)
{
    if (this != &other)
    {
        inputLayout = other.inputLayout;
        other.inputLayout = nullptr;
    }

    return *this;
}

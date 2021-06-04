#include "D3D11SamplerData.h"

SG::D3D11SamplerData::~D3D11SamplerData()
{
	if (sampler)
		sampler->Release();
}

SG::D3D11SamplerData::D3D11SamplerData(D3D11SamplerData&& other) : sampler(other.sampler)
{
	other.sampler = nullptr;
}

SG::D3D11SamplerData& SG::D3D11SamplerData::operator=(D3D11SamplerData&& other)
{
	if (this != &other)
	{
		sampler = other.sampler;
		other.sampler = nullptr;
	}

	return *this;

	// ADD THE TWO SWAP FUNCTIONS TO ALL HANDLERS
	// NOW THAT FRAME MAPS ARE USED THEY ALL NEED THEM
	// PROBABLY SHOULD RENAME THEM TO BETTER NAMES THAT REFLECT WHAT IS ACTUALLY DONE
}

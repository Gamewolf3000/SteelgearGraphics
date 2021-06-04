#include "D3D11ShaderData.h"

SG::D3D11ShaderData::~D3D11ShaderData()
{
    if (shader.vertex != nullptr) // Since only ptrs it does not matter which is used here
    {
		switch (type)
		{
		case ShaderType::VERTEX_SHADER:
			shader.vertex->Release();
			break;
		case ShaderType::HULL_SHADER:
			shader.hull->Release();
			break;
		case ShaderType::DOMAIN_SHADER:
			shader.domain->Release();
			break;
		case ShaderType::GEOMETRY_SHADER:
			shader.geometry->Release();
			break;
		case ShaderType::PIXEL_SHADER:
			shader.pixel->Release();
			break;
		case ShaderType::COMPUTE_SHADER:
			shader.compute->Release();
			break;
		}
    }
}

SG::D3D11ShaderData::D3D11ShaderData(D3D11ShaderData&& other) : type(other.type), shader(other.shader)
{
	other.shader.vertex = nullptr; // Since only ptrs it does not matter which is used here
}

const SG::D3D11ShaderData& SG::D3D11ShaderData::operator=(D3D11ShaderData&& other)
{
	if (this != &other)
	{
		type = other.type;
		shader = other.shader;
		other.shader.vertex = nullptr; // Since only ptrs it does not matter which is used here
	}

	return *this;
}

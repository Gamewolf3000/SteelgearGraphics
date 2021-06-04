#pragma once

#include <d3d11_4.h>

namespace SG
{
	enum class ShaderType
	{
		VERTEX_SHADER,
		HULL_SHADER,
		DOMAIN_SHADER,
		GEOMETRY_SHADER,
		PIXEL_SHADER,
		COMPUTE_SHADER
	};

	struct D3D11ShaderData
	{
		ShaderType type;
		union
		{
			ID3D11VertexShader* vertex;
			ID3D11HullShader* hull;
			ID3D11DomainShader* domain;
			ID3D11GeometryShader* geometry;
			ID3D11PixelShader* pixel;
			ID3D11ComputeShader* compute;
		}shader;

		D3D11ShaderData() = default;
		~D3D11ShaderData();

		D3D11ShaderData(D3D11ShaderData&& other);
		const D3D11ShaderData& operator=(D3D11ShaderData&& other);
	};
}
#pragma once

#include <d3d11_4.h>
#include <vector>
#include <unordered_map>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"


namespace SG
{
	class D3D11ShaderManager
	{
	public:

		D3D11ShaderManager(ID3D11Device* device);
		~D3D11ShaderManager();

		SGResult CreateVertexShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateHullShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateDomainShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateGeometryShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreatePixelShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);

	private:
		enum class ShaderType
		{
			VERTEX_SHADER,
			HULL_SHADER,
			DOMAIN_SHADER,
			GEOMETRY_SHADER,
			PIXEL_SHADER
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
			}shader;
		};

		std::unordered_map<SGGuid, D3D11ShaderData> shaders;

		ID3D11Device* device;

	};
}
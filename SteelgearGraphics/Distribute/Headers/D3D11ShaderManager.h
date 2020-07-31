#pragma once

#include <d3d11_4.h>
#include <string>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"
#include "LockableUnorderedMap.h"
#include "D3D11CommonTypes.h"


namespace SG
{
	struct SGInputElement
	{
		std::string semanticName;
		UINT semanticIndex;
		DXGI_FORMAT format;
		UINT inputSlot;
		UINT alignedByteOffset;
		bool instancedData;
		UINT instanceDataStepRate;
	};

	class D3D11ShaderManager
	{
	public:

		D3D11ShaderManager(ID3D11Device* device);
		~D3D11ShaderManager();

		SGResult CreateInputLayout(const SGGuid& guid, const std::vector<SGInputElement>& inputElements, const void* shaderByteCode, UINT byteCodeLength);
		SGResult CreateVertexShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateHullShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateDomainShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateGeometryShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreatePixelShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);

	private:

		friend class D3D11RenderEngine;

		struct D3D11InputLayoutData
		{
			ID3D11InputLayout* inputLayout = nullptr;
		};

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

		LockableUnorderedMap<SGGuid, D3D11InputLayoutData> inputLayouts;
		LockableUnorderedMap<SGGuid, D3D11ShaderData> shaders;

		ID3D11Device* device;

		void SetInputLayout(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetVertexShader(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetPixelShader(const SGGuid& guid, ID3D11DeviceContext* context);
	};
}
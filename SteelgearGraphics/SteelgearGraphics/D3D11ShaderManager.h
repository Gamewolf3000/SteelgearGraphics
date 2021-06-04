#pragma once

#include <d3d11_4.h>
#include <string>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"
#include "FrameMap.h"

#include "D3D11CommonTypes.h"
#include "D3D11InputLayoutData.h"
#include "D3D11ShaderData.h"

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
		~D3D11ShaderManager() = default;

		SGResult CreateInputLayout(const SGGuid& guid, const std::vector<SGInputElement>& inputElements, const void* shaderByteCode, UINT byteCodeLength);
		void RemoveInputLayout(const SGGuid& guid);

		SGResult CreateVertexShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateHullShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateDomainShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateGeometryShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreatePixelShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		SGResult CreateComputeShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength);
		void RemoveShader(const SGGuid& guid);

	private:

		friend class D3D11RenderEngine;

		FrameMap<SGGuid, D3D11InputLayoutData> inputLayouts;
		FrameMap<SGGuid, D3D11ShaderData> shaders;

		ID3D11Device* device;

		void FinishFrame();
		void SwapFrame();

		void SetInputLayout(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetVertexShader(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetHullShader(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetDomainShader(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetGeometryShader(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetPixelShader(const SGGuid& guid, ID3D11DeviceContext* context);
		void SetComputeShader(const SGGuid& guid, ID3D11DeviceContext* context);

	};
}
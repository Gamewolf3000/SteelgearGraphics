#include "D3D11ShaderManager.h"

SG::D3D11ShaderManager::D3D11ShaderManager(ID3D11Device * device)
{
	this->device = device;
}

SG::SGResult SG::D3D11ShaderManager::CreateInputLayout(const SGGuid& guid, const std::vector<SGInputElement>& inputElements, const void* shaderByteCode, UINT byteCodeLength)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d11InputElements;
	d3d11InputElements.reserve(inputElements.size());

	for (auto& element : inputElements)
	{
		D3D11_INPUT_ELEMENT_DESC desc;
		desc.SemanticName = element.semanticName.c_str();
		desc.SemanticIndex = element.semanticIndex;
		desc.Format = element.format;
		desc.InputSlot = element.inputSlot;
		desc.AlignedByteOffset = element.alignedByteOffset;
		desc.InputSlotClass = element.instancedData ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = element.instanceDataStepRate;
		d3d11InputElements.push_back(desc);
	}

	D3D11InputLayoutData toStore;
	device->CreateInputLayout(&d3d11InputElements[0], static_cast<UINT>(d3d11InputElements.size()), shaderByteCode, byteCodeLength, &toStore.inputLayout);
	inputLayouts.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11ShaderManager::RemoveInputLayout(const SGGuid& guid)
{
	inputLayouts.RemoveElement(guid);
}

SG::SGResult SG::D3D11ShaderManager::CreateVertexShader(const SGGuid & guid, const void * shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11VertexShader* vs;

	if (FAILED(device->CreateVertexShader(shaderByteCode, byteCodeLength, nullptr, &vs)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::VERTEX_SHADER;
	toStore.shader.vertex = vs;
	shaders.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11ShaderManager::CreateHullShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11HullShader* hs;

	if (FAILED(device->CreateHullShader(shaderByteCode, byteCodeLength, nullptr, &hs)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::HULL_SHADER;
	toStore.shader.hull = hs;
	shaders.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11ShaderManager::CreateDomainShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11DomainShader* ds;

	if (FAILED(device->CreateDomainShader(shaderByteCode, byteCodeLength, nullptr, &ds)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::DOMAIN_SHADER;
	toStore.shader.domain = ds;
	shaders.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11ShaderManager::CreateGeometryShader(const SGGuid& guid, const void* shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11GeometryShader* gs;

	if (FAILED(device->CreateGeometryShader(shaderByteCode, byteCodeLength, nullptr, &gs)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::GEOMETRY_SHADER;
	toStore.shader.geometry = gs;
	shaders.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11ShaderManager::CreatePixelShader(const SGGuid & guid, const void * shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11PixelShader* ps;

	if (FAILED(device->CreatePixelShader(shaderByteCode, byteCodeLength, nullptr, &ps)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::PIXEL_SHADER;
	toStore.shader.pixel = ps;
	shaders.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

SG::SGResult SG::D3D11ShaderManager::CreateComputeShader(const SGGuid & guid, const void * shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11ComputeShader* cs;

	if (FAILED(device->CreateComputeShader(shaderByteCode, byteCodeLength, nullptr, &cs)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::COMPUTE_SHADER;
	toStore.shader.compute = cs;
	shaders.AddElement(guid, std::move(toStore));

	return SGResult::OK;
}

void SG::D3D11ShaderManager::RemoveShader(const SGGuid& guid)
{
	shaders.RemoveElement(guid);
}

void SG::D3D11ShaderManager::FinishFrame()
{
	inputLayouts.FinishFrame();
	shaders.FinishFrame();
}

void SG::D3D11ShaderManager::SwapFrame()
{
	inputLayouts.UpdateActive();
	shaders.UpdateActive();
}

void SG::D3D11ShaderManager::SetInputLayout(const SGGuid & guid, ID3D11DeviceContext * context)
{
	if constexpr (DEBUG_VERSION)
		if (!inputLayouts.HasElement(guid))
			throw std::runtime_error("Error setting input layout, guid not found");

	context->IASetInputLayout(inputLayouts[guid].inputLayout);
}

void SG::D3D11ShaderManager::SetVertexShader(const SGGuid & guid, ID3D11DeviceContext * context)
{
	if constexpr (DEBUG_VERSION)
		if (!shaders.HasElement(guid))
			throw std::runtime_error("Error setting vertex shader, guid not found");

	context->VSSetShader(shaders[guid].shader.vertex, nullptr, 0);
}

void SG::D3D11ShaderManager::SetHullShader(const SGGuid& guid, ID3D11DeviceContext* context)
{
	if constexpr (DEBUG_VERSION)
		if (!shaders.HasElement(guid))
			throw std::runtime_error("Error setting hull shader, guid not found");

	context->HSSetShader(shaders[guid].shader.hull, nullptr, 0);
}

void SG::D3D11ShaderManager::SetDomainShader(const SGGuid& guid, ID3D11DeviceContext* context)
{
	if constexpr (DEBUG_VERSION)
		if (!shaders.HasElement(guid))
			throw std::runtime_error("Error setting domain shader, guid not found");

	context->DSSetShader(shaders[guid].shader.domain, nullptr, 0);
}

void SG::D3D11ShaderManager::SetGeometryShader(const SGGuid& guid, ID3D11DeviceContext* context)
{
	if constexpr (DEBUG_VERSION)
		if (!shaders.HasElement(guid))
			throw std::runtime_error("Error setting geometry shader, guid not found");

	context->GSSetShader(shaders[guid].shader.geometry, nullptr, 0);
}

void SG::D3D11ShaderManager::SetPixelShader(const SGGuid & guid, ID3D11DeviceContext * context)
{
	if constexpr (DEBUG_VERSION)
		if (!shaders.HasElement(guid))
			throw std::runtime_error("Error setting pixel shader, guid not found");

	context->PSSetShader(shaders[guid].shader.pixel, nullptr, 0);
}

void SG::D3D11ShaderManager::SetComputeShader(const SGGuid & guid, ID3D11DeviceContext * context)
{
	if constexpr (DEBUG_VERSION)
		if (!shaders.HasElement(guid))
			throw std::runtime_error("Error setting compute shader, guid not found");

	context->CSSetShader(shaders[guid].shader.compute, nullptr, 0);
}

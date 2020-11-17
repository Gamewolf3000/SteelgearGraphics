#include "D3D11ShaderManager.h"

SG::D3D11ShaderManager::D3D11ShaderManager(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11ShaderManager::~D3D11ShaderManager()
{
	for (auto& shader : shaders)
	{
		switch (shader.second.type)
		{
		case ShaderType::VERTEX_SHADER:
			ReleaseCOM(shader.second.shader.vertex);
			break;
		case ShaderType::HULL_SHADER:
			ReleaseCOM(shader.second.shader.hull);
			break;
		case ShaderType::DOMAIN_SHADER:
			ReleaseCOM(shader.second.shader.domain);
			break;
		case ShaderType::GEOMETRY_SHADER:
			ReleaseCOM(shader.second.shader.geometry);
			break;
		case ShaderType::PIXEL_SHADER:
			ReleaseCOM(shader.second.shader.pixel);
			break;
		case ShaderType::COMPUTE_SHADER:
			ReleaseCOM(shader.second.shader.compute);
			break;
		}
	}

	for (auto& layout : inputLayouts)
		ReleaseCOM(layout.second.inputLayout);
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

	inputLayouts.lock();
	inputLayouts[guid] = toStore;
	inputLayouts.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11ShaderManager::CreateVertexShader(const SGGuid & guid, const void * shaderByteCode, SIZE_T byteCodeLength)
{
	ID3D11VertexShader* vs;

	if (FAILED(device->CreateVertexShader(shaderByteCode, byteCodeLength, nullptr, &vs)))
		return SGResult::FAIL;

	D3D11ShaderData toStore;
	toStore.type = ShaderType::VERTEX_SHADER;
	toStore.shader.vertex = vs;

	this->shaders.lock();
	this->shaders[guid] = toStore;
	this->shaders.unlock();

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

	this->shaders.lock();
	this->shaders[guid] = toStore;
	this->shaders.unlock();

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

	this->shaders.lock();
	this->shaders[guid] = toStore;
	this->shaders.unlock();

	return SGResult::OK;
}

void SG::D3D11ShaderManager::SetInputLayout(const SGGuid & guid, ID3D11DeviceContext * context)
{
	inputLayouts.lock();

	if constexpr (DEBUG_VERSION)
		if (inputLayouts.find(guid) == inputLayouts.end())
			throw std::runtime_error("Error setting input layout, guid not found");

	context->IASetInputLayout(inputLayouts[guid].inputLayout);

	inputLayouts.unlock();
}

void SG::D3D11ShaderManager::SetVertexShader(const SGGuid & guid, ID3D11DeviceContext * context)
{
	shaders.lock();

	if constexpr (DEBUG_VERSION)
		if (shaders.find(guid) == shaders.end())
			throw std::runtime_error("Error setting vertex shader, guid not found");

	context->VSSetShader(shaders[guid].shader.vertex, nullptr, 0);

	shaders.unlock();
}

void SG::D3D11ShaderManager::SetPixelShader(const SGGuid & guid, ID3D11DeviceContext * context)
{
	shaders.lock();

	if constexpr (DEBUG_VERSION)
		if (shaders.find(guid) == shaders.end())
			throw std::runtime_error("Error setting pixel shader, guid not found");

	context->PSSetShader(shaders[guid].shader.pixel, nullptr, 0);

	shaders.unlock();
}

void SG::D3D11ShaderManager::SetComputeShader(const SGGuid & guid, ID3D11DeviceContext * context)
{
	shaders.lock();

	if constexpr (DEBUG_VERSION)
		if (shaders.find(guid) == shaders.end())
			throw std::runtime_error("Error setting compute shader, guid not found");

	context->CSSetShader(shaders[guid].shader.compute, nullptr, 0);

	shaders.unlock();
}

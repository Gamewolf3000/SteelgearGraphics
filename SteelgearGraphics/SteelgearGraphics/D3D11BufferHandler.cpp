#include "D3D11BufferHandler.h"

SG::SGResult SG::D3D11BufferHandler::BindBufferToEntity(const SGGraphicalEntityID & entity, const SGGuid & bufferGuid, const SGGuid & bindGuid)
{
	entityData.lock();
	entityData[entity][bindGuid] = bufferGuid;
	entityData.unlock();

	if constexpr (DEBUG_VERSION)
	{
		buffers.lock();
		if (buffers.find(bufferGuid) == buffers.end())
		{
			buffers.unlock();
			return SGResult::GUID_MISSING;
		}
		buffers.unlock();
	}

	return SGResult::OK;
}

void SG::D3D11BufferHandler::UpdateBuffer(const SGGuid & guid, const UpdateStrategy& updateStrategy, void * data, UINT subresource)
{
	dataIndexMutex.lock();
	buffers.lock();
	UpdateData& temp = buffers[guid].UpdatedData[toUpdate];
	memcpy(temp.data, data, temp.size);
	temp.strategy = updateStrategy;
	temp.subresource = subresource;
	buffers.unlock();
	updatedBuffers[toUpdate].push_back(guid); // måste låsa dataIndexMutex pga detta, expansion kan resultera i problem
	dataIndexMutex.unlock();
}

void SG::D3D11BufferHandler::SwapUpdateBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	std::swap(toUpdate, toUseNext);
	updatedBuffers[toUpdate].clear();
}

void SG::D3D11BufferHandler::SwapToWorkWithBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	std::swap(toWorkWith, toUseNext);
}

void SG::D3D11BufferHandler::UpdateBuffers(ID3D11DeviceContext * context)
{
	//gå igenom alla som finns i updatedBuffers[toWorkWith], borde inte behöva låsa index då detta kommer från renderingstråden som också är den enda som kan ändra på det?
	for (auto& guid : updatedBuffers[toWorkWith])
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		buffers.lock();
		D3D11BufferData& bData = buffers[guid];
		buffers.unlock(); // Should be enough based on how an unordered_map works
		UpdateData& uData = bData.UpdatedData[toWorkWith];
		D3D11_MAP mapStrategy = (uData.strategy == UpdateStrategy::DISCARD ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE);
		context->Map(bData.buffer, uData.subresource, mapStrategy, 0, &mappedResource);
		memcpy(mappedResource.pData, uData.data, uData.size);
		context->Unmap(bData.buffer, uData.subresource);
	}
}

SG::D3D11BufferHandler::D3D11BufferHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::SGResult SG::D3D11BufferHandler::CreateVertexBuffer(const SGGuid & guid, UINT size, UINT nrOfVertices, bool dynamic, bool streamOut, void * data)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	if (streamOut)
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	else
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (dynamic && streamOut)
	{
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}
	else if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (streamOut)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}
	else
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA bufferData;
	bufferData.pSysMem = data;

	D3D11BufferData toStore;
	toStore.type = BufferType::VERTEX_BUFFER;
	toStore.specificData.vb.nrOfVertices = nrOfVertices;
	
	for (int i = 0; i < 3; ++i)
	{
		toStore.UpdatedData[i].size = size;
		toStore.UpdatedData[i].data = new char[size];
	}

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = toStore;
	buffers.unlock();


	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateIndexBuffer(const SGGuid & guid, UINT size, UINT nrOfIndices, bool dynamic, void * data)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA bufferData;
	bufferData.pSysMem = data;

	D3D11BufferData toStore;
	toStore.type = BufferType::INDEX_BUFFER;
	toStore.specificData.vb.nrOfVertices = nrOfIndices;

	for (int i = 0; i < 3; ++i)
	{
		toStore.UpdatedData[i].size = size;
		toStore.UpdatedData[i].data = new char[size];
	}

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = toStore;
	buffers.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateConstantBuffer(const SGGuid & guid, UINT size, bool dynamic, bool cpuUpdate, void * data)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (dynamic && cpuUpdate)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (dynamic && !cpuUpdate)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}
	else
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	D3D11_SUBRESOURCE_DATA bufferData;
	bufferData.pSysMem = data;

	D3D11BufferData toStore;
	toStore.type = BufferType::CONSTANT_BUFFER;

	for (int i = 0; i < 3; ++i)
	{
		toStore.UpdatedData[i].size = size;
		toStore.UpdatedData[i].data = new char[size];
	}

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = toStore;
	buffers.unlock();

	return SGResult::OK;
}

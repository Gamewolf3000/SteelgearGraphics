#include "D3D11BufferHandler.h"

SG::SGResult SG::D3D11BufferHandler::BindBufferToEntity(const SGGraphicalEntityID & entity, const SGGuid & bufferGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, bufferGuid, bindGuid);

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

SG::SGResult SG::D3D11BufferHandler::BindBufferToGroup(const SGGuid & group, const SGGuid & bufferGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, bufferGuid, bindGuid);

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

SG::SGResult SG::D3D11BufferHandler::BindOffsetToEntity(const SGGraphicalEntityID & entity, const SGGuid & offsetGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, offsetGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		bufferOffsets.lock();
		if (bufferOffsets.find(offsetGuid) == bufferOffsets.end())
		{
			bufferOffsets.unlock();
			return SGResult::GUID_MISSING;
		}
		bufferOffsets.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::BindOffsetToGroup(const SGGuid & group, const SGGuid & offsetGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, offsetGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		bufferOffsets.lock();
		if (bufferOffsets.find(offsetGuid) == bufferOffsets.end())
		{
			bufferOffsets.unlock();
			return SGResult::GUID_MISSING;
		}
		bufferOffsets.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::BindStrideToEntity(const SGGraphicalEntityID & entity, const SGGuid & strideGuid, const SGGuid & bindGuid)
{
	this->UpdateEntity(entity, strideGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		bufferStrides.lock();
		if (bufferStrides.find(strideGuid) == bufferStrides.end())
		{
			bufferStrides.unlock();
			return SGResult::GUID_MISSING;
		}
		bufferStrides.unlock();
	}

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::BindStrideToGroup(const SGGuid & group, const SGGuid & strideGuid, const SGGuid & bindGuid)
{
	UpdateGroup(group, strideGuid, bindGuid);

	if constexpr (DEBUG_VERSION)
	{
		bufferStrides.lock();
		if (bufferStrides.find(strideGuid) == bufferStrides.end())
		{
			bufferStrides.unlock();
			return SGResult::GUID_MISSING;
		}
		bufferStrides.unlock();
	}

	return SGResult::OK;
}

void SG::D3D11BufferHandler::UpdateBuffer(const SGGuid & guid, const UpdateStrategy& updateStrategy, void * data, UINT subresource)
{
	frameBufferMutex.lock();
	buffers.lock();
	UpdateData& temp = buffers[guid].updatedData.GetToUpdate();
	memcpy(temp.data, data, temp.size);
	temp.strategy = updateStrategy;
	temp.subresource = subresource;
	buffers[guid].updatedData.MarkAsUpdated();
	buffers.unlock();
	updatedFrameBuffer.push_back(guid); // måste låsa frameBufferMutex pga detta, expansion kan resultera i problem
	frameBufferMutex.unlock();
}

void SG::D3D11BufferHandler::SwapUpdateBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapUpdateBuffer();
	
	for (auto& guid : updatedFrameBuffer)
		buffers[guid].updatedData.SwitchUpdateBuffer();
	
	updatedTotalBuffer.insert(updatedTotalBuffer.end(), updatedFrameBuffer.begin(), updatedFrameBuffer.end());
	updatedFrameBuffer.clear();
}

void SG::D3D11BufferHandler::SwapToWorkWithBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapToWorkWithBuffer();

	for (auto& guid : updatedTotalBuffer)
		buffers[guid].updatedData.SwitchActiveBuffer();

	updatedTotalBuffer.clear();
}

void SG::D3D11BufferHandler::UpdateBufferGPU(D3D11BufferData & toUpdate, ID3D11DeviceContext * context)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	UpdateData& uData = toUpdate.updatedData.GetActive();
	D3D11_MAP mapStrategy = (uData.strategy == UpdateStrategy::DISCARD ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE);
	context->Map(toUpdate.buffer, uData.subresource, mapStrategy, 0, &mappedResource);
	memcpy(mappedResource.pData, uData.data, uData.size);
	context->Unmap(toUpdate.buffer, uData.subresource);
	toUpdate.updatedData.MarkAsNotUpdated();
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context)
{
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(guid) == buffers.end())
		{
			buffers.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
	}

	D3D11BufferData& bData = buffers[guid];

	if(bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	buffers.unlock();

	return bData.buffer;
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context, const SGGuid & groupGuid)
{
	groupData.lock();
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(groupData[groupGuid][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
	}

	D3D11BufferData& bData = buffers[groupData[groupGuid][guid].GetActive()];
	buffers.unlock();
	groupData.unlock();

	if (bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(entityData[entity][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
	}

	D3D11BufferData& bData = buffers[entityData[entity][guid].GetActive()];
	buffers.unlock();
	entityData.unlock();

	if (bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

UINT SG::D3D11BufferHandler::GetOffset(const SGGuid & guid)
{
	bufferOffsets.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (bufferOffsets.find(guid) == bufferOffsets.end())
		{
			bufferOffsets.unlock();
			throw std::runtime_error("Error, missing guid when fetching offset");
		}
	}

	UINT toReturn = bufferOffsets[guid];
	bufferOffsets.unlock();
	return toReturn;
}

UINT SG::D3D11BufferHandler::GetOffset(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	bufferOffsets.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (bufferOffsets.find(groupData[groupGuid][guid].GetActive()) == bufferOffsets.end())
		{
			bufferOffsets.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching offset");
		}
	}

	UINT toReturn = bufferOffsets[groupData[groupGuid][guid].GetActive()];
	bufferOffsets.unlock();
	groupData.unlock();
	return toReturn;
}

UINT SG::D3D11BufferHandler::GetOffset(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	bufferOffsets.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (bufferOffsets.find(entityData[entity][guid].GetActive()) == bufferOffsets.end())
		{
			bufferOffsets.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching offset");
		}
	}

	UINT toReturn = bufferOffsets[entityData[entity][guid].GetActive()];
	bufferOffsets.unlock();
	entityData.unlock();
	return toReturn;
}

UINT SG::D3D11BufferHandler::GetStride(const SGGuid & guid)
{
	bufferStrides.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (bufferStrides.find(guid) == bufferStrides.end())
		{
			bufferStrides.unlock();
			throw std::runtime_error("Error, missing guid when fetching stride");
		}
	}

	UINT toReturn = bufferStrides[guid];
	bufferStrides.unlock();
	return toReturn;
}

UINT SG::D3D11BufferHandler::GetStride(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	bufferStrides.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (bufferStrides.find(groupData[groupGuid][guid].GetActive()) == bufferStrides.end())
		{
			bufferStrides.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching stride");
		}
	}

	UINT toReturn = bufferStrides[groupData[groupGuid][guid].GetActive()];
	bufferStrides.unlock();
	groupData.unlock();
	return toReturn;
}

UINT SG::D3D11BufferHandler::GetStride(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	bufferStrides.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (bufferStrides.find(entityData[entity][guid].GetActive()) == bufferStrides.end())
		{
			bufferStrides.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
	}

	UINT toReturn = bufferStrides[entityData[entity][guid].GetActive()];
	bufferStrides.unlock();
	entityData.unlock();
	return toReturn;
}

UINT SG::D3D11BufferHandler::GetElementCount(const SGGuid & guid)
{
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(guid) == buffers.end())
		{
			buffers.unlock();
			throw std::runtime_error("Error, missing guid when fetching element count of buffer");
		}
	}

	D3D11BufferData& bData = buffers[guid];
	buffers.unlock();

	switch (bData.type)
	{
	case BufferType::VERTEX_BUFFER:
		return bData.specificData.vb.nrOfVertices;
	case BufferType::INDEX_BUFFER:
		return bData.specificData.ib.nrOfIndices;
	default:
		throw std::runtime_error("Error, attempting to fetch element count from a buffer that is not a vertex or index buffer");
	}
}

UINT SG::D3D11BufferHandler::GetElementCount(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(groupData[groupGuid][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching element count of buffer");
		}
	}

	D3D11BufferData& bData = buffers[groupData[groupGuid][guid].GetActive()];
	buffers.unlock();
	groupData.unlock();

	switch (bData.type)
	{
	case BufferType::VERTEX_BUFFER:
		return bData.specificData.vb.nrOfVertices;
	case BufferType::INDEX_BUFFER:
		return bData.specificData.ib.nrOfIndices;
	default:
		throw std::runtime_error("Error, attempting to fetch element count from a buffer that is not a vertex or index buffer");
	}
}

UINT SG::D3D11BufferHandler::GetElementCount(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(entityData[entity][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching element count of buffer");
		}
	}

	D3D11BufferData& bData = buffers[entityData[entity][guid].GetActive()];
	buffers.unlock();
	entityData.unlock();

	switch (bData.type)
	{
	case BufferType::VERTEX_BUFFER:
		return bData.specificData.vb.nrOfVertices;
	case BufferType::INDEX_BUFFER:
		return bData.specificData.ib.nrOfIndices;
	default:
		throw std::runtime_error("Error, attempting to fetch element count from a buffer that is not a vertex or index buffer");
	}
}

UINT SG::D3D11BufferHandler::GetVBElementSize(const SGGuid & guid)
{
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(guid) == buffers.end())
		{
			buffers.unlock();
			throw std::runtime_error("Error, missing guid when fetching size of vertice of a vertex buffer");
		}
	}

	D3D11BufferData& bData = buffers[guid];
	buffers.unlock();

	switch (bData.type)
	{
	case BufferType::VERTEX_BUFFER:
		return bData.specificData.vb.vertexSize;
	default:
		throw std::runtime_error("Error, attempting to fetch size of vertice from a buffer that is not a vertex buffer");
	}
}

UINT SG::D3D11BufferHandler::GetVBElementSize(const SGGuid & guid, const SGGuid & groupGuid)
{
	groupData.lock();
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(groupData[groupGuid][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching size of vertice of a vertex buffer");
		}
	}

	D3D11BufferData& bData = buffers[groupData[groupGuid][guid].GetActive()];
	buffers.unlock();
	groupData.unlock();

	switch (bData.type)
	{
	case BufferType::VERTEX_BUFFER:
		return bData.specificData.vb.vertexSize;
	default:
		throw std::runtime_error("Error, attempting to fetch size of vertice from a buffer that is not a vertex buffer");
	}
}

UINT SG::D3D11BufferHandler::GetVBElementSize(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	entityData.lock();
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(entityData[entity][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			entityData.unlock();
			throw std::runtime_error("Error, missing guid when fetching size of vertice of a vertex buffer");
		}
	}

	D3D11BufferData& bData = buffers[entityData[entity][guid].GetActive()];
	buffers.unlock();
	entityData.unlock();

	switch (bData.type)
	{
	case BufferType::VERTEX_BUFFER:
		return bData.specificData.vb.vertexSize;
	default:
		throw std::runtime_error("Error, attempting to fetch size of vertice from a buffer that is not a vertex buffer");
	}
}

SG::D3D11BufferHandler::D3D11BufferHandler(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11BufferHandler::~D3D11BufferHandler()
{
	for (auto& buffer : buffers)
		ReleaseCOM(buffer.second.buffer);

	for (auto& view : views)
	{
		switch (view.second.type)
		{
		case ResourceViewType::SRV:
			ReleaseCOM(view.second.view.srv);
			break;
		case ResourceViewType::UAV:
			ReleaseCOM(view.second.view.uav);
			break;
		default:
			break;
		}
	}
}

SG::SGResult SG::D3D11BufferHandler::CreateVertexBuffer(const SGGuid & guid, UINT size, UINT nrOfVertices, bool dynamic, bool streamOut, void * data)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
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
	ZeroMemory(&bufferData, sizeof(bufferData));
	bufferData.pSysMem = data;

	D3D11BufferData toStore;
	toStore.type = BufferType::VERTEX_BUFFER;
	toStore.specificData.vb.nrOfVertices = nrOfVertices;
	toStore.specificData.vb.vertexSize = size / nrOfVertices;
	toStore.updatedData = TripleBufferedData(UpdateData(size), UpdateData(size), UpdateData(size));

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = std::move(toStore);
	buffers.unlock();


	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateIndexBuffer(const SGGuid & guid, UINT size, UINT nrOfIndices, bool dynamic, void * data)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
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
	ZeroMemory(&bufferData, sizeof(bufferData));
	bufferData.pSysMem = data;

	D3D11BufferData toStore;
	toStore.type = BufferType::INDEX_BUFFER;
	toStore.specificData.vb.nrOfVertices = nrOfIndices;
	toStore.updatedData = TripleBufferedData(UpdateData(size), UpdateData(size), UpdateData(size));

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = std::move(toStore);
	buffers.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateConstantBuffer(const SGGuid & guid, UINT size, bool dynamic, bool cpuUpdate, void * data)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
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
	ZeroMemory(&bufferData, sizeof(bufferData));
	bufferData.pSysMem = data;

	D3D11BufferData toStore;
	toStore.type = BufferType::CONSTANT_BUFFER;
	toStore.updatedData = TripleBufferedData(UpdateData(size), UpdateData(size), UpdateData(size));

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = std::move(toStore);
	buffers.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateBufferOffset(const SGGuid & guid, UINT value)
{
	bufferOffsets.lock();
	bufferOffsets[guid] = value;
	bufferOffsets.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateBufferStride(const SGGuid & guid, UINT value)
{
	bufferStrides.lock();
	bufferStrides[guid] = value;
	bufferStrides.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateSRV(const SGGuid & guid, const SGGuid & bufferGuid, DXGI_FORMAT format, UINT elementOffset, UINT elementWidth)
{
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(bufferGuid) == buffers.end())
		{
			buffers.unlock();
			entityData.unlock();
			return SG::SGResult::GUID_MISSING;
		}
	}

	ID3D11Buffer* buffer = (*buffers.find(bufferGuid)).second.buffer;
	buffers.unlock();

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = format;
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	desc.Buffer.ElementOffset = elementOffset;
	desc.Buffer.ElementWidth = elementWidth;

	D3D11ResourceViewData toStore;
	if(FAILED(device->CreateShaderResourceView(buffer, &desc, &toStore.view.srv)))
		return SGResult::FAIL;

	toStore.resourceGuid = bufferGuid;
	toStore.type = SG::ResourceViewType::SRV;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SG::SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateUAV(const SGGuid & guid, const SGGuid & bufferGuid, DXGI_FORMAT format, UINT firstElement, UINT numberOfElements, bool counter, bool append, bool raw)
{
	buffers.lock();

	if constexpr (DEBUG_VERSION)
	{
		if (buffers.find(bufferGuid) == buffers.end())
		{
			buffers.unlock();
			entityData.unlock();
			return SG::SGResult::GUID_MISSING;
		}
	}

	ID3D11Buffer* buffer = (*buffers.find(bufferGuid)).second.buffer;
	buffers.unlock();

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = format;
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = firstElement;
	desc.Buffer.NumElements = numberOfElements;
	desc.Buffer.Flags = 0 | (counter * D3D11_BUFFER_UAV_FLAG_COUNTER) | (append * D3D11_BUFFER_UAV_FLAG_APPEND) | (raw * D3D11_BUFFER_UAV_FLAG_RAW);

	D3D11ResourceViewData toStore;
	if (FAILED(device->CreateUnorderedAccessView(buffer, &desc, &toStore.view.uav)))
		return SGResult::FAIL;

	toStore.resourceGuid = bufferGuid;
	toStore.type = SG::ResourceViewType::UAV;
	views.lock();
	views[guid] = toStore;
	views.unlock();

	return SG::SGResult::OK;
}

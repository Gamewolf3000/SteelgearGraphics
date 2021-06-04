#include "D3D11BufferHandler.h"

SG::SGResult SG::D3D11BufferHandler::BindBufferToEntity(const SGGraphicalEntityID & entity, const SGGuid & bufferGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, bufferGuid, bindGuid, buffers);
}

SG::SGResult SG::D3D11BufferHandler::BindBufferToGroup(const SGGuid & group, const SGGuid & bufferGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, bufferGuid, bindGuid, buffers);
}

SG::SGResult SG::D3D11BufferHandler::BindOffsetToEntity(const SGGraphicalEntityID & entity, const SGGuid & offsetGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, offsetGuid, bindGuid, bufferOffsets);
}

SG::SGResult SG::D3D11BufferHandler::BindOffsetToGroup(const SGGuid & group, const SGGuid & offsetGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, offsetGuid, bindGuid, bufferOffsets);
}

SG::SGResult SG::D3D11BufferHandler::BindStrideToEntity(const SGGraphicalEntityID & entity, const SGGuid & strideGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, strideGuid, bindGuid, bufferStrides);
}

SG::SGResult SG::D3D11BufferHandler::BindStrideToGroup(const SGGuid & group, const SGGuid & strideGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, strideGuid, bindGuid, bufferStrides);
}

SG::SGResult SG::D3D11BufferHandler::BindViewToEntity(const SGGraphicalEntityID & entity, const SGGuid & viewGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToEntity(entity, viewGuid, bindGuid, views);
}

SG::SGResult SG::D3D11BufferHandler::BindViewToGroup(const SGGuid & group, const SGGuid & viewGuid, const SGGuid & bindGuid)
{
	return SG::SGGraphicsHandler::BindElementToGroup(group, viewGuid, bindGuid, views);
}

void SG::D3D11BufferHandler::UpdateBuffer(const SGGuid & guid, const UpdateStrategy& updateStrategy, void * data, UINT subresource)
{
	D3D11BufferData& temp = buffers.GetElement(guid);
	auto& ToUpdate = temp.updatedData.GetToUpdate();
	memcpy(ToUpdate.data, data, ToUpdate.size);
	ToUpdate.strategy = updateStrategy;
	ToUpdate.subresource = subresource;
	temp.updatedData.MarkAsUpdated();

	frameBufferMutex.lock();
	updatedFrameBuffer.push_back(guid); // måste låsa frameBufferMutex pga detta, expansion kan resultera i problem
	frameBufferMutex.unlock();
}

void SG::D3D11BufferHandler::FinishFrame()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::FinishFrame();

	buffers.FinishFrame();
	views.FinishFrame();
	bufferOffsets.FinishFrame();
	bufferStrides.FinishFrame();

	for (auto& guid : updatedFrameBuffer)
		buffers.GetElement(guid).updatedData.SwitchUpdateBuffer();
	
	updatedTotalBuffer.insert(updatedTotalBuffer.end(), updatedFrameBuffer.begin(), updatedFrameBuffer.end());
	updatedFrameBuffer.clear();
}

void SG::D3D11BufferHandler::SwapFrame()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapFrame();

	buffers.UpdateActive();
	views.UpdateActive();
	bufferOffsets.UpdateActive();
	bufferStrides.UpdateActive();

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
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetGlobalElement(guid, buffers, "buffer");

	if(bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context, const SGGuid & groupGuid)
{
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, buffers, "buffer");

	if (bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context, const SGGraphicalEntityID & entity)
{
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetEntityElement(guid, entity, buffers, "buffer");

	if (bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

ID3D11ShaderResourceView * SG::D3D11BufferHandler::GetSRV(const SGGuid & guid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGlobalResourceView(guid, ResourceViewType::SRV, views, "buffer");
	return static_cast<ID3D11ShaderResourceView *>(toReturn);
}

ID3D11ShaderResourceView * SG::D3D11BufferHandler::GetSRV(const SGGuid & guid, const SGGuid & groupGuid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGroupResourceView(guid, groupGuid, ResourceViewType::SRV, views, "buffer");
	return static_cast<ID3D11ShaderResourceView*>(toReturn);
}

ID3D11ShaderResourceView * SG::D3D11BufferHandler::GetSRV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetEntityResourceView(guid, entity, ResourceViewType::SRV, views, "buffer");
	return static_cast<ID3D11ShaderResourceView*>(toReturn);
}

ID3D11UnorderedAccessView * SG::D3D11BufferHandler::GetUAV(const SGGuid & guid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGlobalResourceView(guid, ResourceViewType::UAV, views, "buffer");
	return static_cast<ID3D11UnorderedAccessView*>(toReturn);
}

ID3D11UnorderedAccessView * SG::D3D11BufferHandler::GetUAV(const SGGuid & guid, const SGGuid & groupGuid)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetGroupResourceView(guid, groupGuid, ResourceViewType::UAV, views, "buffer");
	return static_cast<ID3D11UnorderedAccessView*>(toReturn);
}

ID3D11UnorderedAccessView * SG::D3D11BufferHandler::GetUAV(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	void* toReturn = SG::D3D11GraphicsHandler::GetEntityResourceView(guid, entity, ResourceViewType::UAV, views, "buffer");
	return static_cast<ID3D11UnorderedAccessView*>(toReturn);
}

UINT SG::D3D11BufferHandler::GetOffset(const SGGuid & guid)
{
	return SG::SGGraphicsHandler::GetGlobalElement(guid, bufferOffsets, "offset");
}

UINT SG::D3D11BufferHandler::GetOffset(const SGGuid & guid, const SGGuid & groupGuid)
{
	return SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, bufferOffsets, "offset");
}

UINT SG::D3D11BufferHandler::GetOffset(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	return SG::SGGraphicsHandler::GetEntityElement(guid, entity, bufferOffsets, "offset");
}

UINT SG::D3D11BufferHandler::GetStride(const SGGuid & guid)
{
	return SG::SGGraphicsHandler::GetGlobalElement(guid, bufferStrides, "stride");
}

UINT SG::D3D11BufferHandler::GetStride(const SGGuid & guid, const SGGuid & groupGuid)
{
	return SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, bufferStrides, "stride");
}

UINT SG::D3D11BufferHandler::GetStride(const SGGuid & guid, const SGGraphicalEntityID & entity)
{
	return SG::SGGraphicsHandler::GetEntityElement(guid, entity, bufferStrides, "stride");
}

UINT SG::D3D11BufferHandler::GetElementCount(const SGGuid & guid)
{
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetGlobalElement(guid, buffers, "element count of buffer");

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
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, buffers, "element count of buffer");

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
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetEntityElement(guid, entity, buffers, "element count of buffer");

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
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetGlobalElement(guid, buffers, "size of vertice of a vertex buffer");

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
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetGroupElement(guid, groupGuid, buffers, "size of vertice of a vertex buffer");

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
	D3D11BufferData& bData = SG::SGGraphicsHandler::GetEntityElement(guid, entity, buffers, "size of vertice of a vertex buffer");

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

SG::SGResult SG::D3D11BufferHandler::CreateVertexBuffer(const SGGuid & guid, UINT size, UINT nrOfVertices,
	bool dynamic, bool streamOut, const void* const data)
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

	buffers.AddElement(guid, std::move(toStore));
	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateIndexBuffer(const SGGuid & guid, UINT size, UINT nrOfIndices, bool dynamic, const void* const data)
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

	buffers.AddElement(guid, std::move(toStore));
	return SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateConstantBuffer(const SGGuid & guid, UINT size, bool dynamic, bool cpuUpdate, const void* const data)
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

	buffers.AddElement(guid, std::move(toStore));
	return SGResult::OK;
}

void SG::D3D11BufferHandler::RemoveBuffer(const SGGuid& guid)
{
	buffers.RemoveElement(guid);
}

SG::SGResult SG::D3D11BufferHandler::CreateBufferOffset(const SGGuid & guid, UINT value)
{
	bufferOffsets.AddElement(guid, std::move(value));
	return SGResult::OK;
}

void SG::D3D11BufferHandler::RemoveBufferOffset(const SGGuid& guid)
{
	bufferOffsets.RemoveElement(guid);
}

SG::SGResult SG::D3D11BufferHandler::CreateBufferStride(const SGGuid & guid, UINT value)
{
	bufferStrides.AddElement(guid, std::move(value));
	return SGResult::OK;
}

void SG::D3D11BufferHandler::RemoveBufferStride(const SGGuid& guid)
{
	bufferStrides.RemoveElement(guid);
}

SG::SGResult SG::D3D11BufferHandler::CreateSRV(const SGGuid & guid, const SGGuid & bufferGuid, DXGI_FORMAT format, UINT elementOffset, UINT elementWidth)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!buffers.Exists(bufferGuid))
			return SG::SGResult::GUID_MISSING;
	}

	ID3D11Buffer* buffer = buffers.GetElement(bufferGuid).buffer;

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
	views.AddElement(guid, std::move(toStore));

	return SG::SGResult::OK;
}

SG::SGResult SG::D3D11BufferHandler::CreateUAV(const SGGuid & guid, const SGGuid & bufferGuid, DXGI_FORMAT format, UINT firstElement, UINT numberOfElements, bool counter, bool append, bool raw)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!buffers.Exists(bufferGuid))
			return SG::SGResult::GUID_MISSING;
	}

	ID3D11Buffer* buffer = buffers.GetElement(bufferGuid).buffer;

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
	views.AddElement(guid, std::move(toStore));

	return SG::SGResult::OK;
}

void SG::D3D11BufferHandler::RemoveView(const SGGuid& guid)
{
	views.RemoveElement(guid);
}

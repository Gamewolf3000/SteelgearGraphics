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
	groupData.lock();
	groupData[group][bindGuid] = bufferGuid;
	groupData.unlock();

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
}

void SG::D3D11BufferHandler::SwapToWorkWithBuffer()
{
	// No need to lock since this function is called only by the render engine during certain conditions
	SGGraphicsHandler::SwapToWorkWithBuffer();

	for (auto& guid : updatedTotalBuffer)
		buffers[guid].updatedData.SwitchActiveBuffer();

	updatedTotalBuffer.clear();
}

void SG::D3D11BufferHandler::UpdateBuffers(ID3D11DeviceContext * context)
{
	//DEPRECATED FUNCTION, REPLACE WITH A FUNCTION THAT UPDATES A SINGLE BUFFER TO BE CALLED WHEN A BUFFER THAT IS TO BE USED IS DISCOVERED TO HAVE BEEN UPDATED
	(void)context;
	//gå igenom alla som finns i updatedBuffers[toWorkWith], borde inte behöva låsa index då detta kommer från renderingstråden som också är den enda som kan ändra på det?
	//for (auto& guid : updatedBuffers[toWorkWith])
	//{
	//	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//	buffers.lock();
	//	D3D11BufferData& bData = buffers[guid];
	//	buffers.unlock(); // Should be enough based on how an unordered_map works
	//	UpdateData& uData = bData.UpdatedData[toWorkWith];
	//	D3D11_MAP mapStrategy = (uData.strategy == UpdateStrategy::DISCARD ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE);
	//	context->Map(bData.buffer, uData.subresource, mapStrategy, 0, &mappedResource);
	//	memcpy(mappedResource.pData, uData.data, uData.size);
	//	context->Unmap(bData.buffer, uData.subresource);
	//}
}

void SG::D3D11BufferHandler::SetConstantBuffers(const std::vector<PipelineComponent>& vs, const std::vector<PipelineComponent>& hs, const std::vector<PipelineComponent>& ds, const std::vector<PipelineComponent>& gs, const std::vector<PipelineComponent>& ps, ID3D11DeviceContext * context, const SGGuid& groupGuid, const SGGraphicalEntityID& entity)
{
	const unsigned int arrSize = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
	ID3D11Buffer * bufferArr[arrSize];

	FillBufferArray(vs, bufferArr, arrSize, context, groupGuid, entity);
	context->VSSetConstantBuffers(0, arrSize, bufferArr);

	FillBufferArray(hs, bufferArr, arrSize, context, groupGuid, entity);
	context->HSSetConstantBuffers(0, arrSize, bufferArr);

	FillBufferArray(ds, bufferArr, arrSize, context, groupGuid, entity);
	context->DSSetConstantBuffers(0, arrSize, bufferArr);

	FillBufferArray(gs, bufferArr, arrSize, context, groupGuid, entity);
	context->GSSetConstantBuffers(0, arrSize, bufferArr);

	FillBufferArray(ps, bufferArr, arrSize, context, groupGuid, entity);
	context->PSSetConstantBuffers(0, arrSize, bufferArr);
}

void SG::D3D11BufferHandler::FillBufferArray(const std::vector<PipelineComponent>& resources, ID3D11Buffer ** bufferArr, unsigned int arrSize, ID3D11DeviceContext* context, const SGGuid & groupGuid, const SGGraphicalEntityID & entity)
{
	for (unsigned int i = 0; i < arrSize; ++i)
	{
		if (resources.size() > i)
		{
			const SGGuid* bufferGuid;
			entityData.lock();
			switch (resources[i].source)
			{
			case Association::ENTITY:
				bufferGuid = &entityData[entity][resources[i].resourceGuid].GetActive();
				break;
			case Association::GROUP:
				bufferGuid = &groupData[groupGuid][resources[i].resourceGuid].GetActive();
				break;
			case Association::GLOBAL:
				bufferGuid = &resources[i].resourceGuid;
				break;
			}
			entityData.unlock();

			buffers.lock();
			D3D11BufferData& buffer = buffers[*bufferGuid];
			buffers.unlock();

			if (buffer.updatedData.Updated())
				UpdateBufferGPU(buffer, context);

			bufferArr[i] = buffer.buffer;
		}
		else
			bufferArr[i] = nullptr;
	}
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
	if constexpr (DEBUG_VERSION)
	{
		buffers.lock();
		if (buffers.find(guid) == buffers.end())
		{
			buffers.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
		buffers.unlock();
	}

	buffers.lock();
	D3D11BufferData& bData = buffers[guid];
	buffers.unlock();

	if(bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context, const SGGuid & groupGuid)
{
	if constexpr (DEBUG_VERSION)
	{
		groupData.lock();
		buffers.lock();
		if (buffers.find(groupData[groupGuid][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
		buffers.unlock();
		groupData.unlock();
	}

	groupData.lock();
	buffers.lock();
	D3D11BufferData& bData = buffers[groupData[groupGuid][guid].GetActive()];
	buffers.unlock();
	groupData.unlock();

	if (bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

ID3D11Buffer * SG::D3D11BufferHandler::GetBuffer(const SGGuid & guid, ID3D11DeviceContext * context, const SGGraphicalEntityID & entity)
{
	if constexpr (DEBUG_VERSION)
	{
		entityData.lock();
		buffers.lock();
		if (buffers.find(entityData[entity][guid].GetActive()) == buffers.end())
		{
			buffers.unlock();
			groupData.unlock();
			throw std::runtime_error("Error, missing guid when fetching buffer");
		}
		buffers.unlock();
		entityData.unlock();
	}

	groupData.lock();
	buffers.lock();
	D3D11BufferData& bData = buffers[entityData[entity][guid].GetActive()];
	buffers.unlock();
	groupData.unlock();

	if (bData.updatedData.Updated())
		UpdateBufferGPU(bData, context);

	return bData.buffer;
}

// Gör så att renderengine hämtar buffrar från bufferhandler (undantag kan nog göras för cb)
// Lägg till enum för vilken vbuffer som draw call ska hämta data från
// Lägg till topology i pipelinen
// Fortsätt på render

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
	toStore.updatedData = TripleBufferedData(UpdateData(size), UpdateData(size), UpdateData(size));

	if (FAILED(device->CreateBuffer(&desc, &bufferData, &toStore.buffer)))
		return SGResult::FAIL;

	buffers.lock();
	buffers[guid] = std::move(toStore);
	buffers.unlock();

	return SGResult::OK;
}

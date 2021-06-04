#include "D3D11BufferData.h"

SG::D3D11BufferData::~D3D11BufferData()
{
	if (buffer != nullptr)
		buffer->Release();
}

SG::D3D11BufferData::D3D11BufferData(D3D11BufferData&& other) : type(other.type), updatedData(std::move(other.updatedData))
{
	if (type == BufferType::VERTEX_BUFFER)
		specificData.vb = other.specificData.vb;
	else
		specificData.ib = other.specificData.ib;

	buffer = other.buffer;
	other.buffer = nullptr;
}

const SG::D3D11BufferData& SG::D3D11BufferData::operator=(D3D11BufferData&& other)
{
	if (this != &other)
	{
		type = other.type;

		if (type == BufferType::VERTEX_BUFFER)
			specificData.vb = other.specificData.vb;
		else
			specificData.ib = other.specificData.ib;

		buffer = other.buffer;
		other.buffer = nullptr;

		updatedData = std::move(other.updatedData);
	}

	return *this;
}

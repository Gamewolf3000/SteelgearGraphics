#pragma once

#include <mutex>
#include <unordered_map>

#include <d3d11_4.h>
#include <dxgi1_6.h>

#include "SGGuid.h"
#include "D3D11ResourceViewData.h"

namespace SG
{
	enum class Association
	{
		GLOBAL,
		GROUP,
		ENTITY
	};

	struct PipelineComponent
	{
		Association source;
		SGGuid resourceGuid;
	};

	enum class UpdateStrategy
	{
		DISCARD,
		NO_OVERWRITE
	};

	struct UpdateData
	{
		void* data  = nullptr;
		size_t size = 0;
		UpdateStrategy strategy;
		UINT subresource;

		UpdateData() = default;

		UpdateData(size_t dataSize)
		{
			size = dataSize;
			data = new char[size];
		}

		//UpdateData(const UpdateData& other)
		//{
		//	this->size = other.size;
		//	this->data = new char[this->size];
		//}

		UpdateData(UpdateData&& other)
		{
			this->size = other.size;
			this->data = other.data;
			other.data = nullptr;
			other.size = 0;
		}

		//const UpdateData& operator=(const UpdateData& other)
		//{
		//	if (this != &other)
		//	{
		//		delete this->data;
		//		this->size = other.size;
		//		this->data = other.data;
		//	}

		//	return *this;
		//}

		const UpdateData& operator=(UpdateData&& other)
		{
			if (this != &other)
			{
				delete this->data;
				this->size = other.size;
				this->data = other.data;
				other.data = nullptr;
				other.size = 0;
			}

			return *this;
		}
		
		~UpdateData()
		{
			delete data;
		}
	};

	enum class ComparisonFunction
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS
	};

	template<class T>
	void ReleaseCOM(T* &comObject)
	{

		if (comObject != nullptr)
		{
			comObject->Release();
			comObject = nullptr;
		}
	}

	//https://stackoverflow.com/questions/40339138/convert-dxgi-format-to-a-bpp
	int GetFormatElementSize(DXGI_FORMAT format);

}
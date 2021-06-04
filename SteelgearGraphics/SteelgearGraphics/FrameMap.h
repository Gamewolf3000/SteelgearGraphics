#pragma once

#include "SGGuid.h"

#include <variant>
#include <unordered_map>
#include <vector>
#include <utility>
#include <mutex>
#include <algorithm>

namespace SG
{
	template<typename Key, typename StoredType>
	class FrameMap
	{
	private:
		enum class OperationType
		{
			ADD,
			REMOVE
		};

		struct StoredOperation
		{
			OperationType type;
			std::variant<std::pair<Key, StoredType>, Key> data;
		};

		std::unordered_map<Key, StoredType> activeElements;
		std::vector<StoredOperation> storedOperations;
		typename std::vector<Key>::size_type nrToUpdate = 0;
		std::mutex updateMutex;

	public:
		FrameMap() = default;
		~FrameMap() = default;

		FrameMap(const FrameMap<Key, StoredType>& other) = delete;
		FrameMap<Key, StoredType>& operator=(const FrameMap<Key, StoredType>& other) = delete;

		FrameMap(FrameMap<Key, StoredType>&& other) = default;
		FrameMap<Key, StoredType>& operator=(FrameMap<Key, StoredType>&& other) = default;

		void LockUpdate();
		void UnlockUpdate();

		std::unordered_map<Key, StoredType>& Elements();
		StoredType& GetElement(const Key& key);
		bool HasElement(const Key& key);
		bool Exists(const Key& key);
		StoredType& operator[](const Key& key);

		void AddElement(const Key& elementKey, const StoredType& element);
		void AddElement(const Key& elementKey, StoredType&& element);
		void RemoveElement(const Key& elementKey);

		void FinishFrame();
		void UpdateActive();
	};

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::LockUpdate()
	{
		updateMutex.lock();
	}

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::UnlockUpdate()
	{
		updateMutex.unlock();
	}

	template<typename Key, typename StoredType>
	inline std::unordered_map<Key, StoredType>& FrameMap<Key, StoredType>::Elements()
	{
		return activeElements;
	}

	template<typename Key, typename StoredType>
	inline StoredType& FrameMap<Key, StoredType>::GetElement(const Key& key)
	{
		updateMutex.lock();
		auto elementIterator = activeElements.find(key);

		if (elementIterator == activeElements.end())
		{
			for (auto& operation : storedOperations)
			{
				if (operation.type == OperationType::ADD)
				{
					auto& data = std::get<std::pair<Key, StoredType>>(operation.data);

					if (data.first == key)
					{
						updateMutex.unlock();
						return data.second;
					}
				}
			}
			updateMutex.unlock();
			throw std::runtime_error("Error, cannot find element with that key");
		}
		else
		{
			updateMutex.unlock();
			return elementIterator->second;
		}
	}

	template<typename Key, typename StoredType>
	inline bool FrameMap<Key, StoredType>::HasElement(const Key& key)
	{
		return activeElements.find(key) != activeElements.end();
	}

	template<typename Key, typename StoredType>
	inline bool FrameMap<Key, StoredType>::Exists(const Key& key)
	{
		updateMutex.lock();
		bool toReturn = HasElement(key);

		if (toReturn == false)
		{
			for (auto& storedOperation : storedOperations)
			{
				switch (storedOperation.type)
				{
				case OperationType::ADD:
				{
					std::pair<Key, StoredType>& add = std::get<std::pair<Key, StoredType>>(storedOperation.data);

					if (add.first == key)
						toReturn = true;

					break;
				}
				case OperationType::REMOVE:
				{
					Key& remove = std::get<Key>(storedOperation.data);

					if (remove == key)
						toReturn = true;

					break;

				}
				default:
					break;
				}
			}
		}
		updateMutex.unlock();

		return toReturn;
	}

	template<typename Key, typename StoredType>
	inline StoredType& FrameMap<Key, StoredType>::operator[](const Key& key)
	{
		return activeElements[key];
	}

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::AddElement(const Key& elementKey, const StoredType& element)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::ADD;
		temp.data = std::make_pair(elementKey, element);
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::AddElement(const Key& elementKey, StoredType&& element)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::ADD;
		temp.data = std::make_pair(elementKey, std::move(element));
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::RemoveElement(const Key& elementKey)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::REMOVE;
		temp.data = elementKey;
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::FinishFrame()
	{
		nrToUpdate = storedOperations.size();
	}

	template<typename Key, typename StoredType>
	inline void FrameMap<Key, StoredType>::UpdateActive()
	{
		updateMutex.lock();

		for (decltype(nrToUpdate) i = 0; i < nrToUpdate; ++i)
		{
			switch (storedOperations[i].type)
			{
			case OperationType::ADD:
			{
				std::pair<Key, StoredType>& add = std::get<std::pair<Key, StoredType>>(storedOperations[i].data);
				activeElements[add.first] = std::move(add.second);
				break;
			}
			case OperationType::REMOVE:
			{
				Key& remove = std::get<Key>(storedOperations[i].data);
				activeElements.erase(remove);
				break;
			}
			default:
				break;
			}
		}

		storedOperations.erase(storedOperations.begin(), storedOperations.begin() + nrToUpdate);
		nrToUpdate = 0;
		updateMutex.unlock();
	}
}
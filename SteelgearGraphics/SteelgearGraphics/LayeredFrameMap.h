#pragma once

#include "FrameMap.h"

namespace SG
{
	template<typename OuterKey, typename InnerKey, typename StoredType>
	class LayeredFrameMap
	{
	private:
		enum class OperationType
		{
			ADD_OUTER,
			ADD_INNER,
			REMOVE_OUTER,
			REMOVE_INNER
		};

		struct InnerElementData
		{
			OuterKey outerKey;
			InnerKey innerKey;
			StoredType toStore;
		};

		struct StoredOperation
		{
			OperationType type;
			std::variant<OuterKey, InnerElementData, std::pair<OuterKey, InnerKey>> data;
		};

		std::unordered_map<OuterKey, FrameMap<InnerKey, StoredType>> activeElements;
		std::vector<StoredOperation> storedOperations;
		typename std::vector<OuterKey>::size_type nrToUpdate = 0;

		std::mutex updateMutex;

	public:

		LayeredFrameMap() = default;
		~LayeredFrameMap() = default;

		void LockUpdate();
		void UnlockUpdate();

		std::unordered_map<OuterKey, FrameMap<InnerKey, StoredType>>& Elements();
		StoredType& GetElement(const OuterKey& outerKey, const InnerKey& innerKey);
		bool HasElement(const OuterKey& key);
		FrameMap<InnerKey, StoredType>& operator[](const OuterKey& key);

		void AddElement(const OuterKey& outerKey, const InnerKey& innerKey, StoredType&& element);
		void AddElement(const OuterKey& outerKey);
		void RemoveElement(const OuterKey& outerKey, const InnerKey& innerKey);
		void RemoveElement(const OuterKey& outerKey);

		void FinishFrame();
		void UpdateActive();
	};

	
	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::LockUpdate()
	{
		updateMutex.lock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::UnlockUpdate()
	{
		updateMutex.unlock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline std::unordered_map<OuterKey, FrameMap<InnerKey, StoredType>>& LayeredFrameMap<OuterKey, InnerKey, StoredType>::Elements()
	{
		return activeElements;
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline StoredType& LayeredFrameMap<OuterKey, InnerKey, StoredType>::GetElement(const OuterKey& outerKey, const InnerKey& innerKey)
	{
		auto elementIterator = activeElements.find(outerKey);
		if (elementIterator == activeElements.end())
		{
			updateMutex.lock();
			for (auto& operation : storedOperations)
			{
				if (operation.type == OperationType::ADD_INNER)
				{
					InnerElementData& data = std::get<InnerElementData>(operation.data);

					if (data.outerKey == outerKey && data.innerKey == innerKey)
					{
						updateMutex.unlock();
						return data.toStore;
					}
				}
			}
			updateMutex.unlock();
			throw std::runtime_error("Error, cannot find element with that key");
		}
		else
		{
			return elementIterator->second.GetElement(innerKey);
		}
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline bool LayeredFrameMap<OuterKey, InnerKey, StoredType>::HasElement(const OuterKey& key)
	{
		return activeElements.find(key) != activeElements.end();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline FrameMap<InnerKey, StoredType>& LayeredFrameMap<OuterKey, InnerKey, StoredType>::operator[](const OuterKey& key)
	{
		return activeElements[key];
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::AddElement(const OuterKey& outerKey, const InnerKey& innerKey, StoredType&& element)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::ADD_INNER;
		InnerElementData elementData;
		elementData.outerKey = outerKey;
		elementData.innerKey = innerKey;
		elementData.toStore = std::move(element);
		temp.data = std::move(elementData);
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::AddElement(const OuterKey& outerKey)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::ADD_OUTER;
		temp.data = outerKey;
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::RemoveElement(const OuterKey& outerKey, const InnerKey& innerKey)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::REMOVE_INNER;
		temp.data = std::move(std::make_pair(outerKey, innerKey));
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::RemoveElement(const OuterKey& outerKey)
	{
		updateMutex.lock();
		StoredOperation temp;
		temp.type = OperationType::ADD_INNER;
		temp.data = outerKey;
		storedOperations.push_back(std::move(temp));
		updateMutex.unlock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::FinishFrame()
	{
		//updateMutex.lock();
		nrToUpdate = storedOperations.size();
		//updateMutex.unlock();
	}

	template<typename OuterKey, typename InnerKey, typename StoredType>
	inline void LayeredFrameMap<OuterKey, InnerKey, StoredType>::UpdateActive()
	{
		updateMutex.lock();

		for (decltype(nrToUpdate) i = 0; i < nrToUpdate; ++i)
		{
			switch (storedOperations[i].type)
			{
			case OperationType::ADD_INNER:
			{
				InnerElementData& add_inner = std::get<InnerElementData>(storedOperations[i].data);
				activeElements[add_inner.outerKey].AddElement(add_inner.innerKey, std::move(add_inner.toStore));
				activeElements[add_inner.outerKey].FinishFrame();
				activeElements[add_inner.outerKey].UpdateActive();
				break;
			}
			case OperationType::ADD_OUTER:
			{
				OuterKey& add_outer = std::get<OuterKey>(storedOperations[i].data);
				activeElements[add_outer]; // Think this should force default construction if not already existing, operator= refuses to use move and only refuses to work...
				break;
			}
			case OperationType::REMOVE_INNER:
			{
				std::pair<OuterKey, InnerKey>& remove_inner = std::get<std::pair<OuterKey, InnerKey>>(storedOperations[i].data);
				activeElements[remove_inner.first].RemoveElement(remove_inner.second);
				activeElements[remove_inner.first].FinishFrame();
				activeElements[remove_inner.first].UpdateActive();
				break;
			}
			case OperationType::REMOVE_OUTER:
			{
				OuterKey& remove_outer = std::get<OuterKey>(storedOperations[i].data);
				activeElements.erase(remove_outer);
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
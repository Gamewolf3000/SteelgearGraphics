#pragma once

#include <string>
#include "LockableUnorderedMap.h"

namespace SG
{
	class SGGuid
	{
	private:


		static LockableUnorderedMap<std::string, size_t> guids;
		size_t myID;

	public:
		SGGuid();
		SGGuid(const std::string& identifier);

		~SGGuid() = default;

		bool operator==(const SGGuid& other) const;
		bool operator!=(const SGGuid& other) const;

		size_t GetID() const;
	};

}

namespace std
{
	template<>
	struct hash<SG::SGGuid>
	{
		size_t operator()(const SG::SGGuid & obj) const
		{
			return hash<size_t>()(obj.GetID());
		}
	};
}
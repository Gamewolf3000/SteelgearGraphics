#pragma once

#include <string>
#include <unordered_map>

namespace SG
{
	class SGGuid
	{
	private:


		static std::unordered_map<std::string, size_t> guids;
		size_t myID;

	public:
		SGGuid();
		SGGuid(const std::string& identifier);

		~SGGuid();

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
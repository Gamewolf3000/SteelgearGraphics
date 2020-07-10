#pragma once

#include <string>
#include <unordered_map>

namespace SG
{
	class SGGuid
	{
	private:


		static std::unordered_map<std::string, unsigned int> guids;
		unsigned int myID;

	public:
		SGGuid();
		SGGuid(const std::string& identifier);

		~SGGuid();

		bool operator==(const SGGuid& other) const;

		unsigned int GetID() const;
	};

}

namespace std
{
	template<>
	struct hash<SG::SGGuid>
	{
		size_t operator()(const SG::SGGuid & obj) const
		{
			return hash<int>()(obj.GetID());
		}
	};
}
#include "SGGuid.h"

namespace SG
{
	std::unordered_map<std::string, size_t> SGGuid::guids;

	SG::SGGuid::SGGuid()
	{
		auto target = guids.find("");
	
		if (target == guids.end())
		{
			myID = guids.size();
			guids[""] = guids.size();
		}
		else
		{
			myID = guids[""];
		}
	}

	SGGuid::SGGuid(const std::string& identifier)
	{
		auto target = guids.find(identifier);

		if (target == guids.end())
		{
			myID = guids.size();
			guids[identifier] = guids.size();
		}
		else
		{
			myID = guids[identifier];
		}
	}

	SGGuid::~SGGuid()
	{
	}

	bool SGGuid::operator==(const SGGuid & other) const
	{
		return myID == other.myID;
	}
	size_t SGGuid::GetID() const
	{
		return myID;
	}
}
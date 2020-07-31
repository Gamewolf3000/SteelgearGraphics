#include "SGGuid.h"

namespace SG
{
	std::unordered_map<std::string, size_t> SGGuid::guids;

	SG::SGGuid::SGGuid()
	{
		myID = 0;
	}

	SGGuid::SGGuid(const std::string& identifier)
	{
		auto target = guids.find(identifier);

		if (target == guids.end())
		{
			myID = guids.size() + 1;
			guids[identifier] = myID;
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

	bool SGGuid::operator!=(const SGGuid & other) const
	{
		return myID != other.myID;
	}

	size_t SGGuid::GetID() const
	{
		return myID;
	}
}
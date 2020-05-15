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
		std::size_t operator()(const SGGuid& other) const;

	};
}
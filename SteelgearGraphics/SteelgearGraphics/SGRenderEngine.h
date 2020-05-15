#pragma once

#include <vector>

#include "SGGraphicalEntity.h"

namespace SG
{
	typedef std::vector<SGGraphicalEntity>::size_type SGGraphicalEntityID;

	struct SGRenderSettings
	{

	};

	class SGRenderEngine
	{
	public:
		SGRenderEngine() = default;
		virtual ~SGRenderEngine() = default;


	protected:

		std::vector<SGGraphicalEntity> graphicalEntities;
	};
}
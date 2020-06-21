#pragma once

#include <vector>

#include "SGGraphicalEntity.h"

namespace SG
{
	typedef std::vector<SGGraphicalEntity>::size_type SGGraphicalEntityID;
	typedef std::vector<SGGraphicalEntity>::size_type SGGraphicalEntityGroupID;

	struct SGBackBufferSettings
	{
		int nrOfBackBuffers = 2;
		int width = 1280;
		int height = 720;
		DXGI_FORMAT format;
	};

	struct SGRenderSettings
	{
		int nrOfContexts = 1;
		SGBackBufferSettings backBufferSettings;
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
#pragma once

#include <Windows.h>
#include <dxgi1_6.h>

#if defined(_DEBUG) || defined(DEBUG)
constexpr bool DEBUG_VERSION = true;
#else
constexpr bool DEBUG_VERSION = false;
#endif

#ifdef _SGG_THREADSAFE
constexpr bool TREADEDSAFE_VERSION = true;
#else
constexpr bool TREADEDSAFE_VERSION = false;
#endif

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
		int refreshRate = 60;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		UINT multiSampleCount = 1;
		UINT multiSampleQuality = 0;
		BOOL windowedMode = true;
		DXGI_SWAP_EFFECT swapEffect = DXGI_SWAP_EFFECT_DISCARD;
		UINT flags = 0;
	};

	struct SGRenderSettings
	{
		IDXGIAdapter* adapter = nullptr;
		HWND windowHandle;
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
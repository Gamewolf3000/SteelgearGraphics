#pragma once

#include <Windows.h>
#include <dxgi1_6.h>

#include <vector>
#include <mutex>
#include <atomic>

#include "SGGraphicalEntity.h"
#include "SGGuid.h"
#include "SGThreadPool.h"
#include "SGResult.h"
#include "LockableUnorderedMap.h"

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

namespace SG
{
	typedef std::vector<SGGraphicalEntity>::size_type SGGraphicalEntityID;

	struct SGBackBufferSettings
	{
		DXGI_USAGE usage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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
		bool threadedRenderLoop = true;
		SGBackBufferSettings backBufferSettings;
	};

	struct SGGraphicsJob
	{
		SGGuid pipelineGuid;
		std::vector<SGGraphicalEntityID> entitiesToRender;
	};

	class SGRenderEngine
	{
	public:
		SGRenderEngine(const SGRenderSettings& settings);
		virtual ~SGRenderEngine() = default;

		void Render(const std::vector<SGGraphicsJob>& jobs);

		SGGraphicalEntityID CreateEntity();
		void SetEntityToGroup(const SGGraphicalEntityID& entity, const SGGuid& groupGuid);

	protected:

		void RenderThreadFunction();
		virtual void FinishFrame() = 0;
		virtual void SwapFrame() = 0;
		virtual void ExecuteJobs(const std::vector<SGGraphicsJob>& jobs) = 0;

		std::mutex entityMutex;
		std::vector<SGGraphicalEntity> graphicalEntities;
		std::vector<SGGraphicsJob> pipelineJobs[3];
		std::mutex dataIndexMutex;
		int toWorkWith = 0;
		int toUseNext = 1;
		int toUpdate = 2;
		bool threadedRenderLoop;
		SGThreadPool* threadPool;
		std::atomic<bool> engineActive = true;
		std::atomic<bool> renderthreadActive = false;
	};
}
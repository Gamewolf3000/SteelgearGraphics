#pragma once

#include <d3d11_4.h>
#include <exception>
#include <stdexcept>

#include "SGRenderEngine.h"

#include "D3D11BufferHandler.h"
#include "D3D11SamplerHandler.h"
#include "D3D11ShaderManager.h"
#include "D3D11StateHandler.h"
#include "D3D11TextureHandler.h"
#include "D3D11PipelineManager.h"
#include "D3D11DrawCallHandler.h"

namespace SG
{

	class D3D11RenderEngine : public SGRenderEngine
	{
	public:
		D3D11RenderEngine(const SGRenderSettings& settings);
		~D3D11RenderEngine();

		D3D11BufferHandler* BufferHandler();
		D3D11ShaderManager* ShaderManager();
		D3D11StateHandler* StateHandler();
		D3D11TextureHandler* TextureHandler();
		D3D11PipelineManager* PipelineManager();
		D3D11DrawCallHandler* DrawCallHandler();

	private:
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* immediateContext = nullptr;
		std::vector<ID3D11DeviceContext*> defferedContexts;
		IDXGISwapChain* swapChain = nullptr;

		D3D11BufferHandler* bufferHandler;
		D3D11SamplerHandler* samplerHandler;
		D3D11ShaderManager* shaderManager;
		D3D11StateHandler* stateHandler;
		D3D11TextureHandler* textureHandler;
		D3D11PipelineManager* pipelineManager;
		D3D11DrawCallHandler* drawCallHandler;

		void CreateDeviceAndContext(const SGRenderSettings& settings);
		void CreateSwapChain(const SGRenderSettings& settings);

		// Inherited via SGRenderEngine
		void SwapUpdateBuffer() override;
		void SwapToWorkWithBuffer() override;
		void ExecuteJobs(const std::vector<SGGraphicsJob>& jobs) override;

		void HandlePipelineJobs(const std::vector<SGGraphicsJob>& jobs, int startPos, int endPos, ID3D11DeviceContext* context);

		void HandleRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		D3D11_PRIMITIVE_TOPOLOGY TranslateTopology(const SGTopology& topology);
		void SetShaders(const SGRenderJob& job, ID3D11DeviceContext* context);
		void HandleGlobalRenderJob(const SGRenderJob& job, ID3D11DeviceContext* context);
		void HandleGroupRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		void HandleEntityRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);

		void SetConstantBuffers(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetVertexBuffers(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetIndexBuffer(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetOMViews(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetViewports(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void ExecuteDrawCall(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetConstantBuffersForShader(const std::vector<PipelineComponent>& buffers, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context, void(_stdcall ID3D11DeviceContext::*func)(UINT, UINT, ID3D11Buffer*const*));
		ID3D11Buffer* GetBuffer(const PipelineComponent& component, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		UINT GetOffset(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		UINT GetStride(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		ID3D11RenderTargetView* GetRTV(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		ID3D11DepthStencilView* GetDSV(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		SG::D3D11DrawCallHandler::DrawCall GetDrawCall(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		D3D11_VIEWPORT GetViewport(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		UINT GetVertexCount(const SGRenderJob& job, UINT vertexCount, const SGGraphicalEntityID& entity);
		UINT GetIndexCount(const SGRenderJob& job, UINT indexCount, const SGGraphicalEntityID& entity);

		void HandleClearRenderTargetJob(const SGClearRenderTargetJob& job, ID3D11DeviceContext* context);
		void HandleClearDepthStencilJob(const SGClearDepthStencilJob& job, ID3D11DeviceContext* context);
	};
}
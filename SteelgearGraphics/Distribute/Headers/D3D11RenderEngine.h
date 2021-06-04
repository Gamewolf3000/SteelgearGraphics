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
	struct RenderShaderState
	{
		ID3D11Buffer* constantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
		ID3D11ShaderResourceView* shaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	};

	struct VertexBufferState
	{
		ID3D11Buffer* buffer;
		UINT offset;
		UINT stride;
	};

	struct IndexBufferState
	{
		ID3D11Buffer* buffer;
		UINT offset;
	};

	struct RenderPipelineState
	{
		VertexBufferState vertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		IndexBufferState indexBuffer;
		RenderShaderState vertexShader;
		RenderShaderState hullShader;
		RenderShaderState domainShader;
		RenderShaderState geometryShader;
		RenderShaderState pixelShader;
		ID3D11RenderTargetView* rtvs[8];
		ID3D11UnorderedAccessView* uavs[8];
		D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11DepthStencilView* dsv;
		ID3D11RasterizerState* rasterizerState;
		ID3D11BlendState* blendState;
	};

	struct ComputePipelineState
	{
		ID3D11Buffer* constantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
		ID3D11ShaderResourceView* shaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11UnorderedAccessView* unorderedAccessViews[8];
		ID3D11SamplerState* samplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	};

	class D3D11RenderEngine : public SGRenderEngine
	{
	public:
		D3D11RenderEngine(const SGRenderSettings& settings);
		~D3D11RenderEngine();

		D3D11BufferHandler* BufferHandler();
		D3D11SamplerHandler* SamplerHandler();
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

		void FinishFrame() override;
		void SwapFrame() override;
		void ExecuteJobs(const std::vector<SGGraphicsJob>& jobs) override;

		void HandlePipelineJobs(const std::vector<SGGraphicsJob>& jobs, int startPos, int endPos, ID3D11DeviceContext* context);

		void HandleRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		D3D11_PRIMITIVE_TOPOLOGY TranslateTopology(const SGTopology& topology);
		void SetShaders(const SGRenderJob& job, ID3D11DeviceContext* context);
		void HandleGlobalRenderJob(const SGRenderJob& job, ID3D11DeviceContext* context);
		void HandleGroupRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		void HandleEntityRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);

		void HandleComputeJob(const SGComputeJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		void HandleGlobalComputeJob(const SGComputeJob& job, ID3D11DeviceContext* context);

		void ClearNecessaryResources(const SGRenderJob& job, ID3D11DeviceContext* context);
		void ClearNecessaryResources(const SGComputeJob& job, ID3D11DeviceContext* context);
		void ClearVertexBuffers(const std::vector<SGVertexBuffer>& vertexBuffers, ID3D11DeviceContext* context);

		void ClearConstantBuffers(const SGRenderJob& job, ID3D11DeviceContext* context);
		void ClearConstantBuffersForShader(const std::vector<ConstantBuffer>& constantBuffers,
			ID3D11DeviceContext* context, void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11Buffer* const*));

		void ClearShaderResourceViews(const SGRenderJob& job, ID3D11DeviceContext* context);
		void ClearShaderResourceViewsForShader(const std::vector<ResourceView>& srvs,
			ID3D11DeviceContext* context, void(_stdcall ID3D11DeviceContext::* func)(UINT, UINT, ID3D11ShaderResourceView* const*));

		void ClearOMViews(const SGRenderJob& job, ID3D11DeviceContext* context);

		void SetConstantBuffers(const SGRenderJob& job, RenderPipelineState& currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetShaderResourceViews(const SGRenderJob& job, RenderPipelineState& currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetSamplerStates(const SGRenderJob& job, RenderPipelineState& currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetVertexBuffers(const SGRenderJob& job, VertexBufferState currentState[],
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetIndexBuffer(const SGRenderJob& job, IndexBufferState& currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetOMViews(const SGRenderJob& job, RenderPipelineState& currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetViewports(const SGRenderJob& job, D3D11_VIEWPORT currentState[],
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetStates(const SGRenderJob& job, RenderPipelineState& currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void ExecuteDrawCall(const SGRenderJob& job, const SGGraphicalEntityID& entity, unsigned int nrInGroup, ID3D11DeviceContext* context);
		void ExecuteDrawCall(const SGRenderJob& job, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		void SetConstantBuffersForShader(const std::vector<ConstantBuffer>& buffers, ID3D11Buffer** currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context,
			void(_stdcall ID3D11DeviceContext::*func)(UINT, UINT, ID3D11Buffer*const*));
		void SetShaderResourceViewsForShader(const std::vector<ResourceView>& srvs, ID3D11ShaderResourceView** currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context,
			void(_stdcall ID3D11DeviceContext::*func)(UINT, UINT, ID3D11ShaderResourceView *const*));
		void SetSamplerStatesForShader(const std::vector<PipelineComponent>& samplers, ID3D11SamplerState** currentState,
			const SGGraphicalEntityID& entity, ID3D11DeviceContext* context,
			void(_stdcall ID3D11DeviceContext::*func)(UINT, UINT, ID3D11SamplerState *const*));

		ID3D11Buffer* GetBuffer(const PipelineComponent& component, const SGGraphicalEntityID& entity, ID3D11DeviceContext* context);
		ID3D11SamplerState* GetSamplerState(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		UINT GetOffset(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		UINT GetStride(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		UINT GetStrideFromVB(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		ID3D11ShaderResourceView* GetSRV(const ResourceView& view, const SGGraphicalEntityID& entity);
		ID3D11RenderTargetView* GetRTV(const ResourceView& view, const SGGraphicalEntityID& entity);
		ID3D11DepthStencilView* GetDSV(const ResourceView& view, const SGGraphicalEntityID& entity);
		ID3D11UnorderedAccessView* GetUAV(const ResourceView& view, const SGGraphicalEntityID& entity);
		ID3D11RasterizerState* GetRasterizerState(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		SG::D3D11DrawCallHandler::DrawCall GetDrawCall(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		SG::D3D11DrawCallHandler::DispatchCall GetDispatchCall(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		D3D11_VIEWPORT GetViewport(const PipelineComponent& component, const SGGraphicalEntityID& entity);
		UINT GetVertexCount(const SGRenderJob& job, UINT vertexCount, const SGGraphicalEntityID& entity);
		UINT GetIndexCount(const SGRenderJob& job, UINT indexCount, const SGGraphicalEntityID& entity);

		void HandleClearRenderTargetJob(const SGClearRenderTargetJob& job, ID3D11DeviceContext* context);
		void HandleClearDepthStencilJob(const SGClearDepthStencilJob& job, ID3D11DeviceContext* context);
	};
}
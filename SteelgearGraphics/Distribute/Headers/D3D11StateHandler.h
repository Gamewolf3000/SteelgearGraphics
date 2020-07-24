#pragma once

#include <d3d11_4.h>
#include <utility>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"


namespace SG
{
	enum class FillMode
	{
		WIREFRAME,
		SOLID
	};

	enum class CullMode
	{
		NONE,
		FRONT,
		BACK
	};

	enum class DepthWriteMask
	{
		ZERO,
		ALL
	};

	enum class DepthStencilOp
	{
		KEEP,
		ZERO,
		REPLACE,
		INCR_SAT,
		DECR_SAT,
		INVERT,
		INCR,
		DECR
	};

	enum class Blend
	{
		ZERO,
		ONE,
		SRC_COLOR,
		INV_SRC_COLOR,
		SRC_ALPHA,
		INV_SRC_ALPHA,
		DEST_ALPHA,
		INV_DEST_ALPHA,
		DEST_COLOR,
		INV_DEST_COLOR,
		SRC_ALPHA_SAT,
		BLEND_FACTOR,
		INV_BLEND_FACTOR,
		SRC1_COLOR,
		INV_SRC1_COLOR,
		SRC1_ALPHA,
		INV_SRC1_ALPHA
	};

	enum class BlendOp
	{
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX
	};

	struct RenderTargetBlending
	{
		BOOL blendEnable;
		Blend srcBlend;
		Blend destBlend;
		BlendOp blendOp;
		Blend srcBlendAlpha;
		Blend destBlendAlpha;
		BlendOp blendOpAlpha;
		UINT8 renderTargetWriteMask;
	};

	class D3D11StateHandler : public SGGraphicsHandler
	{
	public:

		D3D11StateHandler(ID3D11Device* device);
		~D3D11StateHandler();

		SGResult CreateRasterizerState(const SGGuid& guid, FillMode fill, CullMode cull, BOOL frontCounterClockwise, INT depthBias, FLOAT depthBiasClamp,
										FLOAT slopeScaledDepthBias, BOOL depthClipEnable, BOOL scissorEnable, BOOL multisampleEnable, BOOL antialiasedEnable);

		SGResult CreateDepthStencilState(const SGGuid& guid, BOOL depthEnable, DepthWriteMask mask, ComparisonFunction depthFunc, BOOL stencilEnable, UINT8 stencilReadMask,
											UINT8 stencilWriteMask, DepthStencilOp frontFaceStencilFailOp, DepthStencilOp frontFaceStencilDepthFailOp,
											DepthStencilOp frontFaceStencilPassOp, ComparisonFunction frontFaceStencilFunc, DepthStencilOp backFaceStencilFailOp,
											DepthStencilOp backFaceStencilDepthFailOp, DepthStencilOp backFaceStencilPassOp, ComparisonFunction backFaceStencilFunc);

		SGResult CreateBlendState(const SGGuid& guid, BOOL alphaToCoverageEnable, BOOL independentBlendEnable, std::vector<RenderTargetBlending> renderTargets);

		SGResult CreateViewport(const SGGuid& guid, FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height, FLOAT minDepth, FLOAT maxDepth);

		SGResult CreateDepthStencilData(const SGGuid& guid, UINT stencilRef);

		SGResult CreateBlendData(const SGGuid& guid, const FLOAT blendFactor[4], UINT sampleMask);


	private:

		struct D3D11StateData
		{
			union
			{
				ID3D11RasterizerState* rasterizer;
				ID3D11DepthStencilState* depthStencil;
				ID3D11BlendState* blend;
			} state;

		};

		struct DepthStencilSetData
		{
			UINT stencilRef;
		};

		struct BlendSetData
		{
			const FLOAT blendFactor[4];
			UINT sampleMask;
		};

		struct D3D11SetData
		{
			union
			{
				DepthStencilSetData depthStencil;
				BlendSetData blend;
			};
		};

		struct D3D11ViewportData
		{
			D3D11_VIEWPORT viewport;
		};

		std::unordered_map<SGGuid, D3D11StateData> states;
		std::unordered_map<SGGuid, D3D11SetData> setData;
		std::unordered_map<SGGuid, D3D11ViewportData> viewports;

		ID3D11Device* device;

	};
}
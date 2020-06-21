#pragma once

#include <d3d11_4.h>

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

		SGResult CreateDepthStencilState(const SGGuid& guid, BOOL blendEnable, Blend srcBlend, Blend destBlend, BlendOp blendOp,
			Blend srcBlendAlpha, Blend destBlendAlpha, BlendOp blendOpAlpha, UINT8 renderTargetWriteMask);

		SGResult CreateViewport(const SGGuid& guid, FLOAT topLeftX, FLOAT topLeftY, FLOAT width, FLOAT height, FLOAT minDepth, FLOAT maxDepth);

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

		struct D3D11ViewportData
		{
			D3D11_VIEWPORT viewport;
		};

		std::unordered_map<SGGuid, D3D11StateData> states;
		std::unordered_map<SGGuid, D3D11ViewportData> viewports;

		ID3D11Device* device;

	};
}
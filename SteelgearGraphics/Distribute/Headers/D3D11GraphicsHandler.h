#pragma once

#include <d3d11_4.h>

#include "SGGraphicsHandler.h"
#include "D3D11ResourceViewData.h"

namespace SG
{
	class D3D11GraphicsHandler : public SGGraphicsHandler
	{
	public:
		D3D11GraphicsHandler() = default;
		virtual ~D3D11GraphicsHandler() = default;


	protected:

		void* GetGlobalResourceView(const SGGuid& guid, const ResourceViewType& resourceViewType,
			FrameMap<SGGuid, D3D11ResourceViewData>& views, const std::string& associatedResourceName);

		void* GetGroupResourceView(const SGGuid& guid, const SGGuid& groupGuid, const ResourceViewType& resourceViewType,
			FrameMap<SGGuid, D3D11ResourceViewData>& views, const std::string& associatedResourceName);

		void* GetEntityResourceView(const SGGuid& guid, const SGGraphicalEntityID& entity, const ResourceViewType& resourceViewType,
			FrameMap<SGGuid, D3D11ResourceViewData>& views, const std::string& associatedResourceName);

	private:
		std::string TranslateViewToString(const ResourceViewType& resourceViewType);

	};
}
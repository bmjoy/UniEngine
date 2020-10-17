#pragma once
#include "Core.h"
#include "RenderTarget.h"
#include "UniEngineAPI.h"

namespace UniEngine {

	class UNIENGINE_API PointLightShadowMap :
		public RenderTarget
	{
		std::unique_ptr<GLTextureCubeMapArray> _DepthCubeMapArray;
	public:
		PointLightShadowMap(size_t amount, size_t resolutionX = 2048, size_t resolutionY = 2048);
		std::unique_ptr<GLTextureCubeMapArray>& DepthCubeMapArray();
		void Bind();
	};

}
#pragma once
#include "Core.h"
#include "RenderTarget.h"
namespace UniEngine {
	class UECORE_API DirectionalLightShadowMap :
		public RenderTarget
	{
		GLTexture* _DepthMapArray;
		
	public:
		DirectionalLightShadowMap(size_t amount, float resolutionX = 2048.0f, float resolutionY = 2048.0f);
		GLTexture* DepthCubeMapArray();
		void Bind(GLint layer);
	};
}

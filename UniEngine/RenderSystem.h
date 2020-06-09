#pragma once
#include "Core.h"
#include "RenderManager.h"
namespace UniEngine {

	class RenderSystem : public SystemBase
	{
		void RenderToCamera(Camera* camera);
	public:
		RenderSystem();
		void OnCreate();
		void OnDestroy();
		void Update();
	};
}

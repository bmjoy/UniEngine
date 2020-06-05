#pragma once
#include "ManagerBase.h"
#include "Core.h"
#include "RenderManager.h"

#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "SpotLightComponent.h"
#include "DirectionalLightShadowMap.h"
#include "PointLightShadowMap.h"
#include "Default.h"
namespace UniEngine {
	class LightingManager :
		public ManagerBase
	{
		static GLUBO* _DirectionalLightBlock;
		static GLUBO* _PointLightBlock;
		static GLUBO* _SpotLightBlock;

		static DirectionalLight _DirectionalLights[Default::ShaderIncludes::MaxDirectionalLightAmount];
		static PointLight _PointLights[Default::ShaderIncludes::MaxPointLightAmount];
		static SpotLight _SpotLights[Default::ShaderIncludes::MaxSpotLightAmount];

		static bool _UpdateDirectionalLightBlock;
		static bool _UpdatePointLightBlock;
		static bool _UpdateSpotLightBlock;

		static GLProgram* _DirectionalLightProgram;
		static GLProgram* _PointLightProgram;
		friend class RenderManager;
		static DirectionalLightShadowMap* _DirectionalLightShadowMap;
		static PointLightShadowMap* _PointLightShadowMap;

	public:
		static void Init();
		static void Start();
	};
}

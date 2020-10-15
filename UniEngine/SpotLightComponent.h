#pragma once
#include "Core.h"
#include "UniEngineAPI.h"
namespace UniEngine {
	struct UNIENGINE_API SpotLight
	{
		glm::vec4 position;
		glm::vec4 direction;
		float cutOff;
		float outerCutOff;

		float constant;
		float linear;
		float quadratic;

		glm::vec4 diffuse;
		glm::vec4 specular;
	};
	struct UNIENGINE_API SpotLightComponent :
		ComponentBase
	{
		SpotLight value;
	};
}

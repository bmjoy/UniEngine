#pragma once
#include "UniEngineAPI.h"
namespace UniEngine {
	struct UNIENGINE_API DirectionalLight
	{
		glm::vec4 position;
		glm::vec4 direction;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::mat4 lightSpaceMatrix;
		glm::vec4 ReservedParameters;
	};
	class UNIENGINE_API DirectionalLightComponent :
		public SharedComponentBase
	{
	public:
		glm::vec3 diffuse;
		glm::vec3 specular;
		float depthBias = 0.01f;
		float normalOffset = 0.01f;
		size_t GetHashCode();
	};

}
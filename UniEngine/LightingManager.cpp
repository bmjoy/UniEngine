#include "pch.h"
#include "LightingManager.h"
#include "MeshMaterialComponent.h"
using namespace UniEngine;

float LightingManager::_ShadowCascadeSplit[Default::ShaderIncludes::ShadowCascadeAmount];
GLUBO* LightingManager::_ShadowCascadeInfoBlock;
ShadowCascadeInfo LightingManager::_ShadowCascadeInfo;
unsigned LightingManager::_DirectionalShadowMapResolution = 2048;

CameraComponent* LightingManager::_TargetMainCamera;
Entity LightingManager::_TargetMainCameraEntity;

GLUBO* LightingManager::_DirectionalLightBlock;
GLUBO* LightingManager::_PointLightBlock;
GLUBO* LightingManager::_SpotLightBlock;

DirectionalLight LightingManager::_DirectionalLights[Default::ShaderIncludes::MaxDirectionalLightAmount];
PointLight LightingManager::_PointLights[Default::ShaderIncludes::MaxPointLightAmount];
SpotLight LightingManager::_SpotLights[Default::ShaderIncludes::MaxSpotLightAmount];

bool LightingManager::_UpdateDirectionalLightBlock;
bool LightingManager::_UpdatePointLightBlock;
bool LightingManager::_UpdateSpotLightBlock;

DirectionalLightShadowMap* LightingManager::_DirectionalLightShadowMap;
PointLightShadowMap* LightingManager::_PointLightShadowMap;

GLProgram* LightingManager::_DirectionalLightProgram;
GLProgram* LightingManager::_PointLightProgram;
GLProgram* LightingManager::_DirectionalLightInstancedProgram;
GLProgram* LightingManager::_PointLightInstancedProgram;


void UniEngine::LightingManager::Init()
{
	_ShadowCascadeSplit[0] = 0.15f;
	_ShadowCascadeSplit[1] = 0.3f;
	_ShadowCascadeSplit[2] = 0.6f;
	_ShadowCascadeSplit[3] = 1.0f;
	_ShadowCascadeInfoBlock = new GLUBO();
	_ShadowCascadeInfoBlock->SetData(sizeof(ShadowCascadeInfo), NULL, GL_DYNAMIC_DRAW);
	_ShadowCascadeInfoBlock->SetBase(4);

#pragma region LightInfoBlocks
	_DirectionalLightBlock = new GLUBO();
	_PointLightBlock = new GLUBO();
	_SpotLightBlock = new GLUBO();
	size_t size = 16 + Default::ShaderIncludes::MaxDirectionalLightAmount * sizeof(DirectionalLight);
	_DirectionalLightBlock->SetData(size, NULL, GL_DYNAMIC_DRAW);
	_DirectionalLightBlock->SetBase(1);
	size = 16 + Default::ShaderIncludes::MaxPointLightAmount * sizeof(PointLight);
	_PointLightBlock->SetData(size, NULL, GL_DYNAMIC_DRAW);
	_PointLightBlock->SetBase(2);
	size = 16 + Default::ShaderIncludes::MaxSpotLightAmount * sizeof(SpotLight);
	_SpotLightBlock->SetData(size, NULL, GL_DYNAMIC_DRAW);
	_SpotLightBlock->SetBase(3);
#pragma endregion
#pragma region DirectionalLight
	_DirectionalLightShadowMap = new DirectionalLightShadowMap(Default::ShaderIncludes::MaxDirectionalLightAmount * Default::ShaderIncludes::ShadowCascadeAmount, _DirectionalShadowMapResolution, _DirectionalShadowMapResolution);

	std::string vertShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Vertex/DirectionalLightShadowMap.vert");
	std::string fragShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Fragment/DirectionalLightShadowMap.frag");
	_DirectionalLightProgram = new GLProgram(
		new GLShader(ShaderType::Vertex, &vertShaderCode),
		new GLShader(ShaderType::Fragment, &fragShaderCode)
	);

	vertShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Vertex/DirectionalLightShadowMapInstanced.vert");
	_DirectionalLightInstancedProgram = new GLProgram(
		new GLShader(ShaderType::Vertex, &vertShaderCode),
		new GLShader(ShaderType::Fragment, &fragShaderCode)
	);
#pragma endregion
#pragma region PointLight
	_PointLightShadowMap = new PointLightShadowMap(Default::ShaderIncludes::MaxPointLightAmount);
	vertShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Vertex/PointLightShadowMap.vert");
	fragShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Fragment/PointLightShadowMap.frag");
	std::string geomShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Geometry/PointLightShadowMap.geom");

	_PointLightProgram = new GLProgram(
		new GLShader(ShaderType::Vertex, &vertShaderCode),
		new GLShader(ShaderType::Fragment, &fragShaderCode),
		new GLShader(ShaderType::Geometry, &geomShaderCode)
	);

	vertShaderCode = std::string("#version 460 core\n") +
		FileIO::LoadFileAsString("Shaders/Vertex/PointLightShadowMapInstanced.vert");
	_PointLightInstancedProgram = new GLProgram(
		new GLShader(ShaderType::Vertex, &vertShaderCode),
		new GLShader(ShaderType::Fragment, &fragShaderCode),
		new GLShader(ShaderType::Geometry, &geomShaderCode)
	);
#pragma endregion
	_UpdateDirectionalLightBlock = true;
	_UpdatePointLightBlock = true;
	_UpdateSpotLightBlock = true;

	GLTexture::Activate(GL_TEXTURE0);
	LightingManager::_DirectionalLightShadowMap->DepthCubeMapArray()->Bind(GL_TEXTURE_2D_ARRAY);
	GLTexture::Activate(GL_TEXTURE1);
	LightingManager::_PointLightShadowMap->DepthCubeMapArray()->Bind(GL_TEXTURE_CUBE_MAP_ARRAY);
	GLTexture::BindDefault();
}

void UniEngine::LightingManager::Start()
{
	Camera* camera = _TargetMainCamera->Value;
	glm::vec3 cameraPos = EntityManager::GetComponentData<Position>(_TargetMainCameraEntity).value;
	if (_UpdateDirectionalLightBlock) {
		auto worldBound = _World->GetBound();
		glm::vec3 maxBound = worldBound.Center + worldBound.Size;
		glm::vec3 minBound = worldBound.Center - worldBound.Size;

		auto directionLightsList = EntityManager::QuerySharedComponents<DirectionalLightComponent>();
		if (directionLightsList != nullptr) {
			size_t size = directionLightsList->size();
			for (int i = 0; i < size; i++) {
#pragma region DirectionalLight data collection
				SCOC* scoc = directionLightsList->at(i);
				Entity lightEntity = scoc->second->second->at(0);
				glm::vec3 position = EntityManager::GetComponentData<Position>(lightEntity).value;
				_DirectionalLights[i].position = glm::vec4(position, 0);
				glm::vec3 lightTarget = glm::vec3(0.0f);
				glm::vec3 lightDir = glm::normalize(glm::vec3(lightTarget.x - _DirectionalLights[i].position.x, lightTarget.y - _DirectionalLights[i].position.y, lightTarget.z - _DirectionalLights[i].position.z));
				
				float x1 = (maxBound.x - cameraPos.x) / lightDir.x;
				float x2 = (minBound.x - cameraPos.x) / lightDir.x;
				if (x1 > x2) {
					float temp = x1;
					x1 = x2;
					x2 = temp;
				}
				float y1 = (maxBound.y - cameraPos.y) / lightDir.y;
				float y2 = (minBound.y - cameraPos.y) / lightDir.y;
				if (y1 > y2) {
					float temp = y1;
					y1 = y2;
					y2 = temp;
				}
				float z1 = (maxBound.z - cameraPos.z) / lightDir.z;
				float z2 = (minBound.z - cameraPos.z) / lightDir.z;
				if (z1 > z2) {
					float temp = z1;
					z1 = z2;
					z2 = temp;
				}

				_DirectionalLights[i].direction = glm::normalize(glm::vec4(lightDir, 0.0f));
				DirectionalLightComponent* dlc = dynamic_cast<DirectionalLightComponent*>(scoc->first);
				_DirectionalLights[i].diffuse = glm::vec4(dlc->diffuse, 0);
				_DirectionalLights[i].specular = glm::vec4(dlc->specular, 0);
#pragma endregion
				for (int split = 0; split < Default::ShaderIncludes::ShadowCascadeAmount; split++) {
					
					float splitStart = camera->_Near;
					float splitEnd = camera->_Far;
					if (split != 0) splitStart = camera->_Near + (camera->_Far - camera->_Near) * _ShadowCascadeSplit[split - 1];
					if (split != Default::ShaderIncludes::ShadowCascadeAmount - 1) splitEnd = camera->_Near + (camera->_Far - camera->_Near) * _ShadowCascadeSplit[split];
					_ShadowCascadeInfo.SplitDistance[split] = splitEnd;
					glm::mat4 lightProjection, lightView;
					float max = 0;
#pragma region AABB
					/*
					glm::vec3 cornerPoints[8];
					camera->CalculateFrustumPoints(splitStart, splitEnd, cameraPos, cornerPoints);
					glm::vec3 cameraFrustumCenter = camera->_Front * ((splitEnd - splitStart) / 2.0f + splitStart) + cameraPos;

					lightView = glm::lookAt(cameraFrustumCenter - lightDir * (dlc->farPlane - dlc->nearPlane) / 2.0f, cameraFrustumCenter, glm::vec3(0.0, 1.0, 0.0));



					max = glm::max(max, glm::distance(cornerPoints[0], ClosestPointOnLine(cornerPoints[0], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[1], ClosestPointOnLine(cornerPoints[1], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[2], ClosestPointOnLine(cornerPoints[2], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[3], ClosestPointOnLine(cornerPoints[3], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[4], ClosestPointOnLine(cornerPoints[4], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[5], ClosestPointOnLine(cornerPoints[5], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[6], ClosestPointOnLine(cornerPoints[6], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					max = glm::max(max, glm::distance(cornerPoints[7], ClosestPointOnLine(cornerPoints[7], cameraFrustumCenter, cameraFrustumCenter - lightDir)));
					*/
#pragma endregion
#pragma region Sphere
					max = splitEnd;
#pragma endregion
					float backPlane, forwardPlane;
					backPlane = glm::max(glm::max(x1, y1), z1) - max;
					forwardPlane = glm::min(glm::min(x2, y2), z2) + max;

					lightView = glm::lookAt(cameraPos + lightDir * backPlane, cameraPos, glm::vec3(0.0, 1.0, 0.0));
					lightProjection = glm::ortho(-max, max, -max, max, 0.0f, forwardPlane - backPlane);

#pragma region Fix Shimmering due to the movement of the camera
					glm::mat4 shadowMatrix = lightProjection * lightView;
					glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
					shadowOrigin = shadowMatrix * shadowOrigin;
					GLfloat storedW = shadowOrigin.w;
					shadowOrigin = shadowOrigin * (float)_DirectionalShadowMapResolution / 2.0f;
					glm::vec4 roundedOrigin = glm::round(shadowOrigin);
					glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
					roundOffset = roundOffset * 2.0f / (float)_DirectionalShadowMapResolution;
					roundOffset.z = 0.0f;
					roundOffset.w = 0.0f;
					glm::mat4 shadowProj = lightProjection;
					shadowProj[3] += roundOffset;
					lightProjection = shadowProj;
#pragma endregion
					_DirectionalLights[i].lightSpaceMatrix[split] = lightProjection * lightView;
					_DirectionalLights[i].ReservedParameters = glm::vec4(forwardPlane - backPlane, 0.0f, dlc->depthBias, dlc->normalOffset);
#pragma region Shadowmap pass
					_DirectionalLightShadowMap->Bind(i * 4 + split);
					glEnable(GL_DEPTH_TEST);
					glClear(GL_DEPTH_BUFFER_BIT);

					_DirectionalLightProgram->Bind();
					_DirectionalLightProgram->SetFloat4x4("lightSpaceMatrix", _DirectionalLights[i].lightSpaceMatrix[split]);

					auto meshMaterials = EntityManager::QuerySharedComponents<MeshMaterialComponent>();
					if (meshMaterials != nullptr) {
						for (auto i : *meshMaterials) {
							auto mmc = dynamic_cast<MeshMaterialComponent*>(i->first);
							if (mmc->_CastShadow) {
								auto entities = EntityManager::QueryEntities<MeshMaterialComponent>(mmc);
								for (auto j : *entities) {
									auto mesh = mmc->_Mesh;
									auto ltw = EntityManager::GetComponentData<LocalToWorld>(j).value;
									auto scale = EntityManager::GetComponentData<Scale>(j).value;
#pragma region Sphere test 1. discard useless meshes. 2. Calculate scene boundary for lightFrustum;
									auto bound = mesh->GetBound();
									glm::vec3 center = ltw * glm::vec4(bound.Center, 1.0f);
									float radius = glm::length(bound.Size * scale);

									if (glm::distance(center, ClosestPointOnLine(center, cameraPos, cameraPos - lightDir)) - radius > max) {
										continue;
									}
#pragma endregion
									_DirectionalLightProgram->SetFloat4x4("model", ltw);
									mesh->Enable();
									mesh->VAO()->DisableAttributeArray(12);
									mesh->VAO()->DisableAttributeArray(13);
									mesh->VAO()->DisableAttributeArray(14);
									mesh->VAO()->DisableAttributeArray(15);
									glDrawElements(GL_TRIANGLES, mesh->Size(), GL_UNSIGNED_INT, 0);
								}
							}
						}
					}

					_DirectionalLightInstancedProgram->Bind();
					_DirectionalLightInstancedProgram->SetFloat4x4("lightSpaceMatrix", _DirectionalLights[i].lightSpaceMatrix[split]);

					auto instancedMeshMaterials = EntityManager::QuerySharedComponents<InstancedMeshMaterialComponent>();
					if (instancedMeshMaterials != nullptr) {
						for (auto i : *instancedMeshMaterials) {
							InstancedMeshMaterialComponent* immc = dynamic_cast<InstancedMeshMaterialComponent*>(i->first);
							if (immc->_CastShadow) {
								auto entities = EntityManager::QueryEntities<InstancedMeshMaterialComponent>(immc);
								size_t count = immc->_Matrices->size();
								GLVBO* matricesBuffer = new GLVBO();
								matricesBuffer->SetData(count * sizeof(glm::mat4), &immc->_Matrices->at(0), GL_STATIC_DRAW);
								for (auto j : *entities) {
									auto mesh = immc->_Mesh;
									_DirectionalLightInstancedProgram->SetFloat4x4("model", EntityManager::GetComponentData<LocalToWorld>(j).value);
									mesh->Enable();
									mesh->VAO()->EnableAttributeArray(12);
									mesh->VAO()->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
									mesh->VAO()->EnableAttributeArray(13);
									mesh->VAO()->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
									mesh->VAO()->EnableAttributeArray(14);
									mesh->VAO()->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
									mesh->VAO()->EnableAttributeArray(15);
									mesh->VAO()->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
									mesh->VAO()->SetAttributeDivisor(12, 1);
									mesh->VAO()->SetAttributeDivisor(13, 1);
									mesh->VAO()->SetAttributeDivisor(14, 1);
									mesh->VAO()->SetAttributeDivisor(15, 1);
									glDrawElementsInstanced(GL_TRIANGLES, mesh->Size(), GL_UNSIGNED_INT, 0, count);
									GLVAO::BindDefault();
								}
								delete matricesBuffer;
							}
						}
					}
#pragma endregion
				}
			}
			_DirectionalLightBlock->SubData(0, 4, &size);
			if (size != 0) {
				_DirectionalLightBlock->SubData(16, size * sizeof(DirectionalLight), &_DirectionalLights[0]);
				_ShadowCascadeInfoBlock->SubData(0, sizeof(ShadowCascadeInfo), &_ShadowCascadeInfo);
			}
		}
	}

	if (_UpdatePointLightBlock) {
		auto pointLightsList = EntityManager::QuerySharedComponents<PointLightComponent>();
		if (pointLightsList != nullptr) {
			size_t size = pointLightsList->size();
			for (int i = 0; i < size; i++) {
				SCOC* scoc = pointLightsList->at(i);
				Entity lightEntity = scoc->second->second->at(0);
				glm::vec3 position = EntityManager::GetComponentData<Position>(lightEntity).value;
				_PointLights[i].position = glm::vec4(position, 0);
				PointLightComponent* plc = dynamic_cast<PointLightComponent*>(scoc->first);
				_PointLights[i].constantLinearQuadFarPlane.x = plc->constant;
				_PointLights[i].constantLinearQuadFarPlane.y = plc->linear;
				_PointLights[i].constantLinearQuadFarPlane.z = plc->quadratic;
				_PointLights[i].diffuse = glm::vec4(plc->diffuse, 0);
				_PointLights[i].specular = glm::vec4(plc->specular, 0);
				_PointLights[i].constantLinearQuadFarPlane.w = plc->farPlane;
				_PointLights[i].ReservedParameters = glm::vec4(plc->bias, 0, 0, 0);
			}
			_PointLightBlock->SubData(0, 4, &size);
			if (size != 0)_PointLightBlock->SubData(16, size * sizeof(PointLight), &_PointLights[0]);
			_PointLightShadowMap->Bind(0);
			glCullFace(GL_FRONT);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);
			for (int i = 0; i < size; i++) {
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), _PointLightShadowMap->GetResolutionRatio(), 1.0f, _PointLights[i].constantLinearQuadFarPlane.w);
				glm::mat4 shadowTransforms[6];
				auto lightPos = glm::vec3(_PointLights[i].position);
				shadowTransforms[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				shadowTransforms[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				shadowTransforms[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				shadowTransforms[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
				shadowTransforms[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				shadowTransforms[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));


				_PointLightProgram->Bind();
				_PointLightProgram->SetInt("index", i);
				_PointLightProgram->SetFloat4x4("shadowMatrices[0]", shadowTransforms[0]);
				_PointLightProgram->SetFloat4x4("shadowMatrices[1]", shadowTransforms[1]);
				_PointLightProgram->SetFloat4x4("shadowMatrices[2]", shadowTransforms[2]);
				_PointLightProgram->SetFloat4x4("shadowMatrices[3]", shadowTransforms[3]);
				_PointLightProgram->SetFloat4x4("shadowMatrices[4]", shadowTransforms[4]);
				_PointLightProgram->SetFloat4x4("shadowMatrices[5]", shadowTransforms[5]);
				_PointLightProgram->SetFloat("far_plane", _PointLights[i].constantLinearQuadFarPlane.w);
				_PointLightProgram->SetFloat3("lightPos", lightPos);

				auto meshMaterials = EntityManager::QuerySharedComponents<MeshMaterialComponent>();
				for (auto j : *meshMaterials) {
					auto mmc = dynamic_cast<MeshMaterialComponent*>(j->first);
					if (mmc->_CastShadow) {
						auto entities = EntityManager::QueryEntities<MeshMaterialComponent>(mmc);
						for (auto k : *entities) {
							auto mesh = mmc->_Mesh;
							_PointLightProgram->SetFloat4x4("model", EntityManager::GetComponentData<LocalToWorld>(k).value);
							mesh->Enable();
							mesh->VAO()->DisableAttributeArray(12);
							mesh->VAO()->DisableAttributeArray(13);
							mesh->VAO()->DisableAttributeArray(14);
							mesh->VAO()->DisableAttributeArray(15);
							glDrawElements(GL_TRIANGLES, mesh->Size(), GL_UNSIGNED_INT, 0);
						}
					}
				}


				_PointLightInstancedProgram->Bind();
				_PointLightInstancedProgram->SetInt("index", i);
				_PointLightInstancedProgram->SetFloat4x4("shadowMatrices[0]", shadowTransforms[0]);
				_PointLightInstancedProgram->SetFloat4x4("shadowMatrices[1]", shadowTransforms[1]);
				_PointLightInstancedProgram->SetFloat4x4("shadowMatrices[2]", shadowTransforms[2]);
				_PointLightInstancedProgram->SetFloat4x4("shadowMatrices[3]", shadowTransforms[3]);
				_PointLightInstancedProgram->SetFloat4x4("shadowMatrices[4]", shadowTransforms[4]);
				_PointLightInstancedProgram->SetFloat4x4("shadowMatrices[5]", shadowTransforms[5]);
				_PointLightInstancedProgram->SetFloat("far_plane", _PointLights[i].constantLinearQuadFarPlane.w);
				_PointLightInstancedProgram->SetFloat3("lightPos", lightPos);

				auto instancedMeshMaterials = EntityManager::QuerySharedComponents<InstancedMeshMaterialComponent>();
				if (instancedMeshMaterials != nullptr) {
					for (auto i : *instancedMeshMaterials) {
						InstancedMeshMaterialComponent* immc = dynamic_cast<InstancedMeshMaterialComponent*>(i->first);
						if (immc->_CastShadow) {
							auto entities = EntityManager::QueryEntities<InstancedMeshMaterialComponent>(immc);
							size_t count = immc->_Matrices->size();
							GLVBO* matricesBuffer = new GLVBO();
							matricesBuffer->SetData(count * sizeof(glm::mat4), &immc->_Matrices->at(0), GL_STATIC_DRAW);
							for (auto j : *entities) {
								auto mesh = immc->_Mesh;
								_PointLightInstancedProgram->SetFloat4x4("model", EntityManager::GetComponentData<LocalToWorld>(j).value);
								mesh->Enable();
								mesh->VAO()->EnableAttributeArray(12);
								mesh->VAO()->SetAttributePointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
								mesh->VAO()->EnableAttributeArray(13);
								mesh->VAO()->SetAttributePointer(13, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
								mesh->VAO()->EnableAttributeArray(14);
								mesh->VAO()->SetAttributePointer(14, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
								mesh->VAO()->EnableAttributeArray(15);
								mesh->VAO()->SetAttributePointer(15, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
								mesh->VAO()->SetAttributeDivisor(12, 1);
								mesh->VAO()->SetAttributeDivisor(13, 1);
								mesh->VAO()->SetAttributeDivisor(14, 1);
								mesh->VAO()->SetAttributeDivisor(15, 1);
								glDrawElementsInstanced(GL_TRIANGLES, mesh->Size(), GL_UNSIGNED_INT, 0, count);
								GLVAO::BindDefault();
							}
							delete matricesBuffer;
						}
					}
				}

			}
			glCullFace(GL_BACK);
		}
	}
	/*
	if (_UpdateSpotLightBlock) {
		size_t size = _SpotLights.size();
		_SpotLightBlock->SubData(0, 4, &size);
		size = size * sizeof(SpotLight);
		if (size != 0)_SpotLightBlock->SubData(16, size, &_SpotLights[0]);
	}

	*/
}

glm::vec3 UniEngine::LightingManager::ClosestPointOnLine(glm::vec3 point, glm::vec3 a, glm::vec3 b)
{
	float LineLength = distance(a, b);
	glm::vec3 Vector = point - a;
	glm::vec3 LineDirection = (b - a) / LineLength;

	// Project Vector to LineDirection to get the distance of point from a
	float Distance = dot(Vector, LineDirection);
	return a + LineDirection * Distance;
}

void UniEngine::LightingManager::SetMainCamera(Entity entity)
{
	_TargetMainCameraEntity = entity;
	_TargetMainCamera = EntityManager::GetSharedComponent<CameraComponent>(entity);
}

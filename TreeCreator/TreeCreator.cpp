// SponzaTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "UniEngine.h"
#include "CameraControlSystem.h"
#include "SpaceColonizationTreeSystem.h"

#include "TreeManager.h"
using namespace UniEngine;
using namespace TreeUtilities;
void InitGround();
int main()
{
	LightingManager::SetAmbientLight(1.0f);
	Engine::Init();
	Engine::SetTimeStep(0.016f);
#pragma region Preparations
	World* world = Engine::GetWorld();
	WorldTime* time = world->Time();
	
	EntityArchetype archetype = EntityManager::CreateEntityArchetype<Translation, Rotation, Scale, LocalToWorld>(Translation(), Rotation(), Scale(), LocalToWorld());
	CameraControlSystem* ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
	ccs->Enable();
	ccs->SetPosition(glm::vec3(0, 30, 60));
	InitGround();
#pragma endregion

	TreeManager::Init();

	auto sctSys = Engine::GetWorld()->CreateSystem<SpaceColonizationTreeSystem>(SystemGroup::SimulationSystemGroup);

	sctSys->ResetEnvelopeType(EnvelopeType::Box);
	sctSys->ResetEnvelope(160, 20, 60);
	sctSys->PushAttractionPoints(10000);

	TreeColor treeColor;
	treeColor.BudColor = glm::vec4(1, 0, 0, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	sctSys->CreateTree(0, treeColor, glm::vec3(30, 0, -30));

	treeColor.BudColor = glm::vec4(0, 1, 0, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	sctSys->CreateTree(1, treeColor, glm::vec3(30, 0, 30));

	treeColor.BudColor = glm::vec4(0, 0, 1, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	sctSys->CreateTree(2, treeColor, glm::vec3(-30, 0, -30));

	treeColor.BudColor = glm::vec4(0, 1, 1, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	sctSys->CreateTree(3, treeColor, glm::vec3(-30, 0, 30));

	sctSys->PushGrowIterations(100);

	Engine::Run();
	Engine::End();
	return 0;
}

void InitGround() {
	EntityArchetype archetype = EntityManager::CreateEntityArchetype<Translation, Rotation, Scale, LocalToWorld>(Translation(), Rotation(), Scale(), LocalToWorld());
	auto entity = EntityManager::CreateEntity(archetype);
	Translation translation = Translation();
	translation.Value = glm::vec3(0.0f, 0.0f, 0.0f);
	Scale scale = Scale();
	scale.Value = glm::vec3(100.0f);
	EntityManager::SetComponentData<Translation>(entity, translation);
	EntityManager::SetComponentData<Scale>(entity, scale);
	/*
	auto entity1 = EntityManager::CreateEntity(archetype);
	translation.value = glm::vec3(-100.0f, 0.0f, 0.0f);
	scale.value = glm::vec3(100.0f, 1.0f, 20.0f);
	Rotation rotation;
	rotation.value = glm::quatLookAt(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
	EntityManager::SetComponentData<Position>(entity1, translation);
	EntityManager::SetComponentData<Scale>(entity1, scale);
	EntityManager::SetComponentData<Rotation>(entity1, rotation);

	auto entity2 = EntityManager::CreateEntity(archetype);
	translation.value = glm::vec3(100.0f, 0.0f, 0.0f);
	scale.value = glm::vec3(100.0f, 1.0f, 20.0f);
	rotation.value = glm::quatLookAt(glm::vec3(0, 1, 0), glm::vec3(-1, 0, 0));

	EntityManager::SetComponentData<Position>(entity2, translation);
	EntityManager::SetComponentData<Scale>(entity2, scale);
	EntityManager::SetComponentData<Rotation>(entity2, rotation);


	auto entity3 = EntityManager::CreateEntity(archetype);
	translation.value = glm::vec3(0.0f, 0.0f, -100.0f);
	scale.value = glm::vec3(100.0f, 1.0f, 20.0f);
	rotation.value = glm::quatLookAt(glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));

	EntityManager::SetComponentData<Position>(entity3, translation);
	EntityManager::SetComponentData<Scale>(entity3, scale);
	EntityManager::SetComponentData<Rotation>(entity3, rotation);

	auto entity4 = EntityManager::CreateEntity(archetype);
	translation.value = glm::vec3(0.0f, 0.0f, 100.0f);
	scale.value = glm::vec3(100.0f, 1.0f, 20.0f);
	rotation.value = glm::quatLookAt(glm::vec3(0, 1, 0), glm::vec3(0, 0, -1));

	EntityManager::SetComponentData<Position>(entity4, translation);
	EntityManager::SetComponentData<Scale>(entity4, scale);
	EntityManager::SetComponentData<Rotation>(entity4, rotation);
	*/

	auto mat = new Material();
	mat->Programs()->push_back(Default::GLPrograms::StandardProgram);
	auto texture = new Texture2D(TextureType::DIFFUSE);
	texture->LoadTexture(FileIO::GetPath("Textures/white.png"), "");
	mat->Textures2Ds()->push_back(texture);
	mat->SetMaterialProperty("material.shininess", 32.0f);
	MeshMaterialComponent* meshMaterial = new MeshMaterialComponent();
	meshMaterial->_Mesh = Default::Primitives::Quad;
	meshMaterial->_Material = mat;
	EntityManager::SetSharedComponent<MeshMaterialComponent>(entity, meshMaterial);
	/*
	EntityManager::SetSharedComponent<MeshMaterialComponent>(entity1, meshMaterial);
	EntityManager::SetSharedComponent<MeshMaterialComponent>(entity2, meshMaterial);
	EntityManager::SetSharedComponent<MeshMaterialComponent>(entity3, meshMaterial);
	EntityManager::SetSharedComponent<MeshMaterialComponent>(entity4, meshMaterial);*/
}

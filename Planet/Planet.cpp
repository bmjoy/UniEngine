// Planet.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "UniEngine.h"
#include "CameraControlSystem.h"
#include "PlanetTerrainSystem.h"

using namespace UniEngine;
using namespace Planet;
int main()
{
	FileIO::SetResourcePath("../Resources/");
	Application::Init();

#pragma region Preparations
	World* world = Application::GetWorld();
	WorldTime* time = world->Time();
	EntityArchetype archetype = EntityManager::CreateEntityArchetype("General", Translation(), Rotation(), Scale(), LocalToWorld());

	
	CameraControlSystem* ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
	ccs->SetSensitivity(0.1f);
	ccs->SetVelocity(15.0f);
	ccs->Enable();

	PlanetTerrainSystem* pts = world->CreateSystem<PlanetTerrainSystem>(SystemGroup::SimulationSystemGroup);
	pts->Enable();

	PlanetInfo pi;
	pi.Position = glm::dvec3(0.0f, 0.0f, 0.0f);
	pi.Rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
	pi.MaxLodLevel = 8;
	pi.LodDistance = 2.0f;
	pi.Radius = 10.0;
	pi.Index = 0;
	pi.Resolution = 64;
	pts->CreatePlanet(pi);

	pi.Position = glm::dvec3(35.0f, 0.0f, 0.0f);
	pi.MaxLodLevel = 20;
	pi.LodDistance = 2.0f;
	pi.Radius = 15.0;
	pi.Index = 1;
	pts->CreatePlanet(pi);

	pi.Position = glm::dvec3(-20.0f, 0.0f, 0.0f);
	pi.MaxLodLevel = 4;
	pi.LodDistance = 2.0f;
	pi.Radius = 5.0;
	pi.Index = 2;
	pts->CreatePlanet(pi);

#pragma endregion

#pragma region Lights
	EntityArchetype dlarc = EntityManager::CreateEntityArchetype("Directional Light", Translation(), Rotation(), Scale(), LocalToWorld(), DirectionalLightComponent());
	EntityArchetype plarc = EntityManager::CreateEntityArchetype("Point Light", Translation(), Rotation(), Scale(), LocalToWorld(), PointLightComponent());
	auto sharedMat = std::make_shared<Material>();
	sharedMat->SetProgram(Default::GLPrograms::DeferredPrepass);
	sharedMat->SetTexture(Default::Textures::StandardTexture, TextureType::DIFFUSE);
	
	MeshRenderer* dlmmc = new MeshRenderer();
	dlmmc->Mesh = Default::Primitives::Cylinder;
	dlmmc->Material = sharedMat;
	Scale scale;
	scale.Value = glm::vec3(0.5f);

	DirectionalLightComponent dlc;
	dlc.diffuse = glm::vec3(1.0f);
	dlc.specular = glm::vec3(0.5f);
	Entity dle = EntityManager::CreateEntity(dlarc);
	EntityManager::SetComponentData<DirectionalLightComponent>(dle, dlc);
	EntityManager::SetComponentData<Scale>(dle, scale);
	EntityManager::SetSharedComponent<MeshRenderer>(dle, std::shared_ptr<MeshRenderer>(dlmmc));

	MeshRenderer* plmmc = new MeshRenderer();
	plmmc->Mesh = Default::Primitives::Sphere;
	plmmc->Material = sharedMat;
	scale.Value = glm::vec3(0.5f);

	PointLightComponent plc;
	plc.constant = 1.0f;
	plc.linear = 0.09f;
	plc.quadratic = 0.032f;
	plc.farPlane = 70.0f;
	plc.diffuse = glm::vec3(2.0f);
	plc.specular = glm::vec3(5.0f);
	Entity ple = EntityManager::CreateEntity(plarc);
	EntityManager::SetComponentData<PointLightComponent>(ple, plc);
	EntityManager::SetComponentData<Scale>(ple, scale);
	EntityManager::SetSharedComponent<MeshRenderer>(ple, std::shared_ptr<MeshRenderer>(plmmc));

	plc.constant = 1.0f;
	plc.linear = 0.09f;
	plc.quadratic = 0.032f;
	plc.farPlane = 70.0f;
	plc.diffuse = glm::vec3(2.0f);
	plc.specular = glm::vec3(5.0f);
	Entity ple2 = EntityManager::CreateEntity(plarc);
	EntityManager::SetComponentData<PointLightComponent>(ple2, plc);
	EntityManager::SetComponentData<Scale>(ple, scale);
	EntityManager::SetSharedComponent<MeshRenderer>(ple2, std::shared_ptr<MeshRenderer>(plmmc));
#pragma endregion

#pragma region EngineLoop
	bool loopable = true;
	

	bool wireFrame = false;
	while (loopable) {
		Application::PreUpdate();
		static bool show = true;
#pragma region LightsPosition
		Translation p;
		p.Value = glm::vec4(glm::vec3(0.0f, 20.0f * glm::sin(time->Time() / 2.0f), -20.0f * glm::cos(time->Time() / 2.0f)), 0.0f);
		EntityManager::SetComponentData<Translation>(dle, p);
		p.Value = glm::vec4(glm::vec3(-20.0f * glm::cos(time->Time() / 2.0f), 20.0f * glm::sin(time->Time() / 2.0f), 0.0f), 0.0f);
		EntityManager::SetComponentData<Translation>(ple, p);
		p.Value = glm::vec4(glm::vec3(20.0f * glm::cos(time->Time() / 2.0f), 15.0f, 20.0f * glm::sin(time->Time() / 2.0f)), 0.0f);
		EntityManager::SetComponentData<Translation>(ple2, p);
#pragma endregion

		Application::Update();
		loopable = Application::LateUpdate();
	}
	Application::End();
#pragma endregion
	return 0;
}

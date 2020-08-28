// SponzaTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "UniEngine.h"
#include "CameraControlSystem.h"
#include "SpaceColonizationTreeSystem.h"
<<<<<<< Updated upstream
=======
#include "PlantSimulationSystem.h"
#include "TreeManager.h"
#include "EntityEditorSystem.h"
#include "NetworkSystem.h"
//#include "EntityEditorSystem.h"
>>>>>>> Stashed changes

#include "TreeManager.h"
//#include "EntityEditorSystem.h"
#include <stdlib.h>
#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
using namespace UniEngine;
using namespace TreeUtilities;
void InitGround();
void InitSpaceColonizationTreeSystem();
<<<<<<< Updated upstream
int startserver();
=======
void InitPlantSimulationSystem();
void InitNetworkSystem();


>>>>>>> Stashed changes
int main()
{
	startserver();
	/*LightingManager::SetAmbientLight(1.0f);
	Application::Init();

	Application::SetTimeStep(0.016f);
#pragma region Preparations
	World* world = Application::GetWorld();
	WorldTime* time = world->Time();
	EntityEditorSystem* editorSystem = world->CreateSystem<EntityEditorSystem>(SystemGroup::PresentationSystemGroup);

	EntityArchetype archetype = EntityManager::CreateEntityArchetype("General", Translation(), Rotation(), Scale(), LocalToWorld());
	CameraControlSystem* ccs = world->CreateSystem<CameraControlSystem>(SystemGroup::SimulationSystemGroup);
	ccs->Enable();
	ccs->SetPosition(glm::vec3(0, 30, 60));
	InitGround();
#pragma endregion
	TreeManager::Init();
<<<<<<< Updated upstream
	InitSpaceColonizationTreeSystem();


=======
	//From top
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(0, 1, 0), 10.0f);
	/*
	//45
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(0, 1, 1), 5.0f);
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(0, 1, -1), 5.0f);
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(1, 1, 0), 5.0f);
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(-1, 1, 0), 5.0f);

	//90
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(0, 0, 1), 1.0f);
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(0, 0, -1), 1.0f);
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(1, 0, 1), 1.0f);
	TreeManager::GetLightEstimator()->PushSnapShot(glm::vec3(-1, 0, 0), 1.0f);
	*/
	//InitSpaceColonizationTreeSystem();
	InitPlantSimulationSystem();
	InitNetworkSystem();
	
>>>>>>> Stashed changes

	Application::Run();
	Application::End();*/
	return 0;
}

<<<<<<< Updated upstream
/*void InitGround() {
	EntityArchetype archetype = EntityManager::CreateEntityArchetype("General", Translation(), Rotation(), Scale(), LocalToWorld());
	auto entity = EntityManager::CreateEntity(archetype);
	Translation translation = Translation();
	translation.Value = glm::vec3(0.0f, 0.0f, 0.0f);
	Scale scale = Scale();
	scale.Value = glm::vec3(100.0f);
	EntityManager::SetComponentData<Translation>(entity, translation);
	EntityManager::SetComponentData<Scale>(entity, scale);

=======
}
void InitNetworkSystem() {
	auto psSys = Application::GetWorld()->CreateSystem<NetworkSystem>(SystemGroup::SimulationSystemGroup);
	psSys->FixedUpdate();

}
>>>>>>> Stashed changes

	auto mat = new Material();
	mat->Programs()->push_back(Default::GLPrograms::StandardProgram);
	auto texture = new Texture2D(TextureType::DIFFUSE);
	texture->LoadTexture(FileIO::GetPath("Textures/treesurface.jpg"), "");
	mat->Textures2Ds()->push_back(texture);
	mat->SetMaterialProperty("material.shininess", 32.0f);
	MeshMaterialComponent* meshMaterial = new MeshMaterialComponent();
	meshMaterial->_Mesh = Default::Primitives::Quad;
	meshMaterial->_Material = mat;
	EntityManager::SetSharedComponent<MeshMaterialComponent>(entity, meshMaterial);

}*/

/*void InitSpaceColonizationTreeSystem()
{
	auto sctSys = Application::GetWorld()->CreateSystem<SpaceColonizationTreeSystem>(SystemGroup::SimulationSystemGroup);
	sctSys->ResetEnvelopeType(EnvelopeType::Box);
	sctSys->ResetEnvelope(160, 20, 60);
	sctSys->PushAttractionPoints(10000);

	TreeColor treeColor;
	treeColor.Color = glm::vec4(1, 1, 1, 1);
	treeColor.BudColor = glm::vec4(1, 0, 0, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	treeColor.LeafColor = glm::vec4(0, 1, 0, 1);
	Entity tree1 = sctSys->CreateTree(treeColor, glm::vec3(30, 0, -30));

	treeColor.BudColor = glm::vec4(0, 1, 0, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	Entity tree2 = sctSys->CreateTree(treeColor, glm::vec3(30, 0, 30));

	treeColor.BudColor = glm::vec4(0, 0, 1, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	Entity tree3 = sctSys->CreateTree(treeColor, glm::vec3(-30, 0, -30));

	treeColor.BudColor = glm::vec4(0, 1, 1, 1);
	treeColor.ConnectionColor = glm::vec4(0.6f, 0.3f, 0, 1);
	Entity tree4 = sctSys->CreateTree(treeColor, glm::vec3(-30, 0, 30));

	sctSys->PushGrowAllTreesIterations(100);
}*/


int startserver() {
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	std::cout << "Sever Start" << std::endl;
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		std::cerr << "Can't Initialize winsock! Quitting" << std::endl;
		return -1;
	}
	//int64_t s = socket(address->addr.sa_family, SOCK_DGRAM, IPPROTO_UDP);
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket! Quitting" << std::endl;
		return -2;
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(57000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 
	int off = 0;
	std::cout << "Set Socket opt" << std::endl;
	int result = setsockopt(listening, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&off, sizeof(off));
	if (result != 0)
	{
		//*errorcode = native_get_last_error();
		closesocket(listening);
		return -1;
	}
	std::cout << "Before Binding" << std::endl;
	bind(listening, (sockaddr*)&hint, sizeof(hint));
	listen(listening, SOMAXCONN);


	sockaddr_in client;
	int clientSize = sizeof(client);
	while (true) {
		SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
		char host[NI_MAXHOST];
		char service[NI_MAXSERV];

		ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
		ZeroMemory(service, NI_MAXSERV);

		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			std::cout << host << " connected on port " << service << std::endl;
			std::cout << "this one " << std::endl;
		}
		else
		{
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			std::cout << ntohs(hint.sin_port) << std::endl;
			std::cout << "second one " << std::endl;
			std::cout << host << " connected on port " <<
				ntohs(client.sin_port) << std::endl;
		}

		//closesocket(listening);

		char buf[4096];

		while (true)
		{
			ZeroMemory(buf, 4096);
			int bytesReceived = recv(clientSocket, buf, 4096, 0);
			if (bytesReceived == SOCKET_ERROR)
			{
				std::cerr << "Error in recv(). Quitting" << std::endl;
				break;
			}

			if (bytesReceived == 0)
			{
				std::cout << "Client disconnected " << std::endl;
				break;
			}

			std::cout << std::string(buf, 0, bytesReceived) << std::endl;
			send(clientSocket, buf, bytesReceived + 1, 0);

		}

		closesocket(clientSocket);
	}
	WSACleanup();
	system("pause");
}
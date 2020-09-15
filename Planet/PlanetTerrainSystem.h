#pragma once
#include "SystemBase.h"
#include "PlanetTerrain.h"
using namespace UniEngine;

namespace Planet
{
	class PlanetTerrainSystem :
		public SystemBase
	{
		std::vector<PlanetTerrain*> _PlanetTerrainList;
		std::queue<TerrainChunk*> _GenerationQueue;
		unsigned _MaxRecycledMeshAmount;
	public:
		void OnCreate() override;
		void Update() override;
		void FixedUpdate() override;
		void Remove(std::vector<TerrainChunk*>* list, unsigned index);
		void GenerateTerrain(PlanetTerrain* planetTerrain, TerrainChunk* targetChunk);
		void SetMaxMeshAmount(unsigned amount);
		void CreatePlanet(PlanetInfo info);
	};
}

#include "pch.h"
#include "TransformSystem.h"
#include "EntityCollection.h"
using namespace UniEngine;
glm::mat4 UniEngine::TransformSystem::TRS(glm::vec3 translation, glm::quat rotation, glm::vec3 scale) {
	return glm::scale(glm::translate(glm::mat4_cast(rotation), translation), scale);
}

void UniEngine::TransformSystem::CalculateTransform(Entity* parent)
{
	for (auto i : *(parent->Children())) {
		LocalToWorld ltw = LocalToWorld();
		auto pltw = _EntityCollection->GetFixedData<LocalToWorld>(parent).value;
		auto ltp = _EntityCollection->GetFixedData<LocalToParent>(i).value;
		ltw.value = pltw * ltp;
		_EntityCollection->SetFixedData<LocalToWorld>(i, ltw);
		CalculateTransform(i);
	}
}

UniEngine::TransformSystem::TransformSystem()
{

}

void UniEngine::TransformSystem::OnCreate()
{
	Enable();
}

void UniEngine::TransformSystem::OnDestroy()
{
	Disable();
}



void UniEngine::TransformSystem::Update()
{
	for (auto i : *(_EntityCollection->Entities())) {
		if (i->Parent() != nullptr) {
			LocalToParent ltp = LocalToParent();
			auto lp = _EntityCollection->GetFixedData<LocalPosition>(i).value;
			auto lr = _EntityCollection->GetFixedData<LocalRotation>(i).value;
			auto ls = _EntityCollection->GetFixedData<LocalScale>(i).value;
			ltp.value = TRS(lp, lr, ls);
			_EntityCollection->SetFixedData<LocalToParent>(i, ltp);
		}
		else {
			LocalToWorld ltw = LocalToWorld();
			auto p = _EntityCollection->GetFixedData<Position>(i).value;
			auto r = _EntityCollection->GetFixedData<Rotation>(i).value;
			auto s = _EntityCollection->GetFixedData<Scale>(i).value;
			ltw.value = TRS(p, r, s);
			_EntityCollection->SetFixedData<LocalToWorld>(i, ltw);
		}
	}
	for (auto i : *(_EntityCollection->Entities())) {
		if (i->Parent() == nullptr) {
			CalculateTransform(i);
		}
	}
}

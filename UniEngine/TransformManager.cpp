#include "pch.h"
#include "TransformManager.h"
#include "UniEngine.h"
#include "EditorManager.h"
using namespace UniEngine;
bool TransformManager::_AddCheck;
size_t TransformManager::_CurrentStoredHierarchyVersion = INT_MAX;
EntityQuery TransformManager::_TransformQuery;
std::vector<std::pair<Entity, ChildInfo>> TransformManager::_CachedParentHierarchies;
void UniEngine::TransformManager::Init()
{
	_TransformQuery = EntityManager::CreateEntityQuery();
	EntityManager::SetEntityQueryAllFilters(_TransformQuery, Transform(), GlobalTransform());
	_CachedParentHierarchies = std::vector<std::pair<Entity, ChildInfo>>();
	
}

void UniEngine::TransformManager::LateUpdate()
{
	EntityManager::ForEach<Transform, GlobalTransform>(_TransformQuery, [](int i, Entity entity, Transform* ltp, GlobalTransform* ltw)
		{
			if(EntityManager::GetParent(entity).IsNull())
			{
				ltw->Value = ltp->Value;
				CalculateLtwRecursive(ltw, entity);
			}
		}
	);
	/*
	if (EntityManager::GetParentHierarchyVersion() == _CurrentStoredHierarchyVersion) {
		std::vector<std::shared_future<void>> futures;
		auto list = &_CachedParentHierarchies;
		auto threadSize = _ThreadPool->Size();
		size_t capacity = _CachedParentHierarchies.size() / threadSize;
		size_t reminder = _CachedParentHierarchies.size() % threadSize;
		for (size_t i = 0; i < threadSize; i++) {
			futures.push_back(_ThreadPool->Push([&list, i, capacity](int id)
				{
					for (size_t j = 0; j < capacity; j++) {
						size_t index = capacity * i + j;
						const auto& info = list->at(index);
						auto pltw = EntityManager::GetComponentData<GlobalTransform>(info.first);
						auto ltp = EntityManager::GetComponentData<Transform>(info.second.Child);

						GlobalTransform ltw;
						ltw.Value = pltw.Value * ltp.Value;
						EntityManager::SetComponentData<GlobalTransform>(info.second.Child, ltw);
					}
				}
			).share());
		}
		for (size_t i = 0; i < reminder; i++) {
			size_t index = capacity * threadSize + i;
			const auto& info = list->at(index);
			auto pltw = EntityManager::GetComponentData<GlobalTransform>(info.first);
			auto ltp = EntityManager::GetComponentData<Transform>(info.second.Child);
			GlobalTransform ltw;
			ltw.Value = pltw.Value * ltp.Value;
			EntityManager::SetComponentData<GlobalTransform>(info.second.Child, ltw);
		}
		for (const auto& i : futures) i.wait();
	}
	else {
		_CachedParentHierarchies.clear();
		EntityManager::ForAllRootParent([](int i, Entity rootParent) {
			CollectHierarchy(&_CachedParentHierarchies, rootParent);
			}
		);
		_CurrentStoredHierarchyVersion = EntityManager::GetParentHierarchyVersion();
	}
	*/
	if (!Application::IsPlaying())
	{
		PhysicsSimulationManager::UploadTransforms();
	}
}

void UniEngine::TransformManager::CalculateLtwRecursive(GlobalTransform* pltw, Entity entity)
{
	/*
	Here we have 2 ways to deal with children, you can use lambda function or you can get children
	and maniputale them directly. The method I'm using here, which is the second one,
	is faster and I don't know why...
	*/
	for (const auto& i : EntityManager::GetChildren(entity)) {
		if (!i.HasComponentData<Transform>()) continue;
		auto ltp = EntityManager::GetComponentData<Transform>(i);
		GlobalTransform ltw;
		ltw.Value = pltw->Value * ltp.Value;
		EntityManager::SetComponentData<GlobalTransform>(i, ltw);
		CalculateLtwRecursive(&ltw, i);
	}
}

void UniEngine::TransformManager::CollectHierarchy(std::vector<std::pair<Entity, ChildInfo>>* container, Entity entity)
{
	auto children = EntityManager::GetChildren(entity);
	const auto initialSize = container->size();
	for (const auto& i : children) {
		if (EntityManager::HasComponentData<GlobalTransform>(i) && EntityManager::HasComponentData<Transform>(i) && EntityManager::HasComponentData<GlobalTransform>(entity)) {
			ChildInfo info;
			info.Child = i;
			auto ltp = EntityManager::GetComponentData<Transform>(i);
			auto pltw = EntityManager::GetComponentData<GlobalTransform>(entity);
			container->push_back(std::make_pair(entity, info));
			GlobalTransform ltw;
			ltw.Value = pltw.Value * ltp.Value;
			EntityManager::SetComponentData<GlobalTransform>(i, ltw);
		}
	}
	for (int i = initialSize; i < container->size(); i++) {
		Entity target = container->at(i).second.Child;
		for (const auto& child : EntityManager::GetChildren(target)) {
			if (EntityManager::HasComponentData<GlobalTransform>(child) && EntityManager::HasComponentData<Transform>(child) && EntityManager::HasComponentData<GlobalTransform>(target)) {
				ChildInfo info;
				info.Child = child;
				auto ltp = EntityManager::GetComponentData<Transform>(child);
				auto pltw = EntityManager::GetComponentData<GlobalTransform>(target);
				container->push_back(std::make_pair(target, info));
				GlobalTransform ltw;
				ltw.Value = pltw.Value * ltp.Value;
				EntityManager::SetComponentData<GlobalTransform>(child, ltw);
			}
		}
	}
}
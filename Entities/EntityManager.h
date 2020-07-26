#pragma once
#include "ManagerBase.h"
#include "Entity.h"
#include "SharedComponentStorage.h"
#include "World.h"
namespace UniEngine {
	namespace Entities {
#pragma region EntityManager
		struct ENTITIES_API WorldEntityStorage {
			size_t Index;
			std::vector<Entity> Entities;
			std::vector<EntityInfo> EntityInfos;
			std::vector<std::queue<Entity>> EntityPool;
			std::vector<EntityComponentStorage> EntityComponentStorage;
			SharedComponentStorage EntitySharedComponentStorage;

			std::vector<EntityQuery> EntityQueries;
			std::vector<EntityQueryInfo> EntityQueryInfos;
			std::queue<EntityQuery> EntityQueryPools;
		};
		class ENTITIES_API EntityManager : public ManagerBase {
			static std::vector<WorldEntityStorage*> _WorldEntityStorage;
			static std::vector<Entity>* _Entities;
			static std::vector<EntityInfo>* _EntityInfos;
			static std::vector<EntityComponentStorage>* _EntityComponentStorage;
			static std::vector<std::queue<Entity>>* _EntityPool;
			static SharedComponentStorage* _EntitySharedComponentStorage;
			static std::vector<EntityQuery>* _EntityQueries;
			static std::vector<EntityQueryInfo>* _EntityQueryInfos;
			static std::queue<EntityQuery>* _EntityQueryPools;

			template<typename T>
			static size_t CollectComponentTypes(std::vector<ComponentType>* componentTypes, T arg);
			template<typename T, typename... Ts>
			static size_t CollectComponentTypes(std::vector<ComponentType>* componentTypes, T arg, Ts... args);
			template<typename T, typename... Ts>
			static std::vector<ComponentType> CollectComponentTypes(T arg, Ts... args);
			static void DeleteEntityInternal(Entity entity);

			static void RefreshEntityQueryInfos(size_t index);

		public:
			static void GetAllEntities(std::vector<Entity>* target);
			static std::vector<Entity>* GetAllEntitiesUnsafe();
			static void SetWorld(World* world);
			template<typename T, typename... Ts>
			static EntityArchetype CreateEntityArchetype(T arg, Ts... args);

			static Entity CreateEntity(EntityArchetype archetype);
			static void DeleteEntity(Entity entity);
			
			static void SetParent(Entity entity, Entity parent);
			static Entity GetParent(Entity entity);
			static std::vector<Entity> GetChildren(Entity entity);
			static void RemoveChild(Entity entity, Entity parent);



			template<typename T>
			static void SetComponentData(Entity entity, T value);
			template<typename T>
			static T GetComponentData(Entity entity);
			template<typename T>
			static bool HasComponentData(Entity entity);

			template <typename T>
			static T* GetSharedComponent(Entity entity);
			template <typename T>
			static void SetSharedComponent(Entity entity, T* value);
			template <typename T>
			static bool RemoveSharedComponent(Entity entity);
			template <typename T>
			static bool HasSharedComponent(Entity entity);


			template <typename T>
			static std::vector<Entity>* QueryEntities(T* value);
			template <typename T>
			static std::vector<T*>* QuerySharedComponents();

			static EntityArchetype GetEntityArchetype(Entity entity);

			static EntityQuery CreateEntityQuery();
			static void DeleteEntityQuery(EntityQuery entityQuery);
			template<typename T, typename... Ts>
			static void SetEntityQueryAllFilters(EntityQuery entityQuery, T arg, Ts... args);
			template<typename T, typename... Ts>
			static void SetEntityQueryAnyFilters(EntityQuery entityQuery, T arg, Ts... args);
			template<typename T, typename... Ts>
			static void SetEntityQueryNoneFilters(EntityQuery entityQuery, T arg, Ts... args);
			//Unsafe zone, allow directly manipulation of entity data, which may result in data corruption.
			static std::vector<EntityComponentStorage> UnsafeQueryStorages(EntityQuery entityQuery);
			static ComponentDataChunkArray* UnsafeGetEntityComponentDataChunkArray(EntityArchetype entityArchetype);
		};
#pragma endregion
#pragma region Functions
		template<typename T>
		inline size_t EntityManager::CollectComponentTypes(std::vector<ComponentType>* componentTypes, T arg)
		{
			ComponentType type = typeof<T>();
			componentTypes->push_back(type);
			return type.Size;
		}

		template<typename T, typename ...Ts>
		inline size_t EntityManager::CollectComponentTypes(std::vector<ComponentType>* componentTypes, T arg, Ts ...args)
		{
			auto offset = CollectComponentTypes(componentTypes, args...);
			ComponentType type = typeof<T>();
			componentTypes->push_back(type);
			return type.Size + offset;
		}

		template<typename T, typename ...Ts>
		inline std::vector<ComponentType> EntityManager::CollectComponentTypes(T arg, Ts ...args)
		{
			std::vector<ComponentType> retVal = std::vector<ComponentType>();
			CollectComponentTypes(&retVal, arg, args...);
			std::sort(retVal.begin(), retVal.end(), ComponentTypeComparator);
			size_t offset = 0;
			ComponentType prev = retVal[0];
			//Erase duplicates
			for (int i = 1; i < retVal.size(); i++) {
				if (retVal[i] == prev) {
					retVal.erase(retVal.begin() + i);
					i--;
				}
				else {
					prev = retVal[i];
				}
			}
			for (int i = 0; i < retVal.size(); i++) {
				retVal[i].Offset = offset;
				offset += retVal[i].Size;
			}
			return retVal;
		}

		template<typename T, typename ...Ts>
		inline EntityArchetype EntityManager::CreateEntityArchetype(T arg, Ts ...args)
		{
			EntityArchetypeInfo* info = new EntityArchetypeInfo();
			info->EntityCount = 0;
			info->ComponentTypes = CollectComponentTypes(arg, args...);
			info->EntitySize = info->ComponentTypes.back().Offset + info->ComponentTypes.back().Size;
			info->ChunkCapacity = ARCHETYPECHUNK_SIZE / info->EntitySize;
			int duplicateIndex = -1;
			for (auto i = 1; i < _EntityComponentStorage->size(); i++) {
				EntityArchetypeInfo* compareInfo = _EntityComponentStorage->at(i).ArchetypeInfo;
				if (info->ChunkCapacity != compareInfo->ChunkCapacity) continue;
				if (info->EntitySize != compareInfo->EntitySize) continue;
				bool typeCheck = true;
				for (auto j = 0; j < compareInfo->ComponentTypes.size(); j++) {
					if (info->ComponentTypes[j] != compareInfo->ComponentTypes[j]) typeCheck = false;
				}
				if (typeCheck) {
					duplicateIndex = compareInfo->Index;
					break;
				}
			}
			EntityArchetype retVal;
			if (duplicateIndex == -1) {
				retVal.Index = _EntityComponentStorage->size();
				info->Index = retVal.Index;
				_EntityComponentStorage->push_back(EntityComponentStorage(info, new ComponentDataChunkArray()));
				_EntityPool->push_back(std::queue<Entity>());
			}
			else {
				retVal.Index = duplicateIndex;
			}
			for (int i = 0; i < _EntityQueryInfos->size(); i++) {
				RefreshEntityQueryInfos(i);
			}
			return retVal;
		}


		template<typename T>
		inline void EntityManager::SetComponentData(Entity entity, T value)
		{
			if (entity.IsNull()) return;
			EntityInfo info;
			info = _EntityInfos->at(entity.Index);
			
			if (_Entities->at(entity.Index) == entity) {
				EntityArchetypeInfo* chunkInfo = _EntityComponentStorage->at(info.ArchetypeInfoIndex).ArchetypeInfo;
				unsigned chunkIndex = info.ChunkArrayIndex / chunkInfo->ChunkCapacity;
				unsigned chunkPointer = info.ChunkArrayIndex % chunkInfo->ChunkCapacity;
				ComponentDataChunk chunk = _EntityComponentStorage->at(info.ArchetypeInfoIndex).ChunkArray->Chunks[chunkIndex];
				unsigned offset = 0;
				bool found = false;
				size_t id = typeid(T).hash_code();
				for (int i = 0; i < chunkInfo->ComponentTypes.size(); i++) {
					if (id == chunkInfo->ComponentTypes[i].TypeID) {
						offset += chunkInfo->ComponentTypes[i].Offset * chunkInfo->ChunkCapacity;
						offset += chunkPointer * chunkInfo->ComponentTypes[i].Size;
						found = true;
						break;
					}
				}
				if (found) {
					chunk.SetData<T>(offset, value);
				}
				else {
					Debug::Log("ComponentData doesn't exist");
					return;
				}
			}
			else {
				Debug::Error("Entity already deleted!");
				return;
			}
		}
		template<typename T>
		inline T EntityManager::GetComponentData(Entity entity)
		{
			if (entity.IsNull()) return T();
			EntityInfo info = _EntityInfos->at(entity.Index);
			if (_Entities->at(entity.Index) == entity) {
				EntityArchetypeInfo* chunkInfo = _EntityComponentStorage->at(info.ArchetypeInfoIndex).ArchetypeInfo;
				unsigned chunkIndex = info.ChunkArrayIndex / chunkInfo->ChunkCapacity;
				unsigned chunkPointer = info.ChunkArrayIndex % chunkInfo->ChunkCapacity;
				ComponentDataChunk chunk = _EntityComponentStorage->at(info.ArchetypeInfoIndex).ChunkArray->Chunks[chunkIndex];
				unsigned offset = 0;
				bool found = false;
				size_t id = typeid(T).hash_code();
				for (int i = 0; i < chunkInfo->ComponentTypes.size(); i++) {
					if (id == chunkInfo->ComponentTypes[i].TypeID) {
						offset += chunkInfo->ComponentTypes[i].Offset * chunkInfo->ChunkCapacity;
						offset += chunkPointer * chunkInfo->ComponentTypes[i].Size;
						found = true;
						break;
					}
				}
				if (found) {
					return chunk.GetData<T>(offset);
				}
				else {
					Debug::Log("ComponentData doesn't exist");
					return T();
				}
			}
			else {
				Debug::Error("Entity already deleted!");
				return T();
			}
		}
		template<typename T>
		inline bool EntityManager::HasComponentData(Entity entity)
		{
			if (entity.IsNull()) return false;
			EntityInfo info = _EntityInfos->at(entity.Index);
			if (_Entities->at(entity.Index) == entity) {
				EntityArchetypeInfo* chunkInfo = _EntityComponentStorage->at(info.ArchetypeInfoIndex).ArchetypeInfo;
				unsigned chunkIndex = info.ChunkArrayIndex / chunkInfo->ChunkCapacity;
				unsigned chunkPointer = info.ChunkArrayIndex % chunkInfo->ChunkCapacity;
				ComponentDataChunk chunk = _EntityComponentStorage->at(info.ArchetypeInfoIndex).ChunkArray->Chunks[chunkIndex];
				unsigned offset = 0;
				bool found = false;
				size_t id = typeid(T).hash_code();
				for (int i = 0; i < chunkInfo->ComponentTypes.size(); i++) {
					if (id == chunkInfo->ComponentTypes[i].TypeID) {
						offset += chunkInfo->ComponentTypes[i].Offset * chunkInfo->ChunkCapacity;
						offset += chunkPointer * chunkInfo->ComponentTypes[i].Size;
						found = true;
						break;
					}
				}
				if (found) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				Debug::Error("Entity already deleted!");
				return false;
			}
		}
		template<typename T>
		inline T* EntityManager::GetSharedComponent(Entity entity)
		{
			if (entity.IsNull()) return nullptr;
			return _EntitySharedComponentStorage->GetSharedComponent<T>(entity);
		}
		template<typename T>
		inline void EntityManager::SetSharedComponent(Entity entity, T* value)
		{
			if (entity.IsNull()) return;
			_EntitySharedComponentStorage->SetSharedComponent<T>(entity, value);
		}
		template<typename T>
		inline bool EntityManager::RemoveSharedComponent(Entity entity)
		{
			if (entity.IsNull()) return false;
			return _EntitySharedComponentStorage->RemoveSharedComponent<T>(entity);
		}
		template<typename T>
		inline bool EntityManager::HasSharedComponent(Entity entity)
		{
			if (entity.IsNull()) return nullptr;
			return _EntitySharedComponentStorage->GetSharedComponent<T>(entity) != nullptr;
		}
		template<typename T>
		inline std::vector<Entity>* EntityManager::QueryEntities(T* value)
		{
			return _EntitySharedComponentStorage->GetOwnersList<T>(value);
		}
		template<typename T>
		inline std::vector<T*>* EntityManager::QuerySharedComponents()
		{
			return _EntitySharedComponentStorage->GetSCList<T>();
		}

		template<typename T, typename ...Ts>
		inline void EntityManager::SetEntityQueryAllFilters(EntityQuery entityQuery, T arg, Ts ...args)
		{
			if (entityQuery.IsNull()) return;
			unsigned index = entityQuery.Index;
			if (_EntityQueries->at(index).IsDeleted()) {
				Debug::Error("EntityQuery already deleted!");
				return;
			}
			if (_EntityQueries->at(index) != entityQuery) {
				Debug::Error("EntityQuery out of date!");
				return;
			}
			_EntityQueryInfos->at(index).AllComponentTypes = CollectComponentTypes(arg, args...);
			RefreshEntityQueryInfos(index);
		}

		template<typename T, typename ...Ts>
		inline void EntityManager::SetEntityQueryAnyFilters(EntityQuery entityQuery, T arg, Ts ...args)
		{
			if (entityQuery.IsNull()) return;
			unsigned index = entityQuery.Index;
			if (_EntityQueries->at(index).IsDeleted()) {
				Debug::Error("EntityQuery already deleted!");
				return;
			}
			if (_EntityQueries->at(index) != entityQuery) {
				Debug::Error("EntityQuery out of date!");
				return;
			}
			_EntityQueryInfos->at(index).AnyComponentTypes = CollectComponentTypes(arg, args...);
			RefreshEntityQueryInfos(index);
		}

		template<typename T, typename ...Ts>
		inline void EntityManager::SetEntityQueryNoneFilters(EntityQuery entityQuery, T arg, Ts ...args)
		{
			if (entityQuery.IsNull()) return;
			unsigned index = entityQuery.Index;
			if (_EntityQueries->at(index).IsDeleted()) {
				Debug::Error("EntityQuery already deleted!");
				return;
			}
			if (_EntityQueries->at(index) != entityQuery) {
				Debug::Error("EntityQuery out of date!");
				return;
			}
			_EntityQueryInfos->at(index).NoneComponentTypes = CollectComponentTypes(arg, args...);
			RefreshEntityQueryInfos(index);
		}

#pragma endregion
	}
}
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

			template<typename T1>
			static void ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*)>& func);
			template<typename T1, typename T2>
			static void ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*, T2*)>& func);
			template<typename T1, typename T2, typename T3>
			static void ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*, T2*, T3*)>& func);
			template<typename T1, typename T2, typename T3, typename T4>
			static void ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*, T2*, T3*, T4*)>& func);
		
			template<typename T>
			static void GetComponentDataArrayStorage(EntityComponentStorage storage, std::vector<T>* container);
		
			static size_t SwapEntity(EntityComponentStorage storage, size_t index1, size_t index2);
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

			template<typename T1>
			static void ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*)>& func);
			template<typename T1, typename T2>
			static void ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*, T2*)>& func);
			template<typename T1, typename T2, typename T3>
			static void ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*, T2*, T3*)>& func);
			template<typename T1, typename T2, typename T3, typename T4>
			static void ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*, T2*, T3*, T4*)>& func);
			template<typename T>
			static void GetComponentDataArray(EntityQuery entityQuery, std::vector<T>* container);

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

		template<typename T1>
		inline void EntityManager::ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*)>& func)
		{
			ComponentType targetType = typeof<T1>();
			size_t entityCount = storage.ArchetypeInfo->EntityCount;
			bool found = false;
			for (auto type : storage.ArchetypeInfo->ComponentTypes) {
				if (targetType == type) {
					targetType = type;
					found = true;
					break;
				}
			}
			if (found) {
				size_t chunkIndex = 0;
				size_t capacity = storage.ArchetypeInfo->ChunkCapacity;
				omp_set_num_threads(OMP_THREAD_AMOUNT);
#pragma omp parallel for
				for (int i = 0; i < entityCount; i++) {
					chunkIndex = i / capacity;
					if (storage.ChunkArray->Entities[i].Version != 0) {
						func(i, 
							(T1*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType.Offset * capacity + (i % capacity) * targetType.Size));
					}
				}
			}
		}

		template<typename T1, typename T2>
		inline void EntityManager::ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*, T2*)>& func)
		{
			ComponentType targetType1 = typeof<T1>();
			ComponentType targetType2 = typeof<T2>();
			size_t entityCount = storage.ArchetypeInfo->EntityCount;
			bool found1 = false;
			bool found2 = false;
			for (auto type : storage.ArchetypeInfo->ComponentTypes) {
				if (targetType1 == type) {
					targetType1 = type;
					found1 = true;
				}
				else if (targetType2 == type) {
					targetType2 = type;
					found2 = true;
				}
			}
			omp_set_num_threads(OMP_THREAD_AMOUNT);
			if (found1 && found2) {
				size_t chunkIndex = 0;
				size_t capacity = storage.ArchetypeInfo->ChunkCapacity;
//#pragma omp parallel for
				for (int i = 0; i < entityCount; i++) {
					chunkIndex = i / capacity;
					if (storage.ChunkArray->Entities[i].Version != 0) {
						func(i,
							(T1*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType1.Offset * capacity + (i % capacity) * targetType1.Size), 
							(T2*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType2.Offset * capacity + (i % capacity) * targetType2.Size)
						);
					}
				}
			}
		}

		template<typename T1, typename T2, typename T3>
		static void EntityManager::ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*, T2*, T3*)>& func) {
			ComponentType targetType1 = typeof<T1>();
			ComponentType targetType2 = typeof<T2>();
			ComponentType targetType3 = typeof<T3>();
			size_t entityCount = storage.ArchetypeInfo->EntityCount;
			bool found1 = false;
			bool found2 = false;
			bool found3 = false;
			for (auto type : storage.ArchetypeInfo->ComponentTypes) {
				if (targetType1 == type) {
					targetType1 = type;
					found1 = true;
				}
				else if (targetType2 == type) {
					targetType2 = type;
					found2 = true;
				}
				else if (targetType3 == type) {
					targetType3 = type;
					found3 = true;
				}
			}
			omp_set_num_threads(OMP_THREAD_AMOUNT);
			if (found1 && found2 && found3) {
				size_t chunkIndex = 0;
				size_t capacity = storage.ArchetypeInfo->ChunkCapacity;
#pragma omp parallel for
				for (int i = 0; i < entityCount; i++) {
					chunkIndex = i / capacity;
					if (storage.ChunkArray->Entities[i].Version != 0) {
						func(i,
							(T1*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType1.Offset * capacity + (i % capacity) * targetType1.Size), 
							(T2*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType2.Offset * capacity + (i % capacity) * targetType2.Size), 
							(T3*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType3.Offset * capacity + (i % capacity) * targetType3.Size)
						);
					}
				}
			}
		}
		template<typename T1, typename T2, typename T3, typename T4>
		static void EntityManager::ForEachStorage(EntityComponentStorage storage, const std::function<void(int i, T1*, T2*, T3*, T4*)>& func) {
			ComponentType targetType1 = typeof<T1>();
			ComponentType targetType2 = typeof<T2>();
			ComponentType targetType3 = typeof<T3>();
			ComponentType targetType4 = typeof<T4>();
			size_t entityCount = storage.ArchetypeInfo->EntityCount;
			bool found1 = false;
			bool found2 = false;
			bool found3 = false;
			bool found4 = false;
			for (auto type : storage.ArchetypeInfo->ComponentTypes) {
				if (targetType1 == type) {
					targetType1 = type;
					found1 = true;
				}
				else if (targetType2 == type) {
					targetType2 = type;
					found2 = true;
				}
				else if (targetType3 == type) {
					targetType3 = type;
					found3 = true;
				}
				else if (targetType4 == type) {
					targetType4 = type;
					found4 = true;
				}
			}
			omp_set_num_threads(OMP_THREAD_AMOUNT);
			if (found1 && found2 && found3 && found4) {
				size_t chunkIndex = 0;
				size_t capacity = storage.ArchetypeInfo->ChunkCapacity;
#pragma omp parallel for
				for (int i = 0; i < entityCount; i++) {
					chunkIndex = i / capacity;
					if (storage.ChunkArray->Entities[i].Version != 0) {
						func(i,
							(T1*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType1.Offset * capacity + (i % capacity) * targetType1.Size), 
							(T2*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType2.Offset * capacity + (i % capacity) * targetType2.Size), 
							(T3*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType3.Offset * capacity + (i % capacity) * targetType3.Size), 
							(T4*)((char*)storage.ChunkArray->Chunks[chunkIndex].Data + targetType4.Offset * capacity + (i % capacity) * targetType4.Size)
						);
					}
				}
			}
		}

		template<typename T>
		inline void EntityManager::GetComponentDataArrayStorage(EntityComponentStorage storage, std::vector<T>* container)
		{
			ComponentType targetType = typeof<T>();
			size_t entityCount = storage.ArchetypeInfo->EntityCount;
			bool found = false;
			for (auto type : storage.ArchetypeInfo->ComponentTypes) {
				if (targetType == type) {
					targetType = type;
					found = true;
					break;
				}
			}
			if (found) {
				size_t amount = storage.ArchetypeInfo->EntityAliveCount;
				container->resize(container->size() + amount);
				size_t capacity = storage.ArchetypeInfo->ChunkCapacity;
				size_t chunkAmount = amount / capacity;
				size_t remainAmount = amount % capacity;
				for (int i = 0; i < chunkAmount; i++) {
					memcpy(&container->at(container->size() - remainAmount - capacity * (chunkAmount - i)), (void*)((char*)storage.ChunkArray->Chunks[i].Data + capacity * targetType.Offset), capacity * targetType.Size);
				}
				if (remainAmount > 0) memcpy(&container->at(container->size() - remainAmount), (void*)((char*)storage.ChunkArray->Chunks[chunkAmount].Data + capacity * targetType.Offset), remainAmount * targetType.Size);
			}
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

		template<typename T1>
		inline void EntityManager::ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*)>& func)
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
			for (auto i : _EntityQueryInfos->at(index).QueriedStorages) {
				ForEachStorage(i, func);
			}
		}
		template<typename T1, typename T2>
		static void EntityManager::ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*, T2*)>& func) {
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
			for (auto i : _EntityQueryInfos->at(index).QueriedStorages) {
				ForEachStorage(i, func);
			}
		}
		template<typename T1, typename T2, typename T3>
		static void EntityManager::ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*, T2*, T3*)>& func) {
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
			for (auto i : _EntityQueryInfos->at(index).QueriedStorages) {
				ForEachStorage(i, func);
			}
		}
		template<typename T1, typename T2, typename T3, typename T4>
		static void EntityManager::ForEach(EntityQuery entityQuery, const std::function<void(int i, T1*, T2*, T3*, T4*)>& func) {
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
			for (auto i : _EntityQueryInfos->at(index).QueriedStorages) {
				ForEachStorage(i, func);
			}
		}
		template<typename T>
		inline void EntityManager::GetComponentDataArray(EntityQuery entityQuery, std::vector<T>* container)
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
			for (auto i : _EntityQueryInfos->at(index).QueriedStorages) {
				GetComponentDataArrayStorage(i, container);
			}
		}
#pragma endregion
	}
}
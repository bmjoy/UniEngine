#include "pch.h"
#include "TreeManager.h"

using namespace TreeUtilities;
TreeSystem* TreeUtilities::TreeManager::_TreeSystem;
BudSystem* TreeUtilities::TreeManager::_BudSystem;
LeafSystem* TreeUtilities::TreeManager::_LeafSystem;
EntityArchetype TreeUtilities::TreeManager::_BudArchetype;
EntityArchetype TreeUtilities::TreeManager::_LeafArchetype;
EntityArchetype TreeUtilities::TreeManager::_TreeArchetype;
EntityQuery TreeUtilities::TreeManager::_TreeQuery;
EntityQuery TreeUtilities::TreeManager::_BudQuery;
EntityQuery TreeUtilities::TreeManager::_LeafQuery;

TreeIndex TreeUtilities::TreeManager::_TreeIndex;
BudIndex TreeUtilities::TreeManager::_BudIndex;
LeafIndex TreeUtilities::TreeManager::_LeafIndex;

bool TreeUtilities::TreeManager::_Ready;
#pragma region Helpers
inline void TreeUtilities::TreeManager::LeafGenerationHelper(LeafInfo info, Entity leaf, Entity bud, int index)
{
    EntityManager::SetParent(leaf, bud);
    EntityManager::SetComponentData(leaf, info);
    EntityManager::SetComponentData(leaf, EntityManager::GetComponentData<BudIndex>(bud));
    LocalTranslation lt;
    LocalScale ls;
    LocalRotation lr;
    lt.value = glm::vec3(0);
    ls.value = glm::vec3(0.4f);

    auto pt = EntityManager::GetComponentData<ParentTranslation>(bud);
    auto ltw = EntityManager::GetComponentData<LocalToWorld>(bud);
    glm::vec3 diff = glm::normalize(glm::vec3(ltw.value[3]) - pt.Value);
    glm::quat right = glm::quatLookAt(glm::cross(diff, glm::vec3(0, 1, 0)), diff);
    right = glm::rotate(right, glm::radians(info.CircleDegreeAddition * index), diff);
    lr.value = glm::quatLookAt(glm::vec3(0, 1, 0), diff);
    lr.value = glm::rotate(lr.value, glm::radians(info.BendDegrees), right * glm::vec3(0, 0, 1));
    EntityManager::SetComponentData(leaf, lt);
    EntityManager::SetComponentData(leaf, ls);
    EntityManager::SetComponentData(leaf, lr);
}
void TreeUtilities::TreeManager::Init()
{
    _LeafArchetype = EntityManager::CreateEntityArchetype(
        LocalTranslation(), LocalRotation(), LocalScale(), LocalToParent(), LocalToWorld(),
        Mass(), Position(), Direction(), ParentTranslation(), Connection(), TreeIndex(), BudIndex(),
        LeafIndex(), LeafInfo(),
        Phototropism(), Gravitropism()
    );
    _BudArchetype = EntityManager::CreateEntityArchetype(
        LocalTranslation(), LocalRotation(), LocalScale(), LocalToParent(), LocalToWorld(),
        Mass(), Position(), Direction(), ParentTranslation(), Connection(),
        Radius(), BudIndex(), Iteration(), Level(), BudType(), TreeIndex(),
        Phototropism(), Gravitropism()
    );
    _TreeArchetype = EntityManager::CreateEntityArchetype(
        Translation(), Rotation(), Scale(), LocalToWorld(),
        TreeIndex(), TreeType(), TreeColor(), TreeGrowIteration()
    );

    _LeafQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(_LeafQuery, LeafInfo());
    _BudQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(_BudQuery, BudType());
    _TreeQuery = EntityManager::CreateEntityQuery();
    EntityManager::SetEntityQueryAllFilters(_TreeQuery, TreeType());
    
    _TreeSystem = Application::GetWorld()->CreateSystem<TreeSystem>(SystemGroup::SimulationSystemGroup);
    _BudSystem = Application::GetWorld()->CreateSystem<BudSystem>(SystemGroup::SimulationSystemGroup);
    _LeafSystem = Application::GetWorld()->CreateSystem<LeafSystem>(SystemGroup::SimulationSystemGroup);
    
    _TreeIndex.Value = 0;
    _BudIndex.Value = 0;
    _LeafIndex.Value = 0;
    
    _Ready = true;
}

bool TreeUtilities::TreeManager::IsReady()
{
    return _Ready;
}


EntityQuery TreeUtilities::TreeManager::GetBudQuery()
{
    return _BudQuery;
}

EntityQuery TreeUtilities::TreeManager::GetTreeQuery()
{
    return _TreeQuery;
}

EntityQuery TreeUtilities::TreeManager::GetLeafQuery()
{
    return _LeafQuery;
}

BudSystem* TreeUtilities::TreeManager::GetBudSystem()
{
    return _BudSystem;
}

TreeSystem* TreeUtilities::TreeManager::GetTreeSystem()
{
    return _TreeSystem;
}

LeafSystem* TreeUtilities::TreeManager::GetLeafSystem()
{
    return _LeafSystem;
}

void TreeUtilities::TreeManager::GetAllTrees(std::vector<Entity>* container)
{
    if (!_Ready) {
        Debug::Error("TreeManager: Not initialized!");
        return;
    }
    return _TreeQuery.ToEntityArray(container);
}

void TreeUtilities::TreeManager::GenerateLeavesForTree(Entity treeEntity, LeafInfo leafInfo)
{
    TreeIndex treeIndex = EntityManager::GetComponentData<TreeIndex>(treeEntity);
#pragma region Create leaves for all buds
    std::vector<Entity> buds;
    _BudQuery.ToEntityArray(treeIndex, &buds);
    for (Entity bud : buds) {
        //Clear all leafs
        auto children = EntityManager::GetChildren(bud);
        for (auto i : children) {
            if (EntityManager::HasComponentData<LeafIndex>(i)) EntityManager::DeleteEntity(i);
        }
        //Generate leaves
        int leavesAmount = 2;
        for (int i = 0; i < leavesAmount; i++) {
            Entity leaf = CreateLeaf();
            //TODO: Set Component Data for leaf.
            EntityManager::SetComponentData(leaf, treeIndex);
            LeafGenerationHelper(leafInfo, leaf, bud, i);
        }
    }
#pragma endregion
#pragma region Prone leaves for all buds
    std::vector<Entity> leaves;
    _LeafQuery.ToEntityArray(treeIndex, &leaves);
    for (Entity leaf : leaves) {
        bool prone = false;
        //TODO: Add prone condition here.
        if (prone) EntityManager::DeleteEntity(leaf);
    }
#pragma endregion
}

void TreeUtilities::TreeManager::GenerateLeavesForAllTrees(LeafInfo leafInfo)
{
#pragma region Create leaves for all buds
    std::vector<Entity> buds;
    _BudQuery.ToEntityArray(&buds);
    for (Entity bud : buds) {
        //Clear all leafs
        auto children = EntityManager::GetChildren(bud);
        for (auto i : children) {
            if (EntityManager::HasComponentData<LeafIndex>(i)) EntityManager::DeleteEntity(i);
        }
        //Generate leaves
        int leavesAmount = 2;
        for (int i = 0; i < leavesAmount; i++) {
            Entity leaf = CreateLeaf();
            //TODO: Set Component Data for leaf.
            EntityManager::SetComponentData(leaf, EntityManager::GetComponentData<TreeIndex>(bud));
            LeafGenerationHelper(leafInfo, leaf, bud, i);
        }
    }
#pragma endregion
#pragma region Prone leaves for all buds
    std::vector<Entity> leaves;
    _LeafQuery.ToEntityArray(&leaves);
    for (Entity leaf : leaves) {
        bool prone = false;
        //TODO: Add prone condition here.
        if (prone) EntityManager::DeleteEntity(leaf);
    }
#pragma endregion
}

void TreeUtilities::TreeManager::GenerateMeshForAllTrees()
{
    if (!_Ready) {
        Debug::Error("TreeManager: Not initialized!");
        return;
    }
    std::vector<Entity> trees;
    GetAllTrees(&trees);
    for (auto i : trees) {
        GenerateMeshForTree(i);
    }
}

Entity TreeUtilities::TreeManager::CreateTree()
{
    auto entity = EntityManager::CreateEntity(_TreeArchetype);
    EntityManager::SetComponentData(entity, _TreeIndex);
    _TreeIndex.Value++;
    return entity;
}

Entity TreeUtilities::TreeManager::CreateBud()
{
    auto entity = EntityManager::CreateEntity(_BudArchetype);
    EntityManager::SetComponentData(entity, _BudIndex);
    _BudIndex.Value++;
    return entity;
}

Entity TreeUtilities::TreeManager::CreateLeaf()
{
    auto entity = EntityManager::CreateEntity(_LeafArchetype);
    EntityManager::SetComponentData(entity, _LeafIndex);
    _LeafIndex.Value++;
    return entity;
}

Mesh* TreeUtilities::TreeManager::GetMeshForTree(Entity treeEntity)
{
    if (!_Ready) {
        Debug::Error("TreeManager: Not initialized!");
        return nullptr;
    }
    return EntityManager::GetSharedComponent<MeshMaterialComponent>(treeEntity)->_Mesh;
}
#pragma endregion


void TreeUtilities::TreeManager::GenerateMeshForTree(Entity treeEntity)
{
    if (!_Ready) {
        Debug::Error("TreeManager: Not initialized!");
        return;
    }
    auto rootBud = EntityManager::GetChildren(treeEntity).at(0);
    //TODO: Finish mesh generation here.
}

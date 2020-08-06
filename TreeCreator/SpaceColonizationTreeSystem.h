#pragma once
#include "UniEngine.h"
#include "TreeManager.h"
#include "Envelope.h"
using namespace UniEngine;
using namespace TreeUtilities;
struct AttractionPointIndex {
    unsigned Value;
    AttractionPointIndex() { Value = 0; }
    AttractionPointIndex(unsigned value) {
        Value = value;
    }
};

struct AttractionPointCurrentStatus {
    bool remove;
    unsigned budEntityIndex;
    float distance;
    glm::vec3 growDirDelta;
};

class SpaceColonizationTreeSystem :
    public SystemBase
{
    TreeBudSystem* _TreeBudSystem;
    EntityArchetype _BudArchetype;
    EntityArchetype _LeafArchetype;
    EntityArchetype _TreeArchetype;
    EntityQuery _BudQuery;
    EntityQuery _LeafQuery;
    EntityQuery _TreeQuery;

    EntityArchetype _AttractionPointArchetype;
    EntityQuery _AttractionPointQuery;

    Material* _AttractionPointMaterial;
    Envelope _Envelope;
    unsigned _ToGrowIteration;
    unsigned _AttractionPointMaxIndex;

    bool _IterationFinishMark;
    void AddAttractionPoint(Translation translation);
    void Grow();
public:
    void OnCreate();
    void OnDestroy();
    void Update();
    void FixedUpdate();
    void ResetEnvelope(float radius, float minHeight, float maxHeight);
    void ResetEnvelope(glm::vec3 spaceOffset, glm::vec3 spaceSize);
    void ResetEnvelopeType(EnvelopeType type);
    void PushAttractionPoints(unsigned value);
    void PushGrowIterations(unsigned iteration);
    void CreateTree(unsigned index, TreeColor color, glm::vec3 position);
    void ClearAttractionPoints();
};


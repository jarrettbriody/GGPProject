#pragma once
#include "pch.h"
#include "BehaviorTree.h"
#include "Entity.h"
#include "DebugLines.h"

class EnemySeesPlayer : public Behavior
{
private:
    Entity* enemy;
    Entity* player;
    float viewAngle;
    float viewDistance;
    bool* inCombat;
public:
    EnemySeesPlayer(Entity* e, Entity* p, float angle, float distance, bool* combat) : enemy(e), player(p), viewAngle(angle), viewDistance(distance), inCombat(combat) {}
    ~EnemySeesPlayer() {}

    void OnInitialize();
    void OnTerminate(Status s);
    Status Update();
};
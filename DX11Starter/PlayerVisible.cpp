#include "pch.h"
#include "PlayerVisible.h"

void PlayerVisible::OnInitialize()
{
}

void PlayerVisible::OnTerminate(Status s)
{
}

Status PlayerVisible::Update()
{
	// Use a raycast to see if there is anything obstructing the enemy's view of the player

	// Update physics
	Config::DynamicsWorld->updateAabbs();
	Config::DynamicsWorld->computeOverlappingPairs();

	btVector3 from(enemy->GetTransform().GetPosition().x, enemy->GetTransform().GetPosition().y, enemy->GetTransform().GetPosition().z);
	btVector3 to(player->GetTransform().GetPosition().x, player->GetTransform().GetPosition().y, player->GetTransform().GetPosition().z);

	// Create variable to store the ray hits and set flags
	btCollisionWorld::AllHitsRayResultCallback allResults(from, to);
	allResults.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
	allResults.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

	Config::DynamicsWorld->rayTest(from, to, allResults); // Raycast

	if (allResults.hasHit())
	{
		PhysicsWrapper* wrapper;

		for (int i = 0; i < allResults.m_hitFractions.size(); i++)
		{
			wrapper = (PhysicsWrapper*)allResults.m_collisionObjects[i]->getUserPointer();
			if (wrapper->type == PHYSICS_WRAPPER_TYPE::ENTITY) {
				Entity* hit = (Entity*)wrapper->objectPointer;

				if (hit->HasTag("Enemy"))
					continue;
				else if (hit->GetName() == "FPSController")
					return SUCCESS;
				else
					return FAILURE;
			}
		}
	}
}
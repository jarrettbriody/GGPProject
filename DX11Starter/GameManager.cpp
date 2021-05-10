#include "pch.h"
#include "GameManager.h"

void GameManager::Init()
{
	eMap = ScriptManager::sceneEntitiesMap;

	enemiesAlive = EESceneLoader->SceneEntitiesTagMap["Enemy"].size(); // enemies alive on game start

	totalSplitMeshEntities = std::vector<Entity*>();
	
	bloodPoolParams = {
			"Blood Pool",					// name
			"Blood Pool",					// tag
			"Blood Pool",					// layer
			"bloodpool",							// mesh
			"Red",							// material
			{"BLOODPOOL"},				// script names
			1,								// script count
			XMFLOAT3(0.0f, 0.0f, 0.0f),		// position
			XMFLOAT3(0.0f, 0.0f, 0.0f),		// rotation
			XMFLOAT3(0.1f, 0.1f, 0.1f),		// scale
			0.0f,							// mass
			false
			// defaults work for the rest
	};
	
	gs = GameState::Gameplay; // For testing purposes right now, change when menus and cinematic are added to the game

	//-------------------------------------------------------
	// AI Tests
	//-------------------------------------------------------

	NavmeshHandler* navmesh = NavmeshHandler::GetInstance();

	GridDescription desc = {};

	// Starting alley
	desc.gridID = 0;
	desc.position = XMFLOAT3(-135.0f, 100.0f, -105.0f);
	desc.size = XMFLOAT3(58.0f, 1000.0f, 280.0f);
	desc.nodeSpacing = 8.0f;
	navmesh->AddGrid(desc, true);

	// Alley before fire
	desc.gridID = 1;
	desc.position = XMFLOAT3(-325.0f, 100.0f, -250.0f);
	desc.size = XMFLOAT3(55.0f, 1000.0f, 580.0f);
	navmesh->AddGrid(desc, true);

	// Alley after fire
	desc.gridID = 2;
	desc.position = XMFLOAT3(-325.0f, 100.0f, 405.0f);
	desc.size = XMFLOAT3(55.0f, 1000.0f, 480.0f);
	navmesh->AddGrid(desc, true);

	// Connecting alley + new_moratorium (WILL PROBABLY NEED SOME ADJUSTING)
	desc.gridID = 3;
	desc.position = XMFLOAT3(-340.0f, 100.0f, 890.0f);
	desc.size = XMFLOAT3(500.0f, 1000.0f, 260.0f);
	navmesh->AddGrid(desc, true);

	// Rooftops
	desc.gridID = 4;
	desc.position = XMFLOAT3(-70.0f, 100.0f, 1250.0f);
	desc.size = XMFLOAT3(60.0f, 1000.0f, 1050.0f);
	navmesh->AddGrid(desc, true);

	// Alley after rooftops/before sewers
	desc.gridID = 5;
	desc.position = XMFLOAT3(-200.0f, 100.0f, 2305.0f);
	desc.size = XMFLOAT3(315.0f, 1000.0f, 55.0f);
	navmesh->AddGrid(desc, true);

	// Area after sewers/before boss
	desc.gridID = 6;
	desc.position = XMFLOAT3(130.0f, 100.0f, 2375.0f);
	desc.size = XMFLOAT3(230.0f, 1000.0f, 260.0f);
	navmesh->AddGrid(desc, true);

	// Boss area
	desc.gridID = 7;
	desc.position = XMFLOAT3(420.0f, 100.0f, 2370.0f);
	desc.size = XMFLOAT3(590.0f, 1000.0f, 420.0f);
	navmesh->AddGrid(desc, true);
}

void GameManager::Update()
{
	if (Keyboard::GetInstance()->KeyIsPressed((unsigned int)'T')) {
		EESceneLoader->SceneEntitiesMap["graveyard"]->GetTransform().RotateAroundAxis(Y_AXIS, 1.0f * deltaTime);
	}

	switch (gs)
	{
	case GameState::Intro:

		break;

	case GameState::MainMenu:
		
		break;

	case GameState::Gameplay:
		gameTimer -= deltaTime;

		// cout << "Game Timer: " << gameTimer << " Enemies Left: " << enemiesAlive << endl;
		if (gameTimer <= 0 && enemiesAlive > 0) // lose condition TODO: Change to if only if boss is alive and make enemies alive part of thes coring
		{
			gs = GameState::GameOver;
		}

		BloodPoolSpawner();

		break;

	case GameState::Victory:
		
		break;

	case GameState::GameOver:
	
		break;

	default:

		break;
	}
}

void GameManager::BloodPoolSpawner()
{
	for (int i = totalSplitMeshEntities.size() - 1; i >= 0; i--)
	{
		btVector3 from = totalSplitMeshEntities[i]->GetRBody()->getCenterOfMassPosition();
		btVector3 to = btVector3(from.getX(), from.getY() - 3.0f, from.getZ());

		btCollisionWorld::ClosestRayResultCallback closestResult = Utility::BulletRaycast(from, to);

		if (closestResult.hasHit())
		{
			PhysicsWrapper* wrapper = (PhysicsWrapper*)closestResult.m_collisionObject->getUserPointer();
			if (wrapper->type == PHYSICS_WRAPPER_TYPE::ENTITY) {
				Entity* e = (Entity*)wrapper->objectPointer;
				if (e->HasTag(std::string("Environment")))
				{
					bloodPoolParams.position = Utility::BulletVectorToFloat3(closestResult.m_hitPointWorld);
					ScriptManager::CreateEntity(bloodPoolParams);
					totalSplitMeshEntities.erase(totalSplitMeshEntities.begin() + i); // remove the body part from the list if it already was close enough to the ground to leave a blood puddle
				}
			}
		}
	}
}

void GameManager::DecrementEnemiesAlive()
{
	enemiesAlive--;
}

void GameManager::AddRangeToTotalSplitMeshEntities(std::vector<Entity*> splitMeshEntities)
{
	this->totalSplitMeshEntities.insert(this->totalSplitMeshEntities.end(), splitMeshEntities.begin(), splitMeshEntities.end());
}

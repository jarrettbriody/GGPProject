#include "pch.h"
#include "TowerEnemy.h"

#define TOWER_DEATH_VOLUME 0.5f
#define HOOKSHOT_ENEMY_IMPACT_VOLUME 0.5f

TowerEnemy::~TowerEnemy()
{
	delete bt;
	int index = (rand() % 10);
	Config::FMODResult = Config::FMODSystem->playSound(Config::TowerDeath[index], Config::SFXGroup, false, &Config::SFXChannel);
	Config::SFXChannel->setVolume(TOWER_DEATH_VOLUME * Config::SFXVolume);
	XMFLOAT3 epos = entity->GetTransform().GetPosition();
	FMOD_VECTOR pos = { epos.x, epos.y, epos.z };
	FMOD_VECTOR vel = { 0, 0, 0 };

	Config::SFXChannel->set3DAttributes(&pos, &vel);
	Config::SFXChannel->set3DMinMaxDistance(0, 75.0f);
}

void TowerEnemy::Init()
{
	eMap = ScriptManager::sceneEntitiesMap;
	sMap = ScriptManager::scriptFunctionsMapVector;

	//sMap["FPSCONTROLLER"].front()
	fpsControllerScript = (FPSController*)(scriptFunctionsMap["FPSController"]["FPSCONTROLLER"]);
	gameManagerScript = (GameManager*)(scriptFunctionsMap["GameManager"]["GAMEMANAGER"]);

	//grid = &controller->grid;

	//entity->AddTag(std::string("Tower"));

	Entity* player = eMap->find("FPSController")->second;

	pos = entity->GetTransform().GetPosition();

	/*bt = BehaviorTreeBuilder()
			.Leaf<EnemySeesPlayer>(entity, player, 30.0f, 30.0f).End()
		.End();*/

	EntityCreationParameters projParams = {
		"towerProjectile",
		"towerProjectile",
		"projectile",
		"spikefat",
		"Red",
		{"PROJECTILE"},
		1,
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(0, 0, 0),
		XMFLOAT3(1, 1, 1),
		1.0f
	};

	entity->GetRBody()->setAngularFactor(btVector3(0, 1, 0)); // Constrain rotations on x and z axes
	entity->GetRBody()->setLinearFactor(btVector3(1, 0, 1)); // Constrain movement on the y axis

	bt =	BehaviorTreeBuilder()
				.Composite<ActiveSelector>()
					.Composite<Sequence>() // Attack the player if they are visible/in range
						.Leaf<InCombat>(&inCombat).End()
						.Leaf<PlayerVisible>(entity, player).End()
						.Leaf<PlayerIsInRange>(entity, player, minimumDistance, &playerIsInRange).End()
						.Leaf<AbilityAvailable>(&projectileCooldownTimer).End()
						.Leaf<FacePlayer>(entity, player, turnSpeed, &deltaTime).End()
						.Leaf<FireProjectile>(entity, player, projParams, projectileSpeed, &projectileCooldownTimer, PROJECTILE_COOLDOWN_MAX).End()
					.End()
					.Composite<Sequence>() // Seek the player if they are visible
						.Leaf<InCombat>(&inCombat).End()
						.Leaf<PlayerVisible>(entity, player).End()
						.Leaf<FacePlayer>(entity, player, turnSpeed, &deltaTime).End()
						.Leaf<SeekAndFleePlayer>(entity, player, &aStarSolver, &teleportCooldownTimer, TELEPORT_COOLDOWN_MAX, minimumDistance, &playerIsInRange).End()
					.End()
					.Composite<Sequence>() // Search player's last known location
						.Leaf<InCombat>(&inCombat).End()
						.Decorator<Invert>()
							.Leaf<PlayerVisible>(entity, player).End()
						.End()
						.Leaf<FindPlayer>(entity, player, &aStarSolver, &path).End()
						.Leaf<FollowPath>(&path, entity, movementSpeed, minimumDistance, turnSpeed, &deltaTime).End()
						.Leaf<Idle>(entity, &inCombat).End()
					.End()
					.Composite<Sequence>() // Enemy idle
						.Decorator<Invert>()
							.Leaf<InCombat>(&inCombat).End()
						.End()
						.Leaf<Idle>(entity, &inCombat).End()
						.Leaf<EnemySeesPlayer>(entity, player, visionConeAngle, visionConeDistance, &inCombat).End()
					.End()
				.End()
			.End();

	keyboard = Keyboard::GetInstance();
	navmesh = NavmeshHandler::GetInstance();
}

void TowerEnemy::Update()
{
	// Hover
	//totalTime += deltaTime;
	//pos.y = pos.y + sin(totalTime) * oscillationMagnitude;
	//entity->SetPosition(pos);
	//entity->CalcWorldMatrix();

	if (projectileCooldownTimer > 0)
	{
		projectileCooldownTimer -= deltaTime;
	}

	if (teleportCooldownTimer > 0)
	{
		teleportCooldownTimer -= deltaTime;
	}

	CheckPlayerState();

	// TODO: Reset the enemy transformation properly after leash is over
	if (delay <= 0)
	{
		if (leashed)
		{
			entity->GetRBody()->setAngularFactor(btVector3(1, 1, 1));
			entity->GetRBody()->setLinearFactor(btVector3(1, 1, 1));
		}
		else
		{
			entity->GetRBody()->setAngularFactor(btVector3(0, 1, 0)); // Constrain rotations on x and z axes
			entity->GetRBody()->setAngularVelocity(btVector3(0, 0, 0));
			entity->GetRBody()->setLinearFactor(btVector3(1, 0, 1)); // Constrain movement on the y axis

			Status result = bt->Run();
		}
	}
	else
	{
		delay -= deltaTime;
	}
}

void TowerEnemy::OnCollision(btCollisionObject* other)
{
	Entity* otherE = (Entity*)((PhysicsWrapper*)other->getUserPointer())->objectPointer;

	// cout << "Enemy collides with: " << otherE->GetName() << endl;

	// kill if slamming into the wall while leashed
	if (otherE->HasTag(std::string("Environment")) && !otherE->HasTag(std::string("street")) && entity->GetRBody()->getLinearVelocity().length() > killSpeedWhileLeashed && !entity->destroyed)
	{
		// Store the old enemy position for later use in case the enemy was killed while leashed
		btVector3 oldEnemyPos = entity->GetRBody()->getCenterOfMassPosition();

		// split it apart
		std::vector<Entity*> childEntities = EESceneLoader->SplitMeshIntoChildEntities(entity, "Body Part", "", 10.0f, 30.0f, 20.0f, "BODYPART");

		// Update the game manager attribute for enemies alive
		gameManagerScript->DecrementEnemiesAlive();

		Entity* newLeashedEntity = childEntities[0];
		for each (Entity * e in childEntities)
		{
			e->GetRBody()->activate();
			//e->GetRBody()->applyCentralImpulse(btVector3(100, 100, 100));
			//e->GetRBody()->applyTorqueImpulse(btVector3(100, 100, 100));

			if (leashed)
			{
				if (e->GetRBody()->getCenterOfMassPosition().distance(oldEnemyPos) < newLeashedEntity->GetRBody()->getCenterOfMassPosition().distance(oldEnemyPos))
				{
					newLeashedEntity = e;
				}
			}
		}

		//gameManagerScript->AddRangeToTotalSplitMeshEntities(childEntities);
		if (leashed) fpsControllerScript->SetLeashedEntity(newLeashedEntity);

		Config::FMODResult = Config::FMODSystem->playSound(Config::Icicle[0], Config::SFXGroup, false, &Config::SFXChannel);
		Config::SFXChannel->setVolume(HOOKSHOT_ENEMY_IMPACT_VOLUME * Config::SFXVolume);
		XMFLOAT3 epos = entity->GetTransform().GetPosition();
		FMOD_VECTOR pos = { epos.x, epos.y, epos.z };
		FMOD_VECTOR vel = { 0, 0, 0 };

		Config::SFXChannel->set3DAttributes(&pos, &vel);
		Config::SFXChannel->set3DMinMaxDistance(0, 75.0f);
	}
}

void TowerEnemy::IsLeashed(bool leashed, float delay)
{
	this->leashed = leashed;
	this->delay = delay;
}

void TowerEnemy::CheckPlayerState()
{
	if (fpsControllerScript->GetPlayerState() == PlayerState::Death) inCombat = false;
}

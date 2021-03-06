#pragma once
#include "ScriptManager.h"
#include "BehaviorTreeBuilder.h"
#include "EnemySeesPlayer.h"
#include "InCombat.h"
#include "Invert.h"
#include "PlayerVisible.h"
#include "FacePlayer.h"
#include "SeekPlayer.h"
#include "FindPlayer.h"
#include "FollowPath.h"
#include "Idle.h"
#include "FireProjectile.h"
#include "PlayerIsInRange.h"
#include "AbilityAvailable.h"
#include "ChargePlayer.h"
#include "FPSController.h"
#include "NavmeshHandler.h"
#include "AStarSolver.h"

class BullEnemy : public ScriptManager, public BaseEnemy
{
	~BullEnemy();

	map<string, Entity*>* eMap;
	map<string, vector<ScriptManager*>> sMap;

	FPSController* fpsControllerScript;
	GameManager* gameManagerScript;

	//Grid* grid;
	std::vector<Node*> path;
	NavmeshHandler* navmesh = nullptr;
	AStarSolver aStarSolver;

	XMFLOAT3 pos;
	bool inCombat = false;
	bool playerIsInRange = false;
	float chargeSpeed = 100.0f;
	float chargeCooldownTimer = 0.0f;
	const float CHARGE_COOLDOWN_MAX = 1.0f;
	float movementSpeed = 15.0f;
	float maxSpeed = 40.0f;
	float dampingScalar = 0.09f;
	float turnSpeed = 7.0f;
	float minimumDistance = 25.0f;
	float visionConeAngle = 50.0f;
	float visionConeDistance = 100.0f;
	float oscillationMagnitude = 0.01f;
	float totalTime = 0.0f;

	float killSpeedWhileLeashed = 35.0f;
	bool leashed;
	float delay;

	BehaviorTree* bt;

	Keyboard* keyboard;

	void Init();

	void Update();

	void OnCollision(btCollisionObject* other);

public:
	void IsLeashed(bool leashed, float delay);
	void CheckPlayerState();
};

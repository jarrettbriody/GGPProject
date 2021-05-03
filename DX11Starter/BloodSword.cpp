#include "pch.h"
#include "BloodSword.h" 
#include "FPSController.h"

void BloodSword::Init()
{
	eMap = ScriptManager::sceneEntitiesMap;

	gameManagerScript = (GameManager*)scriptFunctionsMap[(*eMap)["GameManager"]->GetName()]["GAMEMANAGER"];
	fpsControllerScript = (FPSController*)scriptFunctionsMap[(*eMap)["FPSController"]->GetName()]["FPSCONTROLLER"];

	cam = ScriptManager::EERenderer->GetCamera("main");

	keyboard = Keyboard::GetInstance();

	ss = SwordState::Idle;
	
	// starting positioning
	entity->SetParentWorldMatrix(cam->GetWorldMatrixPtr());
	
	finalLerpPos = originalLerpPos;
	lerpPositionFrom = finalLerpPos; 

	XMFLOAT3 x = X_AXIS;
	XMFLOAT3 y = Y_AXIS;
	XMFLOAT3 z = Z_AXIS;
	XMVECTOR tiltQuat = XMQuaternionRotationAxis(XMLoadFloat3(&x), XMConvertToRadians(-30.0f));
	XMVECTOR yawQuat = XMQuaternionRotationAxis(XMLoadFloat3(&y), 0.0f);
	XMVECTOR rollQuat = XMQuaternionRotationAxis(XMLoadFloat3(&z), XMConvertToRadians(-45.0f));
	XMVECTOR resultQuat = XMVector4Normalize(XMQuaternionMultiply(XMQuaternionMultiply(yawQuat, tiltQuat), rollQuat));
	XMStoreFloat4(&raisedQuatR, resultQuat);

	tiltQuat = XMQuaternionRotationAxis(XMLoadFloat3(&x), XMConvertToRadians(150.0f));
	yawQuat = XMQuaternionRotationAxis(XMLoadFloat3(&y), 0.0f);
	rollQuat = XMQuaternionRotationAxis(XMLoadFloat3(&z), XMConvertToRadians(-45.0f));
	resultQuat = XMVector4Normalize(XMQuaternionMultiply(XMQuaternionMultiply(yawQuat, tiltQuat), rollQuat));
	XMStoreFloat4(&slashingQuatR, resultQuat);

	tiltQuat = XMQuaternionRotationAxis(XMLoadFloat3(&x), XMConvertToRadians(-30.0f));
	yawQuat = XMQuaternionRotationAxis(XMLoadFloat3(&y), 0.0f);
	rollQuat = XMQuaternionRotationAxis(XMLoadFloat3(&z), XMConvertToRadians(45.0f));
	resultQuat = XMVector4Normalize(XMQuaternionMultiply(XMQuaternionMultiply(yawQuat, tiltQuat), rollQuat));
	XMStoreFloat4(&raisedQuatL, resultQuat);

	tiltQuat = XMQuaternionRotationAxis(XMLoadFloat3(&x), XMConvertToRadians(150.0f));
	yawQuat = XMQuaternionRotationAxis(XMLoadFloat3(&y), 0.0f);
	rollQuat = XMQuaternionRotationAxis(XMLoadFloat3(&z), XMConvertToRadians(45.0f));
	resultQuat = XMVector4Normalize(XMQuaternionMultiply(XMQuaternionMultiply(yawQuat, tiltQuat), rollQuat));
	XMStoreFloat4(&slashingQuatL, resultQuat);

	resultQuat = XMQuaternionIdentity();
	XMStoreFloat4(&idleQuat, resultQuat);

	finalLerpRot = idleQuat;
	lerpRotationFrom = finalLerpRot;

	entity->SetPosition(lerpPositionFrom); 
	entity->SetRotation(lerpRotationFrom);
	entity->SetScale(0.4f, 0.4f, 0.4f);
	entity->CalcWorldMatrix();

	/*slashPointsRight = {
		XMFLOAT3(5, 0, 3),
		XMFLOAT3(4, 0.5, 4),
		XMFLOAT3(3, -1.5, 4.25),
		XMFLOAT3(2, -2, 4.5),
		XMFLOAT3(1, -2.5, 4.5),
		XMFLOAT3(0, -3, 4.5),
		XMFLOAT3(-2, -3.5, 4.5),
		XMFLOAT3(-4, -4, 4.25),
		XMFLOAT3(-6, -4.5, 4),
		XMFLOAT3(-7, -5, 3),
		XMFLOAT3(-8, -6, 3)
	};

	slashPointsLeft = {
		XMFLOAT3(-5, 0, 3),
		XMFLOAT3(-4, 0.5, 4),
		XMFLOAT3(-3, -1.5, 4.25),
		XMFLOAT3(-2, -2, 4.5),
		XMFLOAT3(-1, -2.5, 4.5),
		XMFLOAT3(0, -3, 4.5),
		XMFLOAT3(2, -3.5, 4.5),
		XMFLOAT3(4, -4, 4.25),
		XMFLOAT3(6, -4.5, 4),
		XMFLOAT3(7, -5, 3),
		XMFLOAT3(8, -6, 3)
	};*/

	slashPointsRight = GenerateSlashPoints(XMFLOAT3(5, 2, 3), XMFLOAT3(-8, -2, 3), 0.5, 5.0f);
	slashPointsLeft = GenerateSlashPoints(XMFLOAT3(-5, 2, 3), XMFLOAT3(8, -2, 3), 0.5, 5.0f);

	callback = {};

	callback.vShader = EESceneLoader->vertexShadersMap["BloodSword"];
	callback.prepassVShader = EESceneLoader->vertexShadersMap["BloodSwordPrepass"];
	callback.waveHeightX = 0.01f;
	callback.waveHeightY = 0.3f;
	callback.waveHeightZ = 0.02f;
	callback.waveRateX = 5.0f;
	callback.waveRateY = 1.0f;
	callback.waveRateZ = 5.0f;

	EERenderer->SetRenderObjectCallback(entity, &callback);
	callback.active = true;

	ParticleEmitterDescription emitDesc;
	emitDesc.emitterPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	emitDesc.parentName = entity->GetName();
	emitDesc.parentWorld = entity->GetWorldMatrixPtr();//&emitterTransform;//
	emitDesc.emitterDirection = Z_AXIS;
	emitDesc.colorCount = 1;
	ParticleColor particleColors[1] = {
		{XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f},
	};
	emitDesc.colors = particleColors;
	emitDesc.bakeWorldMatOnEmission = true;
	emitDesc.emissionStartRadius = 0.5f;
	emitDesc.emissionEndRadius = 0.5f;
	emitDesc.emissionRate = 100.0f;
	emitDesc.maxParticles = 5000;
	emitDesc.particleInitMinSpeed = 0.01f;
	emitDesc.particleInitMaxSpeed = 0.11f;
	emitDesc.particleMinLifetime = 3.0f;
	emitDesc.particleMaxLifetime = 5.0f;
	emitDesc.particleInitMinScale = 0.01f;
	emitDesc.particleInitMaxScale = 0.03f;
	emitDesc.fadeInEndTime = 0.1f;
	emitDesc.fadeIn = true;
	emitDesc.fadeOutStartTime = 1.0f;
	emitDesc.fadeOut = true;
	emitDesc.particleAcceleration = XMFLOAT3(0, -10.0f, 0);

	emitter = new GPUParticleEmitter(emitDesc);
	((ParticleEmitter*)emitter)->SetIsActive(false);
}

void BloodSword::Update()
{
	callback.totalTime = totalTime;
	callback.deltaTime = deltaTime;
	originalLerpPos.y = originalLerpPos.y + sin(totalTime) * bobMagnitude;
	// Get the current lerp in relative to the local space of the camera
	switch (ss)
	{
	case SwordState::Idle:
		IdleState();
		break;
	case SwordState::Raised:
		RaisedState();
		break;
	case SwordState::Slashing:
		SlashingState();
		break;
	case SwordState::Reset:
		ResetState();
		break;
	default:
		break;
	}
	
	// Update the lerpFromPosition for the next time the lerp calculation is made 
	lerpPositionFrom = finalLerpPos;
	lerpRotationFrom = finalLerpRot;
	
	// Update blood sword position with local coordinate lerping data relative to camera 
	//if(XMVector3Length(XMVectorSubtract(XMLoadFloat3(&originalLerpRot), XMLoadFloat3(&finalLerpRot))).m128_f32[0] > 0.05f) 
	if (ss != SwordState::Idle)
		entity->SetRotation(finalLerpRot);
	entity->SetPosition(finalLerpPos);
	

	// Calc world matrix of the sword
	cam->CalcViewMatrix();
	cam->CalcWorldMatrix();
	entity->CalcWorldMatrix();
	//cam->CalcWorldMatrix(); // Putting camera world matrix calc after the entity makes the sword jitter much less severe...not sure why?

	totalTime += deltaTime;

	XMFLOAT4X4 world = entity->GetWorldMatrix();
	XMMATRIX transMat = XMMatrixTranslation(world._14, world._24, world._34);
	XMStoreFloat4x4(&emitterTransform, XMMatrixTranspose(transMat));
}


void BloodSword::StartSlash()
{
	if (ss == SwordState::Idle) // Only allow slash if it is idle state
	{
		if (keyboard->KeyIsPressed(0x41)) // a - left -> since character is right handed we only have to check for this, otherwise do the right handed slash
		{
			slashPoints = slashPointsLeft;
			//raisedRotation = XMFLOAT3(XMConvertToRadians(45.0f), 0.0f, 0.0f);
			//slashRotation = XMFLOAT3(XMConvertToRadians(45.0f), 0.0f, XMConvertToRadians(-70.0f));
			raisedQuat = raisedQuatL;
			slashingQuat = slashingQuatL;
		}
		else
		{
			slashPoints = slashPointsRight;
			//raisedRotation = XMFLOAT3(0.0f, 0.0f, XMConvertToRadians(-45.0f));
			//slashRotation = XMFLOAT3(XMConvertToRadians(70.0f), XMConvertToRadians(-90.0f), XMConvertToRadians(-45.0f));
			raisedQuat = raisedQuatR;
			slashingQuat = slashingQuatR;
		}

		ss = SwordState::Raised;
	}
}

void BloodSword::IdleState()
{
	// not doing any lerping in the idle state
	finalLerpPos = originalLerpPos; // starting pos
	finalLerpRot = idleQuat;
}

void BloodSword::RaisedState()
{
	lerpPositionTo = slashPoints[slashPointsIndex]; // First index is the raised state
	lerpRotationTo = raisedQuat;
	
	CalcLerp();

	cout << "Raising" << endl;

	if (CheckTransformationsNearEqual(true, true))
	{
		slashPointsIndex++;
		CheckSwordSlashHit();
		ss = SwordState::Slashing;
		((ParticleEmitter*)emitter)->SetIsActive(true);
	}
}

void BloodSword::SlashingState()
{
	lerpPositionTo = slashPoints[slashPointsIndex];
	lerpRotationTo = slashingQuat;
	
	CalcLerp();

	cout << "Slashing" << endl;

	if (CheckTransformationsNearEqual(true, false))
	{
		slashPointsIndex++;

		if (slashPointsIndex == slashPoints.size() && CheckTransformationsNearEqual(true, true))
		{
			slashPointsIndex = 0;
			ss = SwordState::Reset;
			((ParticleEmitter*)emitter)->SetIsActive(false);
		}
		
	}
}

void BloodSword::ResetState()
{
	lerpPositionTo = originalLerpPos;
	lerpRotationTo = idleQuat;

	CalcLerp();

	cout << "Resetting" << endl;

	animReset = true;

	if (CheckTransformationsNearEqual(true, true))
	{
		if (!approachingReset) {
			slashPositionLerpScalar = 300.0f;
			slashRotationLerpScalar = 50.0f;

			readyingPositionLerpScalar = 20.0f;
			readyingRotationLerpScalar = 20.0f;

			positionLerpTolerance = XMFLOAT3(0.01f, 0.01f, 0.01f);
			rotationLerpTolerance = XMFLOAT4(0.01f, 0.01f, 0.01f, 0.01f);

			approachingReset = true;
		}
		else {
			slashPositionLerpScalar = 150.0f;
			slashRotationLerpScalar = 25.0f;

			readyingPositionLerpScalar = 10.0f;
			readyingRotationLerpScalar = 10.0f;

			positionLerpTolerance = XMFLOAT3(0.5f, 0.5f, 0.5f);
			rotationLerpTolerance = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);

			approachingReset = false; 
			ss = SwordState::Idle;
			animReset = false;
		}
	}
}

bool BloodSword::CheckTransformationsNearEqual(bool checkPos, bool checkRot)
{
	XMVECTOR lerpPosFrom = XMLoadFloat3(&lerpPositionFrom);
	XMVECTOR lerpPosTo = XMLoadFloat3(&lerpPositionTo);
	XMVECTOR posTolerance = XMLoadFloat3(&positionLerpTolerance);

	XMVECTOR lerpRotFrom = XMLoadFloat4(&lerpRotationFrom);
	XMVECTOR lerpRotTo = XMLoadFloat4(&lerpRotationTo);
	XMVECTOR rotTolerance = XMLoadFloat4(&rotationLerpTolerance);

	if (checkPos)
	{
		return XMVector3NearEqual(lerpPosFrom, lerpPosTo, posTolerance);
	}
	else if (checkRot)
	{
		return XMVector4NearEqual(lerpRotFrom, lerpRotTo, rotTolerance);
	}
	else
	{
		return XMVector3NearEqual(lerpPosFrom, lerpPosTo, posTolerance) && XMVector4NearEqual(lerpRotFrom, lerpRotTo, rotTolerance);
	}

	
}

void BloodSword::CalcLerp() 
{
	float posLerpScalar;
	float rotLerpScalar;

	if (ss == SwordState::Raised || ss == SwordState::Reset)
	{
		posLerpScalar = readyingPositionLerpScalar;
		rotLerpScalar = readyingRotationLerpScalar;
	}
	else
	{
		posLerpScalar = slashPositionLerpScalar;
		rotLerpScalar = slashRotationLerpScalar;
	}

	XMStoreFloat3(&finalLerpPos, XMVectorLerp(XMLoadFloat3(&lerpPositionFrom), XMLoadFloat3(&lerpPositionTo), deltaTime * posLerpScalar));
	XMStoreFloat4(&finalLerpRot, XMQuaternionSlerp(XMLoadFloat4(&lerpRotationFrom), XMLoadFloat4(&lerpRotationTo), deltaTime * rotLerpScalar));
}

std::vector<XMFLOAT3> BloodSword::GenerateSlashPoints(XMFLOAT3 startingPos, XMFLOAT3 endingPos, float interval, float maxZ)
{
	std::vector<XMFLOAT3> generatedSlashPoints;
	XMFLOAT3 newPos = startingPos;
	generatedSlashPoints.push_back(newPos);

	bool leftSlash = newPos.x < 0; // if the starting x position is less than zero then we are slashing from the left

	// loop through and increment/decrement new points accordingly (x goes down or up depending on the slash direction, y always goes down, z increases until x is line with camera and then goes down)
	// TODO: Only caring about the x and y works for now, but for future work would be better to consider all coordinates (the problem is the z start and end position should be around the same because you are slashing out and in)
	while (!XMVector3NearEqual(XMLoadFloat3(&XMFLOAT3(newPos.x, newPos.y, 0)), XMLoadFloat3((&XMFLOAT3(endingPos.x, endingPos.y, 0))), XMLoadFloat3(&positionLerpTolerance))) 
	{
		float x = newPos.x;
		float y = newPos.y;
		float z = newPos.z;
		y -= interval;
		y = max(y, endingPos.y); // don't go beyond ending y
		
		if (leftSlash)
		{
			x += interval;
			x = min(x, endingPos.x); // don't go beyond ending x

			if (newPos.x >= 1) // we want to make sure that the max z is kept for a couple lerp points so this is >= 1 rather than >= 0
			{
				z -= interval;
			}
			else
			{
				z += interval;

				z = min(z, maxZ); // don't go beyond ending x
			}
		}
		else
		{
			x -= interval;
			x = max(x, endingPos.x); // don't go beyond ending x

			if (newPos.x <= -1) // we want to make sure that the max z is kept for a couple lerp points so this is <= -1 rather than <= 0
			{
				z -= interval;
			}
			else
			{
				z += interval;

				z = min(z, maxZ); // don't go beyond max z
			}
		}										    

		newPos = XMFLOAT3(x, y, z);
		generatedSlashPoints.push_back(newPos);
	}

	return generatedSlashPoints;
}

void BloodSword::CheckSwordSlashHit()
{
	std::vector<Entity*> enemies = EESceneLoader->sceneEntitiesTagMap["Enemy"];

	for each (Entity* enemy in enemies)
	{
		if (EntityInSlashDetectionField(enemy))
		{
			// Store the old enemy position for later use in case the enemy was killed while leashed
			btVector3 oldEnemyPos = enemy->GetRBody()->getCenterOfMassPosition();

			// Check if this enemy in the detection field is the leashed enemy when it is hit
			bool leashedWhenKilled = fpsControllerScript->GetPlayerState() == PlayerState::HookshotLeash && fpsControllerScript->GetLeashedEntity() == enemy;

			// enemy is in the triangle, split it apart
			std::vector<Entity*> childEntities = EESceneLoader->SplitMeshIntoChildEntities(enemy, 10.0f);

			// Update the game manager attribute for enemies alive
			gameManagerScript->DecrementEnemiesAlive();

			Entity* newLeashedEntity = childEntities[0];
			for each (Entity * e in childEntities)
			{
				e->tag = std::string("Body Part");

				if (leashedWhenKilled) // if the enemy is leashed while they are killed transfer the leash to the next closest body part
				{
					if (e->GetRBody()->getCenterOfMassPosition().distance(oldEnemyPos) < newLeashedEntity->GetRBody()->getCenterOfMassPosition().distance(oldEnemyPos))
					{
						newLeashedEntity = e;
					}
				}
			}

			if (leashedWhenKilled)
			{
				fpsControllerScript->SetLeashedEntity(newLeashedEntity);
			}
		}
	}

	/*std::vector<Entity*> bodyParts = EESceneLoader->sceneEntitiesTagMap["Body Part"];

	for each (Entity* bodyPart in bodyParts)
	{
		if (EntityInSlashDetectionField(bodyPart))
		{
			bodyPart->GetRBody()->applyCentralImpulse(Utility::Float3ToBulletVector(cam->direction).normalized() * 100.0f);
		}
	}*/
}

bool BloodSword::EntityInSlashDetectionField(Entity* e)
{
	float currentAngle = atan2(cam->direction.z, cam->direction.x);
	float halfViewAngle = XMConvertToRadians(viewAngle / 2);
	float hypotenuse = viewDistance / cos(halfViewAngle);
	XMFLOAT3 viewLeft = XMFLOAT3(cos(currentAngle + halfViewAngle) * hypotenuse, 0.0f, sin(currentAngle + halfViewAngle) * hypotenuse);
	XMFLOAT3 viewRight = XMFLOAT3(cos(currentAngle - halfViewAngle) * hypotenuse, 0.0f, sin(currentAngle - halfViewAngle) * hypotenuse);

	XMFLOAT3 viewFront;
	XMStoreFloat3(&viewFront, XMVectorSubtract(XMLoadFloat3(&viewRight), XMLoadFloat3(&viewLeft)));

	XMFLOAT3 playerPos = Utility::BulletVectorToFloat3(eMap->find("FPSController")->second->GetRBody()->getCenterOfMassPosition());
	XMFLOAT3 entityPos = Utility::BulletVectorToFloat3(e->GetRBody()->getCenterOfMassPosition());

	XMFLOAT3 flooredPos = XMFLOAT3(playerPos.x, 0, playerPos.z);
	XMFLOAT3 flooredEnemyPos = XMFLOAT3(entityPos.x, 0, entityPos.z);

	XMFLOAT3 triVertToEnemy;
	XMStoreFloat3(&triVertToEnemy, XMVectorSubtract(XMLoadFloat3(&flooredEnemyPos), XMLoadFloat3(&flooredPos)));

	XMFLOAT3 perpendicular = XMFLOAT3(viewLeft.z, viewLeft.y, -viewLeft.x);

	float dotProduct = 0;
	XMStoreFloat(&dotProduct, XMVector3Dot(XMLoadFloat3(&triVertToEnemy), XMLoadFloat3(&perpendicular)));

	if (dotProduct > 0)
	{
		XMStoreFloat3(&triVertToEnemy, XMVectorSubtract(XMLoadFloat3(&flooredEnemyPos), XMVectorAdd(XMLoadFloat3(&flooredPos), XMLoadFloat3(&viewLeft))));
		perpendicular = XMFLOAT3(viewFront.z, viewFront.y, -viewFront.x);

		XMStoreFloat(&dotProduct, XMVector3Dot(XMLoadFloat3(&triVertToEnemy), XMLoadFloat3(&perpendicular)));

		if (dotProduct > 0)
		{
			XMStoreFloat3(&triVertToEnemy, XMVectorSubtract(XMLoadFloat3(&flooredEnemyPos), XMVectorAdd(XMLoadFloat3(&flooredPos), XMLoadFloat3(&viewRight))));
			perpendicular = XMFLOAT3(-viewRight.z, -viewRight.y, viewRight.x);

			XMStoreFloat(&dotProduct, XMVector3Dot(XMLoadFloat3(&triVertToEnemy), XMLoadFloat3(&perpendicular)));

			if (dotProduct > 0)
			{
				return true;
			}
		}
	}

	return false;
}

void BloodSword::OnCollision(btCollisionObject* other)
{
	PhysicsWrapper* wrapper = (PhysicsWrapper*)other->getUserPointer();

	if (wrapper->type == PHYSICS_WRAPPER_TYPE::ENTITY)
	{
		Entity* otherE = (Entity*)wrapper->objectPointer;

		// cout << "Blood Sword Hit: " << otherE->GetName().c_str() << endl;
		
		if (otherE->tag.STDStr() == std::string("Enemy"))
		{
			std::vector<Entity*> childEntities = EESceneLoader->SplitMeshIntoChildEntities(otherE, 1.0f);
		}
	}
}

#pragma once

#include "pch.h"
#include "DXCore.h"

#include "Config.h"
#include "Utility.h"
#include "PhysicsWrapper.h"
#include "Lights.h"
#include "EEString.h"

#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Renderer.h"
#include "SceneLoader.h"
#include "DebugLines.h"
#include "ScriptManager.h"
#include "DebugLines.h"
#include "DecalHandler.h"
#include "ParticleEmitter.h"
#include "GPUParticleEmitter.h"
#include "CPUParticleEmitter.h"
#include "Scripts.h"
#include "LightHandler.h"
#include "NavmeshHandler.h"
#include "AudioManager.h"
#include <dxgidebug.h>

using namespace std;

struct SceneLoaderGarbageCallback : SceneLoaderCallback{
	DecalHandler* EEDecalHandler = nullptr;

	void callback(Entity* e) {
		string name = e->GetName();

		EEDecalHandler->DestroyDecalsByOwner(name);

		if (ParticleEmitter::EntityEmitterMap.count(name)) {
			for (auto iter = ParticleEmitter::EntityEmitterMap[name].begin(); iter != ParticleEmitter::EntityEmitterMap[name].end(); ++iter)
			{
				iter->second->Destroy();
			}
			ParticleEmitter::EntityEmitterMap.erase(name);
		}

		if (Config::EtherealDebugLinesEnabled && e->colliderDebugLinesEnabled) {
			DebugLines::debugLinesMap[name]->destroyed = true;
			DebugLines::debugLinesMap.erase(name);
		}

		vector<ScriptManager*> scriptFuncs = ScriptManager::scriptFunctionsMapVector[name];
		size_t cnt = scriptFuncs.size();
		for (size_t j = cnt; j > 0; j--)
		{
			scriptFuncs[j - 1]->destroyed = true;
		}
		ScriptManager::scriptFunctionsMap.erase(name);
		ScriptManager::scriptFunctionsMapVector.erase(name);
	}
};

struct SceneChangeCallback : Utility::Callback {
	bool cancelDraw = false;
	void* game = nullptr;
	void Call();
};

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(double deltaTime, double totalTime);
	void Draw(double deltaTime, double totalTime);

	void PhysicsStep(double deltaTime);
	void EnforcePhysics();
	void AudioStep();

	void FmodErrorCheck(FMOD_RESULT result); // Define it here because current file structure wont let me put it in utility
	void GarbageCollect();
	void ScriptGarbageCollect();
	void ParticleSystemGarbageCollect();
	void DebugLineGarbageCollect();

private:
	// Keeps track of the old mouse position for determining how far the mouse moved in a single frame

	Camera* EECamera = nullptr;
	Renderer* EERenderer = nullptr;
	SceneLoader* EESceneLoader = nullptr;
	MemoryAllocator* EEMemoryAllocator = nullptr;
	DecalHandler* EEDecalHandler = nullptr;
	LightHandler* EELightHandler = nullptr;
	
	//testing
	//Light* testLight = nullptr;

	// Physics
	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btBroadphaseInterface* broadphase = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;

	// DebugLines* physicsDraw;
	
	// Audio
	bool isPlaying = 0;
	FMOD_VECTOR listener_pos = FMOD_VECTOR();
	//FMOD_VECTOR listener_vel; // If we want a doppler effect
	FMOD_VECTOR listener_forward = FMOD_VECTOR();
	FMOD_VECTOR listener_up = FMOD_VECTOR();

	SceneLoaderGarbageCallback sceneLoaderGarbageCallback;
	SceneChangeCallback sceneChangeCallback;
};

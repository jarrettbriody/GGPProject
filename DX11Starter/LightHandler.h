#pragma once
#include "pch.h"
#include "Lights.h"
#include "MemoryAllocator.h"
#include "Config.h"

using namespace std;

class LightHandler
{
private:
	static LightHandler* instance;

	static LightShaders defaultShaders;

	MemoryAllocator* memAlloc = nullptr;

	LightHandler();
	~LightHandler();

public:
	vector<LightContainer*> lightsVec;
	map<string, LightContainer*> lightsMap;
	LightContainer* dirLight;
	map<string, map<string, LightContainer*>> entityLightMap;
	Light DrawList[MAX_LIGHTS];
	unsigned int DrawCount = 0;

	static bool SetupInstance();
	static LightHandler* GetInstance();
	static bool DestroyInstance();

	static void SetDefaultShaders(LightShaders shaders);

	void GarbageCollect();

	void Update(XMFLOAT4X4 camView);

	LightContainer* AddLight(LightContainer newLight);
	bool DestroyLight(string lightName);
	bool DestroyLightsByOwner(string entityName);
	LightContainer* GetLight(string lightName);
	map<string, LightContainer*> GetLights(string entityName);
};


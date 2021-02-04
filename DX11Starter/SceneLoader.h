#pragma once
#include "pch.h"
#include "DebugLines.h"
#include "Utility.h"
#include "Mesh.h"
#include "Material.h"
#include "Terrain.h"
#include "TerrainMaterial.h"
#include "Water.h"
#include "WaterMaterial.h"
#include "Entity.h"
#include "Config.h"
#include "MemoryAllocator.h"

using namespace Utility;

class SceneLoader
{
public:
	MemoryAllocator* EEMemoryAllocator = nullptr;

	btDiscreteDynamicsWorld* dynamicsWorld;

	map<string, SimpleVertexShader*> vertexShadersMap;
	map<string, SimplePixelShader*> pixelShadersMap;

	map<string, bool> utilizedMeshesMap;
	map<string, bool> utilizedMaterialsMap;
	map<string, bool> utilizedTexturesMap;

	map<string, Mesh*> defaultMeshesMap;
	map<string, Mesh*> generatedMeshesMap;

	map<string, ID3D11ShaderResourceView*> defaultTexturesMap;
	map<string, ID3D11ShaderResourceView*> generatedTexturesMap;

	map<string, Material*> defaultMaterialsMap;
	map<string, Material*> generatedMaterialsMap;

	map<string, Entity*> sceneEntitiesMap;
	vector<Entity*> sceneEntities;

	SceneLoader(MemoryAllocator* memAlloc, btDiscreteDynamicsWorld* dw);
	~SceneLoader();

	void LoadShaders();
	void LoadDefaultMeshes();
	void LoadDefaultTextures();
	void LoadDefaultMaterials();

	void BuildDefaultEntity(string entityName, string objName, Entity* e);

	Utility::MESH_TYPE AutoLoadOBJMTL(string name);
	void LoadScene(string sceneName = "scene");

	bool AddEntity(Entity* e);
};


#include "Game.h"
#include "Vertex.h"

using namespace DirectX;

Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,					// The application's handle
		"Small Shooting Arena",	   	// Text for the window's title bar
		1600,						// Width of the window's client area
		900,						// Height of the window's client area
		true)						// Show extra stats (fps) in title bar?
{

#if defined(DEBUG) || defined(_DEBUG)
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
}

Game::~Game()
{
	//defaults
	for (auto texMapIter = defaultTexturesMap.begin(); texMapIter != defaultTexturesMap.end(); ++texMapIter)
	{
		texMapIter->second->Release();
		//cout << "Releasing " << texMapIter->first << endl;
	}

	for (auto matMapIter = defaultMaterialsMap.begin(); matMapIter != defaultMaterialsMap.end(); ++matMapIter)
	{
		delete matMapIter->second;
		//cout << "Deleting " << matMapIter->first << endl;
	}

	for (auto meshMapIter = defaultMeshesMap.begin(); meshMapIter != defaultMeshesMap.end(); ++meshMapIter)
	{
		if (meshMapIter->first != "Ground") {
			delete meshMapIter->second;
			//cout << "Deleting " << meshMapIter->first << endl;
		}
	}

	//generated
	for (auto texMapIter = generatedTexturesMap.begin(); texMapIter != generatedTexturesMap.end(); ++texMapIter)
	{
		texMapIter->second->Release();
		//cout << "Releasing " << texMapIter->first << endl;
	}

	for (auto matMapIter = generatedMaterialsMap.begin(); matMapIter != generatedMaterialsMap.end(); ++matMapIter)
	{
		delete matMapIter->second;
		//cout << "Deleting " << matMapIter->first << endl;
	}

	for (auto meshMapIter = generatedMeshesMap.begin(); meshMapIter != generatedMeshesMap.end(); ++meshMapIter)
	{
		delete meshMapIter->second;
		//cout << "Deleting " << meshMapIter->first << endl;
	}

	for (size_t i = 0; i < sceneEntities.size(); i++)
	{
		delete sceneEntities[i];
	}

	//delete shaders
	for (auto vertSIter = vertexShadersMap.begin(); vertSIter != vertexShadersMap.end(); ++vertSIter)
	{
		delete vertSIter->second;
	}

	for (auto pixSIter = pixelShadersMap.begin(); pixSIter != pixelShadersMap.end(); ++pixSIter)
	{
		delete pixSIter->second;
	}

	sampler->Release();

	skySRV->Release();
	skyDepthState->Release();
	skyRasterState->Release();

	delete terrain;

	delete camera;
	delete renderer;
}

void Game::Init()
{
	//EtherealEngine::GetInstance()->SetDevice(device);
	//EtherealEngine::GetInstance()->SetContext(context);
	DebugLines::device = device;

	LoadShaders();

	camera = new Camera();
	camera->UpdateProjectionMatrix(width, height);
	//EtherealEngine::GetInstance()->SetCamera(camera);

	DirectX::CreateDDSTextureFromFile(device, L"../../Assets/Textures/SunnyCubeMap.dds", 0, &skySRV);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);

	D3D11_RASTERIZER_DESC skyRD = {};
	skyRD.CullMode = D3D11_CULL_FRONT;
	skyRD.FillMode = D3D11_FILL_SOLID;
	skyRD.DepthClipEnable = true;
	device->CreateRasterizerState(&skyRD, &skyRasterState);

	D3D11_DEPTH_STENCIL_DESC skyDS = {};
	skyDS.DepthEnable = true;
	skyDS.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	skyDS.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&skyDS, &skyDepthState);

	LoadDefaultMeshes();
	LoadDefaultTextures();
	LoadDefaultMaterials();

	LoadScene("ArenaV2");

	//test area --------------------------
	/*
	CUBE ALWAYS TRIES TO APPLY GRASS MATERIAL BECAUSE OF GROUND NEED TO FIX

	Entity* cube1;
	cube1 = new Entity("cube1", defaultMeshesMap["Cube"]);
	cube1->AddMaterial(defaultMaterialsMap["DEFAULT"]);
	cube1->AddMaterialNameToMesh("DEFAULT");
	cube1->SetPosition(8.0f, 8.0f, 8.0f);
	cube1->SetRotation(DirectX::XMConvertToRadians(30), DirectX::XMConvertToRadians(30), DirectX::XMConvertToRadians(30));
	cube1->SetScale(1.0f, 2.0f, 1.0f);
	sceneEntitiesMap.insert({ "cube1", cube1 });
	sceneEntities.push_back(cube1);

	Entity* cube2;
	cube2 = new Entity("cube2", defaultMeshesMap["Cube"]);
	cube2->AddMaterial(defaultMaterialsMap["DEFAULT"]);
	cube2->AddMaterialNameToMesh("DEFAULT");
	cube2->SetPosition(1.0f, 1.0f, 1.0f);
	cube2->SetRotation(DirectX::XMConvertToRadians(30), DirectX::XMConvertToRadians(30), DirectX::XMConvertToRadians(30));
	cube2->SetScale(1.0f, 1.0f, 1.0f);
	sceneEntitiesMap.insert({ "cube2", cube2 });
	sceneEntities.push_back(cube2);

	cube1->AddChildEntity(cube2);

	cube1->CalcWorldMatrix();
	*/

	Entity* sphere1;
	sphere1 = new Entity("sphere1", defaultMeshesMap["Sphere"]);
	sphere1->AddMaterial(defaultMaterialsMap["DEFAULT"]);
	sphere1->AddMaterialNameToMesh("DEFAULT");
	sphere1->SetPosition(8.0f, 8.0f, 8.0f);
	sphere1->SetRotation(DirectX::XMConvertToRadians(30), DirectX::XMConvertToRadians(30), DirectX::XMConvertToRadians(30));
	sphere1->SetScale(1.0f, 2.0f, 1.0f);
	sceneEntitiesMap.insert({ "sphere1", sphere1 });
	sceneEntities.push_back(sphere1);

	Entity* sphere2;
	sphere2 = new Entity("sphere2", defaultMeshesMap["Sphere"]);
	sphere2->AddMaterial(defaultMaterialsMap["DEFAULT"]);
	sphere2->AddMaterialNameToMesh("DEFAULT");
	sphere2->SetPosition(2.0f, 2.0f, 2.0f);
	sphere2->SetRotation(0.0f, 0.0f, 0.0f);
	sphere2->SetScale(1.0f, 1.0f, 2.0f);
	sceneEntitiesMap.insert({ "sphere2", sphere2 });
	sceneEntities.push_back(sphere2);

	sphere1->AddChildEntity(sphere2);

	sphere1->CalcWorldMatrix();

	sceneEntitiesMap["Rock"]->AddAutoBoxCollider();
	sceneEntitiesMap["Rock"]->CalcWorldMatrix();
	sceneEntitiesMap["Rock (1)"]->AddAutoBoxCollider();
	sceneEntitiesMap["Rock (1)"]->CalcWorldMatrix();

	//------------------------------------

	prevMousePos.x = 0;
	prevMousePos.y = 0;

	Light* dLight = new Light;
	dLight->Type = LIGHT_TYPE_DIR;
	dLight->Color = XMFLOAT3(1.0f, 244.0f / 255.0f, 214.0f / 255.0f);
	dLight->Direction = XMFLOAT3(0.5f, -1.0f, 1.0f);
	dLight->Intensity = 1.f;

	/*testLight = new Light;
	testLight->Type = LIGHT_TYPE_SPOT;
	testLight->Direction = camera->direction;
	testLight->Intensity = 5.f;
	testLight->Position = XMFLOAT3(-3.2f, 2.f, -5.f);
	testLight->Color = XMFLOAT3(1.f, 1.f, 1.f);
	testLight->Range = 10.f;
	testLight->SpotFalloff = 20.f;*/

	renderer = new Renderer(device, context, swapChain, backBufferRTV, depthStencilView, width, height);
	renderer->SetCamera(camera);
	renderer->SetShadowVertexShader(vertexShadersMap["Shadow"]);
	renderer->SetDebugLineVertexShader(vertexShadersMap["DebugLine"]);
	renderer->SetDebugLinePixelShader(pixelShadersMap["DebugLine"]);
	renderer->SetEntities(&sceneEntities);
	renderer->AddLight("Sun", dLight);
	//renderer->AddLight("testLight", testLight);
	renderer->SendAllLightsToShader(pixelShadersMap["DEFAULT"]);
	renderer->SendAllLightsToShader(pixelShadersMap["Normal"]);
	renderer->SetShadowMapResolution(4096);
	renderer->InitShadows();
	//EtherealEngine::GetInstance()->SetRenderer(renderer);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//terrain -----------------

	terrain = new Terrain(device, "../../Assets/valley.raw16", 513, 513, 1.0f, 50.0f, 1.0f);
	Entity* terrainEntity = new Entity("Terrain", terrain);
	terrainEntity->AddMaterial(defaultMaterialsMap["Terrain"]);
	terrainEntity->AddMaterialNameToMesh("Terrain");
	terrainEntity->SetPosition(0.f, -10.f, 0.f);
	terrainEntity->SetRotation(0.f, 0.f, 0.f);
	terrainEntity->SetScale(1.0f, 1.0f, 1.0f);
	sceneEntitiesMap.insert({ "Terrain", terrainEntity });
	sceneEntities.push_back(terrainEntity);
	terrainEntity->CalcWorldMatrix();

	water = new Water(0.0002f, device, 513, 513, 1.f, 1.f, 1.f, pixelShadersMap["Water"]);
	water->SetOffsets(0.2f, 0.1f, 0.1f, 0.2f);
	Entity* waterEntity = new Entity("Water", water->terrain);
	waterEntity->AddMaterial(defaultMaterialsMap["Water"]);
	waterEntity->AddMaterialNameToMesh("Water");
	waterEntity->SetPosition(0.f, -3.f, 0.f);
	waterEntity->SetRotation(0.f, 0.f, 0.f);
	waterEntity->SetScale(1.f, 1.f, 1.f);
	sceneEntitiesMap.insert({ "Water", waterEntity });
	sceneEntities.push_back(waterEntity);
	waterEntity->CalcWorldMatrix();

}

void Game::LoadShaders()
{
	//vertex shaders
	SimpleVertexShader* defaultVS = new SimpleVertexShader(device, context);
	defaultVS->LoadShaderFile(L"DefaultVS.cso");
	vertexShadersMap.insert({ "DEFAULT", defaultVS });

	SimpleVertexShader* shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"ShadowVS.cso");
	vertexShadersMap.insert({ "Shadow", shadowVS });

	SimpleVertexShader* skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"SkyVS.cso");
	vertexShadersMap.insert({ "Sky", skyVS });

	SimpleVertexShader* normalVS = new SimpleVertexShader(device, context);
	normalVS->LoadShaderFile(L"NormalVS.cso");
	vertexShadersMap.insert({ "Normal", normalVS });

	SimpleVertexShader* debugLineVS = new SimpleVertexShader(device, context);
	debugLineVS->LoadShaderFile(L"DebugLineVS.cso");
	vertexShadersMap.insert({ "DebugLine", debugLineVS });

	//pixel shaders
	SimplePixelShader* defaultPS = new SimplePixelShader(device, context);
	defaultPS->LoadShaderFile(L"DefaultPS.cso");
	pixelShadersMap.insert({ "DEFAULT", defaultPS });

	SimplePixelShader* skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"SkyPS.cso");
	pixelShadersMap.insert({ "Sky", skyPS });

	SimplePixelShader* normalPS = new SimplePixelShader(device, context);
	normalPS->LoadShaderFile(L"NormalPS.cso");
	pixelShadersMap.insert({ "Normal", normalPS });

	SimplePixelShader* debugLinePS = new SimplePixelShader(device, context);
	debugLinePS->LoadShaderFile(L"DebugLinePS.cso");
	pixelShadersMap.insert({ "DebugLine", debugLinePS });
  
	SimplePixelShader* terrainPS = new SimplePixelShader(device, context);
	terrainPS->LoadShaderFile(L"TerrainPS.cso");
	pixelShadersMap.insert({ "Terrain", terrainPS });

	SimplePixelShader* waterPS = new SimplePixelShader(device, context);
	waterPS->LoadShaderFile(L"WaterPS.cso");
	pixelShadersMap.insert({ "Water", waterPS });

}

void Game::LoadDefaultMeshes()
{
	defaultMeshesMap.insert({ "Cube", new Mesh("Cube", "../../Assets/Models/Default/cube.obj", device) });
	defaultMeshesMap.insert({ "Cylinder", new Mesh("Cylinder", "../../Assets/Models/Default/cylinder.obj", device) });
	defaultMeshesMap.insert({ "Cone", new Mesh("Cone", "../../Assets/Models/Default/cone.obj", device) });
	defaultMeshesMap.insert({ "Sphere", new Mesh("Sphere", "../../Assets/Models/Default/sphere.obj", device) });
	defaultMeshesMap.insert({ "Helix", new Mesh("Helix", "../../Assets/Models/Default/helix.obj", device) });
	defaultMeshesMap.insert({ "Torus", new Mesh("Torus", "../../Assets/Models/Default/torus.obj", device) });
	defaultMeshesMap.insert({ "Ground", defaultMeshesMap["Cube"] });
}

void Game::LoadDefaultTextures()
{
	defaultTexturesMap.insert({ "GrassDiffuse", Utility::LoadSRV(device,context,"Default/Grass/DefaultGrassDiffuse.jpg") });
	defaultTexturesMap.insert({ "GrassNormal", Utility::LoadSRV(device,context,"Default/Grass/DefaultGrassNormal.jpg") });
	defaultTexturesMap.insert({ "Red", Utility::LoadSRV(device,context,"Default/red.png") });
	defaultTexturesMap.insert({ "Marble", Utility::LoadSRV(device,context,"Default/marble.png") });
	defaultTexturesMap.insert({ "Hedge", Utility::LoadSRV(device,context,"Default/hedge.jpg") });
	defaultTexturesMap.insert({ "terrain2", Utility::LoadSRV(device,context,"grass.png") });
	defaultTexturesMap.insert({ "terrain3", Utility::LoadSRV(device,context,"rocky.png") });
	defaultTexturesMap.insert({ "terrain1", Utility::LoadSRV(device,context,"snow.jpg") });
	defaultTexturesMap.insert({ "terrainNormal2", Utility::LoadSRV(device,context,"grass_normal.png") });
	defaultTexturesMap.insert({ "terrainNormal3", Utility::LoadSRV(device,context,"rocky_normal.png") });
	defaultTexturesMap.insert({ "terrainNormal1", Utility::LoadSRV(device,context,"snow_normal.jpg") });
	defaultTexturesMap.insert({ "terrainBlendMap", Utility::LoadSRV(device,context,"blendMap.png") });
	defaultTexturesMap.insert({ "waterBase", Utility::LoadSRV(device,context,"water_base.png") });
	defaultTexturesMap.insert({ "waterFoam", Utility::LoadSRV(device,context,"water_foam.jpg") });
	defaultTexturesMap.insert({ "waterNormal1", Utility::LoadSRV(device,context,"water_normal1.jpeg") });
	defaultTexturesMap.insert({ "waterNormal2", Utility::LoadSRV(device,context,"water_normal2.png") });
}

void Game::LoadDefaultMaterials()
{
	MaterialData materialData = {};
	defaultMaterialsMap.insert({"DEFAULT", new Material("DEFAULT", materialData, vertexShadersMap["DEFAULT"], pixelShadersMap["DEFAULT"], sampler)});

	materialData = {};
	materialData.DiffuseTextureMapSRV = defaultTexturesMap["GrassDiffuse"];
	materialData.NormalTextureMapSRV = defaultTexturesMap["GrassNormal"];
	defaultMaterialsMap.insert({ "Grass", new Material("Grass", materialData, vertexShadersMap["Normal"], pixelShadersMap["Normal"], sampler) });

	materialData = {};
	materialData.DiffuseTextureMapSRV = defaultTexturesMap["Red"];
	defaultMaterialsMap.insert({ "Red", new Material("Red", materialData, vertexShadersMap["DEFAULT"], pixelShadersMap["DEFAULT"], sampler) });

	materialData = {};
	materialData.DiffuseTextureMapSRV = defaultTexturesMap["Marble"];
	defaultMaterialsMap.insert({ "Marble", new Material("Marble", materialData, vertexShadersMap["DEFAULT"], pixelShadersMap["DEFAULT"], sampler) });

	materialData = {};
	materialData.DiffuseTextureMapSRV = defaultTexturesMap["Hedge"];
	defaultMaterialsMap.insert({ "Hedge", new Material("Hedge", materialData, vertexShadersMap["DEFAULT"], pixelShadersMap["DEFAULT"], sampler) });

	TerrainMaterialData terrainMaterialData = {};
	terrainMaterialData.SurfaceTexture1 = defaultTexturesMap["terrain1"];
	terrainMaterialData.SurfaceTexture2 = defaultTexturesMap["terrain2"];
	terrainMaterialData.SurfaceTexture3 = defaultTexturesMap["terrain3"];
	terrainMaterialData.SurfaceNormal1 = defaultTexturesMap["terrainNormal1"];
	terrainMaterialData.SurfaceNormal2 = defaultTexturesMap["terrainNormal2"];
	terrainMaterialData.SurfaceNormal3 = defaultTexturesMap["terrainNormal3"];
	terrainMaterialData.uvScale = 50.0f;
	terrainMaterialData.BlendMap = defaultTexturesMap["terrainBlendMap"];
	defaultMaterialsMap.insert({ "Terrain", new TerrainMaterial("Terrain", terrainMaterialData, vertexShadersMap["DEFAULT"], pixelShadersMap["Terrain"], sampler) });

	WaterMaterialData waterMaterialData = {};
	waterMaterialData.SurfaceTexture1 = defaultTexturesMap["waterBase"];
	waterMaterialData.SurfaceTexture2 = defaultTexturesMap["waterFoam"];
	waterMaterialData.SurfaceNormal1 = defaultTexturesMap["waterNormal1"];
	waterMaterialData.SurfaceNormal2 = defaultTexturesMap["waterNormal2"];
	waterMaterialData.uvScale = 20.0f;
	defaultMaterialsMap.insert({ "Water", new WaterMaterial("Water", waterMaterialData, vertexShadersMap["DEFAULT"], pixelShadersMap["Water"], sampler) });

}

void Game::BuildDefaultEntity(string entityName, string objName, Entity* e)
{
	if (objName == "Ground") {
		e->AddMaterial(defaultMaterialsMap["Grass"]);
		e->AddMaterialNameToMesh("Grass");
		XMFLOAT3 s = e->GetScale();
		e->SetRepeatTexture(s.x / 2.0f, s.z / 2.0f);
	}
}

Utility::MESH_TYPE Game::AutoLoadOBJMTL(string name)
{
	//mesh exists in default meshes map, which will remain untouched during program execution
	if (defaultMeshesMap.count(name))
		return Utility::MESH_TYPE::DEFAULT_MESH;

	//if mesh is already loaded
	if (generatedMeshesMap.count(name)) {
		//if the mesh is not already recorded as utilized, utilize it
		if (!utilizedMeshesMap.count(name)){
			utilizedMeshesMap.insert({ name, true });
			//get the material names utilized under the mesh
			vector<string> utilizedMaterials = generatedMeshesMap[name]->GetMaterialNameList();
			for (int i = 0; i < utilizedMaterials.size(); i++)
			{
				//if the material is not already utilized, utilize it
				if (!utilizedMaterialsMap.count(utilizedMaterials[i])) {
					utilizedMaterialsMap.insert({ utilizedMaterials[i], true });
					//get the texture names utilized under the material
					vector<string> utilizedTextures = generatedMaterialsMap[utilizedMaterials[i]]->GetMaterialData().SRVNames;
					for (int j = 0; j < utilizedTextures.size(); j++)
					{
						//if the texture is not already utilized, utilize it
						if (!utilizedTexturesMap.count(utilizedTextures[i]))
							utilizedTexturesMap.insert({ utilizedTextures[i], true });
					}
				}
			}
		}
		//mesh is already loaded and mesh, materials, and textures are now marked as utilized, exit func
		return Utility::MESH_TYPE::GENERATED_MESH;
	}

	string objPath = "../../Assets/Models/" + name + ".obj";

	//Mesh will change bool ref to false if OBJ file does not exist, otherwise it will generate it and add to map
	bool success;
	generatedMeshesMap.insert({ name, new Mesh(name, (char*)objPath.c_str(), device, &success) });
	if (!success) {
		cout << "Cannot load Object (OBJ) file: " + string(objPath) << endl;
		return Utility::MESH_TYPE::LOAD_FAILURE;
	}

	//record mesh as utilized
	utilizedMeshesMap.insert({ name,true });

	string mtlPath = generatedMeshesMap[name]->GetMTLPath();

	if (mtlPath == "") {
		cout << "Material Template Library (MTL) link not found inside \"" + name + ".obj\". If this is unintentional, link MTL file inside OBJ file." << endl;
		return Utility::MESH_TYPE::GENERATED_MESH;
	}

	ifstream infile("../../Assets/Models/" + mtlPath);

	if (!infile.is_open()) {
		cout << "Material Template Library (MTL) file not found. Please include MTL file in same directory as OBJ file or remove the internal OBJ link to " + mtlPath + "." << endl;
		return Utility::MESH_TYPE::GENERATED_MESH;
	}

	using namespace Utility;

	regex newMtlRgx("^(newmtl )");
	regex ambientColorRgx("^(Ka )");
	regex diffuseColorRgx("^(Kd )");
	regex specularColorRgx("^(Ks )");
	regex specularExpRgx("^(Ns )");
	regex dTransparencyRgx("^(d )");
	regex trTransparencyRgx("^(Tr )");
	regex illuminationRgx("^(illum )");
	regex ambientTextureRgx("^(map_Ka )");
	regex diffuseTextureRgx("^(map_Kd )");
	regex specularColorTextureRgx("^(map_Ks )");
	regex specularHighlightTextureRgx("^(map_Ns )");
	regex alphaTextureRgx("^(map_d )");
	regex normalTextureRgx("^(map_Bump )");

	bool ongoingMat = false;
	string ongoingMatName = "";
	MaterialData matData;

	string line;
	smatch match;

	while (getline(infile, line)) {
		if (line != "" && !regex_search(line, match, regex("^#"))) {
			//search for new material line
			if (regex_search(line, match, newMtlRgx)) {
				line = regex_replace(line, newMtlRgx, "");
				//new material line was found but a material was in progress, complete this material before continuing
				if (ongoingMat) {
					//Different shaders based on matData values
					if (matData.NormalTextureMapSRV) {
						generatedMaterialsMap.insert({ ongoingMatName, new Material(ongoingMatName, matData, vertexShadersMap["Normal"], pixelShadersMap["Normal"], sampler) });
					}
					else {
						generatedMaterialsMap.insert({ ongoingMatName, new Material(ongoingMatName, matData, vertexShadersMap["DEFAULT"], pixelShadersMap["DEFAULT"], sampler) });
					}
					matData = {};
				}
				ongoingMat = true;
				ongoingMatName = line;

				//record material as utilized
				utilizedMaterialsMap.insert({ ongoingMatName,true });
			}
			//ambient color
			else if (regex_search(line, match, ambientColorRgx)) {
				line = regex_replace(line, ambientColorRgx, "");
				ParseFloat3FromString(line, matData.AmbientColor);
			}
			//diffuse color
			else if (regex_search(line, match, diffuseColorRgx)) {
				line = regex_replace(line, diffuseColorRgx, "");
				ParseFloat3FromString(line, matData.DiffuseColor);
			}
			//specular color
			else if (regex_search(line, match, specularColorRgx)) {
				line = regex_replace(line, specularColorRgx, "");
				ParseFloat3FromString(line, matData.SpecularColor);
			}
			//specular value
			else if (regex_search(line, match, specularExpRgx)) {
				line = regex_replace(line, specularExpRgx, "");
				ParseFloatFromString(line, matData.SpecularExponent);
			}
			//transparency value
			else if (regex_search(line, match, dTransparencyRgx)) {
				line = regex_replace(line, dTransparencyRgx, "");
				ParseFloatFromString(line, matData.Transparency);
			}
			//transparency value
			else if (regex_search(line, match, trTransparencyRgx)) {
				line = regex_replace(line, trTransparencyRgx, "");
				ParseFloatFromString(line, matData.Transparency);
				matData.Transparency = 1.0f - matData.Transparency;
			}
			//illumination value
			else if (regex_search(line, match, illuminationRgx)) {
				line = regex_replace(line, illuminationRgx, "");
				ParseIntFromString(line, matData.Illumination);
			}
			//ambient occlusion map
			else if (regex_search(line, match, ambientTextureRgx)) {
				line = regex_replace(line, ambientTextureRgx, "");
				if (!generatedTexturesMap.count(line)) {
					generatedTexturesMap.insert({ line, LoadSRV(device,context,line) });

					//record texture as utilized
					utilizedTexturesMap.insert({ line,true });
				}
				matData.AmbientTextureMapSRV = generatedTexturesMap[line];
				matData.SRVNames.push_back(line);
			}
			//diffuse map
			else if (regex_search(line, match, diffuseTextureRgx)) {
				line = regex_replace(line, diffuseTextureRgx, "");
				if (!generatedTexturesMap.count(line)) {
					generatedTexturesMap.insert({ line, LoadSRV(device,context,line) });

					//record texture as utilized
					utilizedTexturesMap.insert({ line,true });
				}
				matData.DiffuseTextureMapSRV = generatedTexturesMap[line];
				matData.SRVNames.push_back(line);
			}
			//specular color map
			else if (regex_search(line, match, specularColorTextureRgx)) {
				line = regex_replace(line, specularColorTextureRgx, "");
				if (!generatedTexturesMap.count(line)) {
					generatedTexturesMap.insert({ line, LoadSRV(device,context,line) });

					//record texture as utilized
					utilizedTexturesMap.insert({ line,true });
				}
				matData.SpecularColorTextureMapSRV = generatedTexturesMap[line];
				matData.SRVNames.push_back(line);
			}
			//specular highlight map
			else if (regex_search(line, match, specularHighlightTextureRgx)) {
				line = regex_replace(line, specularHighlightTextureRgx, "");
				if (!generatedTexturesMap.count(line)) {
					generatedTexturesMap.insert({ line, LoadSRV(device,context,line) });

					//record texture as utilized
					utilizedTexturesMap.insert({ line,true });
				}
				matData.SpecularHighlightTextureMapSRV = generatedTexturesMap[line];
				matData.SRVNames.push_back(line);
			}
			//alpha map
			else if (regex_search(line, match, alphaTextureRgx)) {
				line = regex_replace(line, alphaTextureRgx, "");
				if (!generatedTexturesMap.count(line)) {
					generatedTexturesMap.insert({ line, LoadSRV(device,context,line) });

					//record texture as utilized
					utilizedTexturesMap.insert({ line,true });
				}
				matData.AlphaTextureMapSRV = generatedTexturesMap[line];
				matData.SRVNames.push_back(line);
			}
			//bump map
			else if (regex_search(line, match, normalTextureRgx)) {
				line = regex_replace(line, normalTextureRgx, "");
				if (!generatedTexturesMap.count(line)) {
					generatedTexturesMap.insert({ line, LoadSRV(device,context,line) });

					//record texture as utilized
					utilizedTexturesMap.insert({ line,true });
				}
				matData.NormalTextureMapSRV = generatedTexturesMap[line];
				matData.SRVNames.push_back(line);
			}
		}
	}
	//basically only executes if the end of the file is reached and there was an ongoing material being created
	if (ongoingMat) {
		if (matData.NormalTextureMapSRV) {
			generatedMaterialsMap.insert({ ongoingMatName, new Material(ongoingMatName, matData, vertexShadersMap["Normal"], pixelShadersMap["Normal"], sampler) });
		}
		else {
			generatedMaterialsMap.insert({ ongoingMatName, new Material(ongoingMatName, matData, vertexShadersMap["DEFAULT"], pixelShadersMap["DEFAULT"], sampler) });
		}
		matData = {};
		ongoingMat = false;
	}
	infile.close();
	return Utility::MESH_TYPE::GENERATED_MESH;
}

void Game::LoadScene(string sceneName)
{
	//remove all current entities loaded
	for (size_t i = 0; i < sceneEntities.size(); i++)
	{
		delete sceneEntities[i];
	}
	sceneEntities.clear();
	sceneEntitiesMap.clear();
	utilizedMeshesMap.clear();
	utilizedMaterialsMap.clear();
	utilizedTexturesMap.clear();

	//for iterating over each line to get the float values for transformations
	regex iteratorRegex = regex("-\\d*\\.\\d*|\\d*\\.\\d*|-\\d+|\\d+");

	ifstream infile("../../Assets/Scenes/" + sceneName + ".txt");
	string line;
	smatch match;
	float parsedNumbers[9];
	string objName;
	while (getline(infile, line))
	{
		//cout << line << endl;
		if (line != "") {
			//if the line does not start with "//"
			if (!regex_match(line, regex("//.*"))) {
				//search for OBJ name at start of line
				regex_search(line, match, regex("^(\\S+)"));
				objName = match[0];

				//load mesh, material, and textures, and if they already exist then mark them as utilized
				Utility::MESH_TYPE meshType = AutoLoadOBJMTL(objName);

				Entity* someEntity;

				//naming of entity internally
				string entityName = objName; //temporary, should have entity name in scene file
				int sameNameEntityCnt = 1;
				while (sceneEntitiesMap.count(entityName)) {
					entityName = objName + " (" + to_string(sameNameEntityCnt) + ")";
					sameNameEntityCnt++;
				}

				//figure out what map to pull from
				switch (meshType) {
				case Utility::LOAD_FAILURE:
					continue;
				case Utility::DEFAULT_MESH:
					someEntity = new Entity(entityName, defaultMeshesMap[objName]);
					break;
				case Utility::GENERATED_MESH: {
					someEntity = new Entity(entityName, generatedMeshesMap[objName]);

					//generated meshes should have a list of required materials,
					//add them if they do or add the default (just black) if they dont
					vector<string> requiredMaterials = someEntity->GetMaterialNameList();
					for (int i = 0; i < requiredMaterials.size(); i++)
					{
						string requiredMat = requiredMaterials[i];
						if (generatedMaterialsMap.count(requiredMat))
							someEntity->AddMaterial(generatedMaterialsMap[requiredMat]);
					}
					if (requiredMaterials.size() == 0) {
						someEntity->AddMaterial(defaultMaterialsMap["DEFAULT"]);
						someEntity->AddMaterialNameToMesh("DEFAULT");
					}
					break;
				}
				default:
					break;
				}
				
				//get the transformation data associated with this entity
				line = regex_replace(line, regex("^(\\S+ )"), "");
				std::sregex_iterator iter(line.begin(), line.end(), iteratorRegex);
				int counter = 0;
				for (; iter != std::sregex_iterator(); ++iter) {
					if (counter < 9) {
						match = *iter;
						parsedNumbers[counter] = std::stof(match.str());
					}
					counter++;
				}
				someEntity->SetPosition(parsedNumbers[0], parsedNumbers[1], parsedNumbers[2]);
				someEntity->SetRotation(DirectX::XMConvertToRadians(parsedNumbers[3]), DirectX::XMConvertToRadians(parsedNumbers[4]), DirectX::XMConvertToRadians(parsedNumbers[5]));
				someEntity->SetScale(parsedNumbers[6], parsedNumbers[7], parsedNumbers[8]);
				someEntity->CalcWorldMatrix();

				if (meshType == Utility::DEFAULT_MESH)
					BuildDefaultEntity(entityName, objName, someEntity);

				//finally add the entity to the appropriate lists
				sceneEntitiesMap.insert({ entityName,someEntity });
				sceneEntities.push_back(someEntity);
			}
		}
	}

	infile.close();

	//clean up memory from prior scene, wont reload any resources that already exist and that
	//are needed, but will remove unused resources in the current scene
	vector<string> meshesToDelete;
	for (auto meshMapIter = generatedMeshesMap.begin(); meshMapIter != generatedMeshesMap.end(); ++meshMapIter)
	{
		if (!utilizedMeshesMap.count(meshMapIter->first)) {
			meshesToDelete.push_back(meshMapIter->first);
		}
	}
	for (size_t i = 0; i < meshesToDelete.size(); i++)
	{
		delete generatedMeshesMap[meshesToDelete[i]];
		generatedMeshesMap.erase(meshesToDelete[i]);
	}

	vector<string> texturesToDelete;
	for (auto texMapIter = generatedTexturesMap.begin(); texMapIter != generatedTexturesMap.end(); ++texMapIter)
	{
		if (!utilizedTexturesMap.count(texMapIter->first)) {
			texturesToDelete.push_back(texMapIter->first);
		}
	}
	for (size_t i = 0; i < texturesToDelete.size(); i++)
	{
		delete generatedTexturesMap[texturesToDelete[i]];
		generatedTexturesMap.erase(texturesToDelete[i]);
	}

	vector<string> materialsToDelete;
	for (auto matMapIter = generatedMaterialsMap.begin(); matMapIter != generatedMaterialsMap.end(); ++matMapIter)
	{
		if (!utilizedMaterialsMap.count(matMapIter->first)) {
			materialsToDelete.push_back(matMapIter->first);
		}
	}
	for (size_t i = 0; i < materialsToDelete.size(); i++)
	{
		delete generatedMaterialsMap[materialsToDelete[i]];
		generatedMaterialsMap.erase(materialsToDelete[i]);
	}
}

void Game::OnResize()
{
	DXCore::OnResize();
	camera->UpdateProjectionMatrix(width, height);
}

void Game::Update(float deltaTime, float totalTime)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	if (GetAsyncKeyState('F') & 0x8000) {
		DirectX::XMFLOAT3 rot = sceneEntitiesMap["sphere1"]->GetRotation();
		rot.y += DirectX::XMConvertToRadians(2.0f);
		sceneEntitiesMap["sphere1"]->SetRotation(rot.x,rot.y,rot.z);
		sceneEntitiesMap["sphere1"]->CalcWorldMatrix();
	}

	if (GetAsyncKeyState('G') & 0x8000) {
		DirectX::XMFLOAT3 rot = sceneEntitiesMap["sphere1"]->GetRotation();
		rot.y -= DirectX::XMConvertToRadians(2.0f);
		sceneEntitiesMap["sphere1"]->SetRotation(rot.x, rot.y, rot.z);
		sceneEntitiesMap["sphere1"]->CalcWorldMatrix();
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		DirectX::XMFLOAT3 trans = sceneEntitiesMap["Rock"]->GetPosition();
		trans.x -= 0.1f;
		sceneEntitiesMap["Rock"]->SetPosition(trans.x, trans.y, trans.z);
		sceneEntitiesMap["Rock"]->CalcWorldMatrix();
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		DirectX::XMFLOAT3 trans = sceneEntitiesMap["Rock"]->GetPosition();
		trans.x += 0.1f;
		sceneEntitiesMap["Rock"]->SetPosition(trans.x, trans.y, trans.z);
		sceneEntitiesMap["Rock"]->CalcWorldMatrix();
	}
	if (GetAsyncKeyState(VK_UP))
	{
		DirectX::XMFLOAT3 trans = sceneEntitiesMap["Rock"]->GetPosition();
		trans.z += 0.1f;
		sceneEntitiesMap["Rock"]->SetPosition(trans.x, trans.y, trans.z);
		sceneEntitiesMap["Rock"]->CalcWorldMatrix();
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		DirectX::XMFLOAT3 trans = sceneEntitiesMap["Rock"]->GetPosition();
		trans.z -= 0.1f;
		sceneEntitiesMap["Rock"]->SetPosition(trans.x, trans.y, trans.z);
		sceneEntitiesMap["Rock"]->CalcWorldMatrix();
	}

	if (sceneEntitiesMap["Rock"]->CheckSATCollision(sceneEntitiesMap["Rock (1)"])) 
	{
		cout << "colliding" << endl;
	}

	camera->Update();
	water->Update();
	/*if (!GetAsyncKeyState(VK_CONTROL))
	{
		testLight->Position = camera->position;
		testLight->Direction = camera->direction;
	}*/
}

void Game::Draw(float deltaTime, float totalTime)
{
	renderer->ClearFrame();

	renderer->SendAllLightsToShader(pixelShadersMap["DEFAULT"]);
	renderer->SendAllLightsToShader(pixelShadersMap["Normal"]);
	renderer->SendAllLightsToShader(pixelShadersMap["Water"]);
	renderer->SendAllLightsToShader(pixelShadersMap["Terrain"]);

	renderer->RenderShadowMap();

	renderer->RenderFrame();

	DrawSky();

	renderer->PresentFrame();
}


void Game::DrawSky() {
	ID3D11Buffer* vb = defaultMeshesMap["Cube"]->GetVertexBuffer();
	ID3D11Buffer* ib = defaultMeshesMap["Cube"]->GetIndexBuffer();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

	vertexShadersMap["Sky"]->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShadersMap["Sky"]->SetMatrix4x4("projection", camera->GetProjMatrix());
	vertexShadersMap["Sky"]->CopyAllBufferData();
	vertexShadersMap["Sky"]->SetShader();

	pixelShadersMap["Sky"]->SetShaderResourceView("Sky", skySRV);
	pixelShadersMap["Sky"]->SetSamplerState("BasicSampler", sampler);
	pixelShadersMap["Sky"]->SetShader();

	context->RSSetState(skyRasterState);
	context->OMSetDepthStencilState(skyDepthState, 0);

	context->DrawIndexed(defaultMeshesMap["Cube"]->GetIndexCount(), 0, 0);

	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}

#pragma region Mouse Input
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	prevMousePos.x = x;
	prevMousePos.y = y;

	SetCapture(hWnd);
}

void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{

	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (buttonState & 0x0001) {
		camera->RotateCamera(x - (int)prevMousePos.x, y - (int)prevMousePos.y);

		prevMousePos.x = x;
		prevMousePos.y = y;
	}
}

void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	
}
#pragma endregion
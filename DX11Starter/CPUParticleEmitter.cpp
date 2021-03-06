#include "pch.h"
#include "CPUParticleEmitter.h"

DefaultCPUParticleShaders CPUParticleEmitter::defaultShaders = {};

using namespace DirectX;

void CPUParticleEmitter::SetDefaultShaders(DefaultCPUParticleShaders s)
{
	defaultShaders = s;
}

CPUParticleEmitter::CPUParticleEmitter() : ParticleEmitter()
{
	InitBuffers();
	ParticleEmitter::particleSysType = PARTICLESYSTYPE::CPU;
}

CPUParticleEmitter::CPUParticleEmitter(ParticleEmitterDescription d, bool blendingEnabled) : ParticleEmitter(d)
{
	this->blendingEnabled = blendingEnabled;
	InitBuffers();
	ParticleEmitter::particleSysType = PARTICLESYSTYPE::CPU;
}

CPUParticleEmitter::~CPUParticleEmitter()
{
	indexBuffer->Release();
	vertexBuffer->Release();
	additiveBlend->Release();
	depthWriteOff->Release();
	if (collisionsEnabled) {
		//ghostCallback.
		//delete ghostCallback;
		for (size_t i = 0; i < maxParticles; i++)
		{
			if (particlePhysicsPool[i].enabled) {
				Config::DynamicsWorld->removeCollisionObject((btCollisionObject*)particlePhysicsPool[i].ghostObject);
				//Config::DynamicsWorld->updateSingleAabb(particlePhysicsPool[i].ghostObject);
			}
			delete particlePhysicsPool[i].collShape;
			particlePhysicsPool[i].collShape = nullptr;
			particlePhysicsPool[i].ghostObject->setUserPointer(nullptr);
			delete particlePhysicsPool[i].ghostObject;
			particlePhysicsPool[i].ghostObject = nullptr;
		}
		delete[] particlePhysicsPool;
	}
	delete[] particlePool;
	delete[] particleVertices;
	delete[] deadList;
	delete[] indices;
}

void CPUParticleEmitter::InitBuffers()
{
	// DYNAMIC vertex buffer (no initial data necessary)
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticles;
	Config::Device->CreateBuffer(&vbDesc, 0, &vertexBuffer);

	// Index buffer data
	indices = new unsigned int[(size_t)maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Regular (static) index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ibDesc.Usage = D3D11_USAGE_DYNAMIC;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	Config::Device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	particlePool = new Particle[maxParticles];
	ZeroMemory(particlePool, sizeof(Particle) * maxParticles);
	deadList = new unsigned int[maxParticles];
	particleVertices = new ParticleVertex[4 * (size_t)maxParticles];
	ZeroMemory(particleVertices, sizeof(ParticleVertex) * 4 * maxParticles);
	particleVertCount = 0;
	drawCount = 0;
	deadListCount = maxParticles;

	for (size_t i = 0; i < maxParticles; i++)
	{
		deadList[i] = i;
		//particleVertices[i + 0].UV = XMFLOAT2(0, 0);
		//particleVertices[i + 1].UV = XMFLOAT2(1, 0);
		//particleVertices[i + 2].UV = XMFLOAT2(1, 1);
		//particleVertices[i + 3].UV = XMFLOAT2(0, 1);
		//
		//particleVertices[i + 0].ID = 0;
		//particleVertices[i + 1].ID = 1;
		//particleVertices[i + 2].ID = 2;
		//particleVertices[i + 3].ID = 3;
	}

	// Additive blend state
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Config::Device->CreateBlendState(&blend, &additiveBlend);

	D3D11_DEPTH_STENCIL_DESC depth = {};
	depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth.DepthFunc = D3D11_COMPARISON_LESS;
	depth.DepthEnable = true;
	Config::Device->CreateDepthStencilState(&depth, &depthWriteOff);
}

void CPUParticleEmitter::CalcVertex(Particle p, XMFLOAT4X4 view)
{
	//particleVertices[i + 0].ID = 0;
	//particleVertices[i + 1].ID = 1;
	//particleVertices[i + 2].ID = 2;
	//particleVertices[i + 3].ID = 3;

	particleVertices[particleVertCount].Position = p.position;
	particleVertices[particleVertCount].Color = p.color;
	particleVertices[particleVertCount].Scale = p.scale;
	particleVertices[particleVertCount].UV = XMFLOAT2(0, 0);
	particleVertices[particleVertCount].ID = 0;
	particleVertices[particleVertCount].TextureIndex = p.textureIndex;
	particleVertices[particleVertCount].RotationRadians = p.rotationRadians;
	particleVertices[particleVertCount].Transparency = p.transparency;
	particleVertices[particleVertCount].WorldMatBaked = p.worldMatBaked;
	particleVertCount++;

	particleVertices[particleVertCount].Position = p.position;
	particleVertices[particleVertCount].Color = p.color;
	particleVertices[particleVertCount].Scale = p.scale;
	particleVertices[particleVertCount].UV = XMFLOAT2(1, 0);
	particleVertices[particleVertCount].ID = 1;
	particleVertices[particleVertCount].TextureIndex = p.textureIndex;
	particleVertices[particleVertCount].RotationRadians = p.rotationRadians;
	particleVertices[particleVertCount].Transparency = p.transparency;
	particleVertices[particleVertCount].WorldMatBaked = p.worldMatBaked;
	particleVertCount++;

	particleVertices[particleVertCount].Position = p.position;
	particleVertices[particleVertCount].Color = p.color;
	particleVertices[particleVertCount].Scale = p.scale; 
	particleVertices[particleVertCount].UV = XMFLOAT2(1, 1);
	particleVertices[particleVertCount].ID = 2;
	particleVertices[particleVertCount].TextureIndex = p.textureIndex;
	particleVertices[particleVertCount].RotationRadians = p.rotationRadians;
	particleVertices[particleVertCount].Transparency = p.transparency;
	particleVertices[particleVertCount].WorldMatBaked = p.worldMatBaked;
	particleVertCount++;

	particleVertices[particleVertCount].Position = p.position;
	particleVertices[particleVertCount].Color = p.color;
	particleVertices[particleVertCount].Scale = p.scale;
	particleVertices[particleVertCount].UV = XMFLOAT2(0, 1);
	particleVertices[particleVertCount].ID = 3;
	particleVertices[particleVertCount].TextureIndex = p.textureIndex;
	particleVertices[particleVertCount].RotationRadians = p.rotationRadians;
	particleVertices[particleVertCount].Transparency = p.transparency;
	particleVertices[particleVertCount].WorldMatBaked = p.worldMatBaked;
	particleVertCount++;
}

void CPUParticleEmitter::SetBlendingEnabled(bool toggle)
{
	this->blendingEnabled = toggle;
}

void CPUParticleEmitter::SetCollisionsEnabled(void(*collisionCallback)(void* ptr))
{
	if (collisionsEnabled) {
		this->collisionCallback = collisionCallback;
		return;
	};

	collisionsEnabled = true;

	particlePhysicsPool = new ParticlePhysics[maxParticles];
	ZeroMemory(particlePhysicsPool, sizeof(ParticlePhysics) * maxParticles);

	for (unsigned int i = 0; i < maxParticles; i++)
	{
		Particle p = particlePool[i];
		particlePhysicsPool[i].collShape = new btBoxShape(btVector3(0.01f, 0.01f, 0.01f));//btVector3(btScalar(p.scale), btScalar(p.scale), btScalar(p.scale)));

		btTransform transform;
		transform.setIdentity();

		particlePhysicsPool[i].ghostObject = new btGhostObject();
		particlePhysicsPool[i].ghostObject->setCollisionFlags(particlePhysicsPool[i].ghostObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);//
		particlePhysicsPool[i].ghostObject->setCollisionShape(particlePhysicsPool[i].collShape);
		particlePhysicsPool[i].ghostObject->setWorldTransform(transform);


		particlePhysicsPool[i].particleWrapper = { i, (void*)&particlePool[i], (void*)this };
		particlePhysicsPool[i].wrapper = { PHYSICS_WRAPPER_TYPE::PARTICLE, &particlePhysicsPool[i].particleWrapper, collisionCallback };
		particlePhysicsPool[i].ghostObject->setUserPointer(&particlePhysicsPool[i].wrapper);
		particlePhysicsPool[i].enabled = false;
	}
	//ghostCallback = new btGhostPairCallback();
	//Config::DynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(ghostCallback);

	/*

	Config::DynamicsWorld->setInternalTickCallback([](btDynamicsWorld* world, btScalar timeStep) {
		btGhostObject* ghost = (btGhostObject*)world->getWorldUserInfo();
		for (int j = 0; j < ghost->getNumOverlappingObjects(); j++)
		{
			btCollisionObject* other = ghost->getOverlappingObject(j);
			if (other->getInternalType() == btCollisionObject::CO_RIGID_BODY) {
				PhysicsWrapper* wrapper = (PhysicsWrapper*)ghost->getUserPointer();
				PhysicsWrapper* otherWrapper = (PhysicsWrapper*)other->getUserPointer();

				ParticlePhysicsWrapper* particleWrapper = (ParticlePhysicsWrapper*)wrapper->objectPointer;

				if (otherWrapper->type == PHYSICS_WRAPPER_TYPE::ENTITY) {
					//int index = ((ParticlePhysicsWrapper*)((PhysicsWrapper*)ghost->getUserPointer())->objectPointer)->particleIndex;
					std::cout << particleWrapper->particleIndex << " hit------------------------------------------------------------" << std::endl;
					//ParticleCollision coll = { particleWrapper->particleIndex, particleWrapper->particle, particleWrapper->emitter, otherWrapper->objectPointer };
					//collisionCallback(&coll);
				}
				//btRigidBody* pRigidBody = dynamic_cast<btRigidBody*>(ghost->getOverlappingObject(i));
				// do whatever you want to do with these pairs of colliding objects
			}
		}
		}, particlePhysicsPool[i].ghostObject, true);

	*/
}

void CPUParticleEmitter::KillParticle(unsigned int index)
{
	particlePool[index].remainingLife = 0.0f;
	particlePool[index].transparency = 0.0f;
	particlePool[index].color.w = 0.0f;
	if (collisionsEnabled) {
		if (particlePhysicsPool[index].enabled) {
			Config::DynamicsWorld->removeCollisionObject((btCollisionObject*)particlePhysicsPool[index].ghostObject);
			particlePhysicsPool[index].enabled = false;
			deadList[deadListCount] = index;
			deadListCount++;
		}
	}
}

void CPUParticleEmitter::Destroy()
{
	if (collisionsEnabled) {
		for (size_t i = 0; i < maxParticles; i++)
		{
			if (particlePhysicsPool[i].enabled) {
				Config::DynamicsWorld->removeCollisionObject((btCollisionObject*)particlePhysicsPool[i].ghostObject);
				//Config::DynamicsWorld->updateSingleAabb(particlePhysicsPool[i].ghostObject);
				particlePhysicsPool[i].enabled = false;
			}
		}
	}
	ParticleEmitter::Destroy();
}

void CPUParticleEmitter::Update(double deltaTime, double totalTime, XMFLOAT4X4 view)
{
	//if (deltaTime > 0.25f) return;
	ParticleEmitter::Update(deltaTime, totalTime);

	particleVertCount = 0;
	drawCount = 0;

	// Track time
	while ((emitTimeCounter >= emissionRate) && isActive && isAlive)
	{
		// How many to emit?
		int emitCount = (int)(emitTimeCounter / emissionRate);

		// Max to emit in a single batch is 65,535
		emitCount = min(emitCount, 65535);

		// Adjust time counter
		emitTimeCounter = fmod(emitTimeCounter, emissionRate);

		float randNum1;
		float randNum2;
		float randNum3;
		float randNum4;
		float randNum5;
		float randNum6;
		float randNum7;
		float randNum8;
		unsigned int newParticleIndex;
		float speed;
		float randomOffset;
		float randomOffset2;
		XMFLOAT3 start;
		XMFLOAT3 end;

		for (size_t i = 0; i < emitCount; i++)
		{
			randNum1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum4 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum5 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum6 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum7 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			randNum8 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			// Grab a single index from the dead list
			if (deadListCount == 0) break;
			newParticleIndex = deadList[deadListCount - 1];
			deadListCount--;

			// Update it in the particle pool
			Particle newParticle;

			newParticle.color = XMFLOAT4(1, 0, 0, 1);

			// Color and position depend on the grid position and size
			bool isColor = false;
			for (int j = 0; j < colorCount; j++)
			{
				if (randNum1 <= colors[j].weight) {
					newParticle.color = colors[j].color;
					newParticle.originalTransparency = colors[j].color.w;
					isColor = true;
					break;
				}
			}
			if (!isColor) {
				for (int j = 0; j < textureCount; j++)
				{
					if (randNum1 <= textures[j].weight) {
						newParticle.textureIndex = j;
						newParticle.transparency = textures[j].transparency;
						newParticle.originalTransparency = textures[j].transparency;
						break;
					}
				}
			}

			speed = particleInitMinSpeed + (particleInitMaxSpeed - particleInitMinSpeed) * randNum2;
			randomOffset = randNum3 * 2.0f - 1.0f;
			randomOffset2 = randNum4 * 2.0f - 1.0f;

			//float dirRand = RandNums.Consume();
			//float3 start = normalize(float3(randomOffset, randomOffset2, 0.0f)) * (emissionStartRadius * dirRand);
			//float3 end = float3(normalize(float2(randomOffset, randomOffset2)) * (emissionEndRadius * dirRand), 1.0f);

			//start = XMFLOAT3(randomOffset * emissionStartRadius, randomOffset2 * emissionStartRadius, 0.0f);
			XMStoreFloat3(&start, XMVectorScale((XMVector3Normalize(XMVectorSet(randomOffset, randomOffset2, 0, 0))), emissionStartRadius * randNum5));
			//end = XMFLOAT3(randomOffset * emissionEndRadius, randomOffset2 * emissionEndRadius, 1.0f);
			XMStoreFloat3(&end, XMVectorScale((XMVector2Normalize(XMVectorSet(randomOffset, randomOffset2, 0, 0))), emissionEndRadius * randNum5));
			end.z = 1.0f;

			newParticle.remainingLife = particleMinLifetime + (particleMaxLifetime - particleMinLifetime) * randNum6;
			newParticle.originalRemainingLife = newParticle.remainingLife;
			newParticle.position = start;
			newParticle.scale = particleInitMinScale + (particleInitMaxScale - particleInitMinScale) * randNum7;
			DirectX::XMStoreFloat3(&newParticle.velocity, XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&end), XMLoadFloat3(&start))), speed));
			newParticle.rotationRadians = 0.0f;
			newParticle.angularVelocity = particleInitMinAngularVelocity + (particleInitMaxAngularVelocity - particleInitMinAngularVelocity) * randNum8;
			newParticle.acceleration = particleAcceleration;

			newParticle.worldMatBaked = (int)bakeWorldMatOnEmission;
			if (bakeWorldMatOnEmission == true) {
				XMVECTOR pos = XMLoadFloat3(&newParticle.position);
				XMVECTOR vel = XMLoadFloat3(&newParticle.velocity);
				XMVECTOR accel = XMLoadFloat3(&newParticle.acceleration);
				pos.m128_f32[3] = 1.0f;
				vel.m128_f32[3] = 0.0f;
				accel.m128_f32[3] = 0.0f;
				XMMATRIX world = XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix));
				XMStoreFloat3(&newParticle.position, XMVector4Transform(pos, world));
				XMStoreFloat3(&newParticle.velocity, XMVector4Transform(vel, world));
				//XMStoreFloat3(&newParticle.acceleration, XMVector4Transform(accel, world));
			}

			// Put it back
			particlePool[newParticleIndex] = newParticle;

			if (collisionsEnabled) {
				btTransform transform;
				transform.setIdentity();
				XMVECTOR particlePos = XMLoadFloat3(&newParticle.position);
				particlePos.m128_f32[3] = 1.0f;
				XMVECTOR pos = (newParticle.worldMatBaked) ? particlePos : XMVector4Transform(particlePos, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
				btVector3 transformPos = btVector3(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
				transform.setOrigin(transformPos);

				particlePhysicsPool[newParticleIndex].ghostObject->setWorldTransform(transform);

				//if (particlePhysicsPool[newParticleIndex].ghostObject->getBroadphaseHandle())
					//Config::DynamicsWorld->updateSingleAabb(particlePhysicsPool[newParticleIndex].ghostObject);

				if (!particlePhysicsPool[newParticleIndex].enabled) {
					Config::DynamicsWorld->addCollisionObject(particlePhysicsPool[newParticleIndex].ghostObject);
					particlePhysicsPool[newParticleIndex].enabled = true;
				}
			}
			//drawList[drawListCount] = newParticleIndex;
			//drawListCount++;
		}
	}

	if (isAlive) {
		for (size_t i = 0; i < maxParticles; i++)
		{
			Particle particle = particlePool[i];

			bool isDead = particle.remainingLife <= 0.0f;

			// Early out for ALREADY DEAD particles (so they don't go back on dead list)
			if (isDead) {
				if (collisionsEnabled) {
					if (particlePhysicsPool[i].enabled) {
						deadList[deadListCount] = i;
						deadListCount++;
						Config::DynamicsWorld->removeCollisionObject((btCollisionObject*)particlePhysicsPool[i].ghostObject);
						//Config::DynamicsWorld->updateSingleAabb(particlePhysicsPool[i].ghostObject);
						particlePhysicsPool[i].enabled = false;
					}
				}
				//continue;
			}
			else {
				// Update the particle
				particle.remainingLife -= deltaTime;
				XMVECTOR vel = XMLoadFloat3(&particle.velocity);
				DirectX::XMStoreFloat3(&particle.velocity, XMVectorAdd(vel, XMVectorScale(XMLoadFloat3(&particle.acceleration), deltaTime)));
				DirectX::XMStoreFloat3(&particle.position, XMVectorAdd(XMLoadFloat3(&particle.position), XMVectorScale(vel, deltaTime)));
				particle.rotationRadians += particle.angularVelocity * deltaTime;

				if (fadeIn) {
					float totalLifetime = particle.originalRemainingLife - particle.remainingLife;
					if (fadeInEndTime > 0.0f && (totalLifetime < fadeInEndTime)) {
						particle.transparency = min((totalLifetime / fadeInEndTime), 1.0f) * particle.originalTransparency;
						particle.color.w = particle.transparency;
					}
				}

				if (fadeOut) {
					if (fadeOutStartTime > 0.0f && (particle.remainingLife < fadeOutStartTime)) {
						particle.transparency = (particle.remainingLife / fadeOutStartTime) * particle.originalTransparency;
					}
					else if (fadeOutStartTime <= 0.0f) {
						particle.transparency = (particle.remainingLife / particleAvgLifetime) * particle.originalTransparency;
						particle.color.w = particle.transparency;
					}
				}

				// Put the particle back
				particlePool[i] = particle;

				// Newly dead?
				if (particle.remainingLife <= 0.0f)
				{
					deadList[deadListCount] = i;
					deadListCount++;

					if (collisionsEnabled) {
						if (particlePhysicsPool[i].enabled) {
							Config::DynamicsWorld->removeCollisionObject((btCollisionObject*)particlePhysicsPool[i].ghostObject);
							//Config::DynamicsWorld->updateSingleAabb(particlePhysicsPool[i].ghostObject);
							particlePhysicsPool[i].enabled = false;
						}
					}
				}
				else if (collisionsEnabled)
				{
					if (particlePhysicsPool[i].enabled) {
						btTransform transform;
						transform.setIdentity();
						XMVECTOR particlePos = XMLoadFloat3(&particle.position);
						particlePos.m128_f32[3] = 1.0f;
						XMVECTOR pos = (particle.worldMatBaked) ? particlePos : XMVector4Transform(particlePos, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
						btVector3 transformPos = btVector3(pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);
						transform.setOrigin(transformPos);

						particlePhysicsPool[i].ghostObject->setWorldTransform(transform);
					}
					
					//particlePhysicsPool[i].collShape->setLocalScaling(btVector3(particle.scale, particle.scale, particle.scale));

					//if (particlePhysicsPool[i].ghostObject->getBroadphaseHandle())
						//Config::DynamicsWorld->updateSingleAabb(particlePhysicsPool[i].ghostObject);

					//if (i == 0) {
					//	//std::cout << pos.m128_f32[0] << " | " << pos.m128_f32[1] << " | " << pos.m128_f32[2] << std::endl;
					//	btTransform trans = particlePhysicsPool[i].rigidBody->getWorldTransform();
					//	btVector3 p = trans.getOrigin();
					//	std::cout << p.getX() << " | " << p.getY() << " | " << p.getZ() << std::endl;
					//}
				}
			}
			CalcVertex(particle, view);
			drawCount++;
		}
	}
}

void CPUParticleEmitter::Draw(XMFLOAT4X4 view, XMFLOAT4X4 proj)
{
	if (blendingEnabled)
	{
		Config::Context->OMSetBlendState(additiveBlend, 0, 0xFFFFFFFF);
		Config::Context->OMSetDepthStencilState(depthWriteOff, 0);
	}

	// Copy to dynamic buffer
	// All particles copied locally - send whole buffer to GPU
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	Config::Context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, particleVertices, sizeof(ParticleVertex) * 4 * maxParticles);
	Config::Context->Unmap(vertexBuffer, 0);

	// Set up buffers
	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	Config::Context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	Config::Context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	defaultShaders.particleVS->SetMatrix4x4("world", worldMatrix);
	defaultShaders.particleVS->SetMatrix4x4("view", view);
	defaultShaders.particleVS->SetMatrix4x4("projection", proj);
	defaultShaders.particleVS->SetShader();
	defaultShaders.particleVS->CopyAllBufferData();

	defaultShaders.particlePS->SetShaderResourceView("particleTextures", texturesSRV);
	defaultShaders.particlePS->SetSamplerState("Sampler", Config::Sampler);
	defaultShaders.particlePS->SetShader();

	// Draw the correct parts of the buffer
	Config::Context->DrawIndexed(maxParticles * 6, 0, 0);

	defaultShaders.particlePS->SetShaderResourceView("particleTextures", NULL);

	if (blendingEnabled)
	{
		Config::Context->OMSetBlendState(0, 0, 0xFFFFFFFF);
		Config::Context->OMSetDepthStencilState(0, 0);
	}
}


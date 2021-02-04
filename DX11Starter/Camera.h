#pragma once
#include "pch.h"

using namespace DirectX;
using namespace std;

class Camera
{
private:
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projMatrix;
public:
	XMFLOAT3 position;
	XMFLOAT3 direction;
	float xRotation;
	float yRotation;
	Camera();
	~Camera();
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjMatrix();
	void SetProjMatrix(XMFLOAT4X4 pm);
	void SetViewMatrix(XMFLOAT4X4 vm);
	void RotateCamera(int x, int y);
	void SetPosition(XMFLOAT3 pos);
	void UpdateProjectionMatrix(int w, int h);
	void Update();
};


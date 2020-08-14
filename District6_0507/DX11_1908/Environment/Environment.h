#pragma once

class Environment
{
private:
	Matrix projection;

	ViewProjectionBuffer* viewProjection;
	LightInfoBuffer* lightInfoBuffer;
	LightBuffer* light;

	ID3D11SamplerState* linearSamp;
	ID3D11SamplerState* pointSamp;

	Camera* mainCamera;
	Camera* subCamera;
	
	
	static Environment* instance;
	bool isFree;
	Environment();
	~Environment();
public:
	static Environment* Get() { return instance; }
	static void Create() { instance = NEW Environment(); }
	static void Delete() { delete instance; }
	void ChangeCam();
	void ChangeFPSCam();
	void ChangeFreeCam();
	void CreateViewport();
	void CreatePerspective();
	void CreateSamplerState();	

	ViewProjectionBuffer* GetVP() { return viewProjection; }
	Camera* MainCamera() { return mainCamera; }
	LightInfoBuffer* GetLightInfoBuffer() { return lightInfoBuffer; }
	LightBuffer* GetLight() { return light; }
	Matrix GetProjection() { return projection; }
	bool GetIsFreeCam() { return isFree; }
};
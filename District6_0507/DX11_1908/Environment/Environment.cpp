#include "Framework.h"
#include "Environment.h"

Environment* Environment::instance = nullptr;

Environment::Environment()
{
    isFree = false;
    viewProjection = NEW ViewProjectionBuffer();
    light = NEW LightBuffer();
    lightInfoBuffer = NEW LightInfoBuffer();
	
	CreateViewport();
	CreatePerspective();	
    CreateSamplerState();

    //mainCamera = NEW FreeCam();
   // mainCamera->position = Vector3(0, 0, -3);    
    //mainCamera = NEW FollowCam();
	mainCamera = NEW FPSCam();
    subCamera = NEW FreeCam();
}

Environment::~Environment()
{
	delete viewProjection;
    delete mainCamera;
    delete subCamera;
    delete lightInfoBuffer;
	SAFE_DELETE(light);
	
    linearSamp->Release();
    pointSamp->Release();
}

void Environment::ChangeCam()
{
    isFree = !isFree;
    Camera* cam = mainCamera;
    mainCamera = subCamera;
    subCamera = cam;
}

void Environment::ChangeFPSCam()
{
	if (!isFree)
		return;
	
	isFree = !isFree;
	Camera* cam = mainCamera;
	mainCamera = subCamera;
	subCamera = cam;
}

void Environment::ChangeFreeCam()
{
	if (isFree)
		return;

	isFree = !isFree;
	Camera* cam = mainCamera;
	mainCamera = subCamera;
	subCamera = cam;
}

void Environment::CreateViewport()
{
    D3D11_VIEWPORT vp;
    vp.Width = WIN_WIDTH;
    vp.Height = WIN_HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    DC->RSSetViewports(1, &vp);
}

void Environment::CreatePerspective()
{
    projection = XMMatrixPerspectiveFovLH(XM_PIDIV4,
        WIN_WIDTH / (float)WIN_HEIGHT, 0.5f, 1000.0f);

    viewProjection->SetProjection(projection);
}

void Environment::CreateSamplerState()
{
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    DEVICE->CreateSamplerState(&sampDesc, &linearSamp);

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

    DEVICE->CreateSamplerState(&sampDesc, &pointSamp);

    DC->PSSetSamplers(0, 1, &linearSamp);
    DC->PSSetSamplers(1, 1, &pointSamp);
}

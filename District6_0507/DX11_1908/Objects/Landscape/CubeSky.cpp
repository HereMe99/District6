#include "Framework.h"

CubeSky::CubeSky()
{
	sphere = NEW Sphere(L"CubeSky");
	sphere->GetFillMode()->FrontCounterClockwise(true);

	cubeMap = Texture::Add(L"Textures/sky2.dds");

	depthMode[0] = NEW DepthStencilState();
	depthMode[1] = NEW DepthStencilState();
	depthMode[1]->DepthEnable(false);
}

CubeSky::~CubeSky()
{
	delete sphere;

	delete depthMode[0];
	delete depthMode[1];
}

void CubeSky::Render()
{
	cubeMap->Set(24);

	depthMode[1]->SetState();

	sphere->Render();

	depthMode[0]->SetState();
}

#include "Framework.h"

Reflection::Reflection(Transform* transform)
	: transform(transform)
{
	renderTarget = NEW RenderTarget();
	depthStencil = NEW DepthStencil();	

	reflectionBuffer = NEW WorldBuffer();

	camera = NEW FixityCam();

	targetTexture = NEW Render2D();
	targetTexture->SetSRV(renderTarget->GetSRV());
	targetTexture->scale = Vector3(300, 300, 300);
	targetTexture->position = Vector3(150, 150, 0);
}

Reflection::~Reflection()
{
	delete renderTarget;
	delete depthStencil;
	delete reflectionBuffer;
	delete camera;
	delete targetTexture;
}

void Reflection::Update()
{
	targetTexture->Update();

	Vector3 R, T;
	R = CAMERA->rotation;
	T = CAMERA->position;

	R = Vector3(R.GetX() * -1.0f, R.GetY() * -1.0f, R.GetZ() * -1.0f);
	camera->rotation = R;

	Vector3 position = transform->position;
	
	T.SetY((position.GetY() * 2.0f) - T.GetY());
	camera->position = T;

	camera->Rotation();
	camera->View();

	reflectionBuffer->SetWorld(camera->GetView());
}

void Reflection::PreRender()
{
	renderTarget->Set(depthStencil);

	VP->SetView(camera->GetView());
	VP->SetVSBuffer(0);
}

void Reflection::Render()
{
	DC->PSSetShaderResources(10, 1, &renderTarget->GetSRV());	

	reflectionBuffer->SetVSBuffer(10);
}

void Reflection::PostRender()
{
	targetTexture->Render();
}

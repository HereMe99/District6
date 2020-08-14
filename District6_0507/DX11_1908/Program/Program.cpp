#include "Framework.h"
#include "Program.h"



Program::Program()
{
	
	SCENE->AddScene("Play", NEW PlayScene());
	SCENE->AddScene("Map", NEW MapToolScene());
	SCENE->AddScene("End", NEW EndScene());
	SCENE->AddScene("Title", NEW TitleScene());
	
}

Program::~Program()
{
	
}

void Program::Update()
{
	SCENE->Update();
	SOUND->Update();
	CAMERA->Update();	
	Keyboard::Get()->SetWheel(0.0f);
}

void Program::PreRender()
{
	VP->SetProjection(Environment::Get()->GetProjection());	
	VP->SetView(CAMERA->GetView());
	VP->SetVSBuffer(0);
	D_LIGHT->SetPSBuffer(0);
	LIGHT->SetPSBuffer(3);

	SCENE->PreRender();
}

void Program::Render()
{
	VP->SetProjection(Environment::Get()->GetProjection());
	VP->SetView(CAMERA->GetView());
	VP->SetVSBuffer(0);

	SCENE->Render();
}

void Program::PostRender()
{
	ImGui::Text("FPS : %d", (int)Timer::Get()->FPS());
	ImGui::Checkbox("Vsync", Device::Get()->Vsync());
	CAMERA->PostRender();
	ImGui::Text("Light");
	ImGui::SliderFloat3("LightDir", (float*)&D_LIGHT->data.direction, -1, 1);
	ImGui::SliderFloat("SpecExp", &D_LIGHT->data.specExp, 1.0f, 100.0f);
	ImGui::ColorEdit4("Ambient", (float*)&D_LIGHT->data.ambient);
	ImGui::ColorEdit4("AmbientFloor", (float*)&D_LIGHT->data.ambientFloor);
	ImGui::SliderFloat("Power", &D_LIGHT->data.lightPower, 1.0f, 100.0f);

	SCENE->PostRender();
}

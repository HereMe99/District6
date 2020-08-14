#include "Framework.h"
#include "TitleScene.h"

TitleScene::TitleScene()
{
	Texture* texture = Texture::Add(L"Textures/title2.png");
	bg = NEW Render2D;
	bg->SetTexture(texture);
	bg->scale.x = WIN_WIDTH;
	bg->scale.y = WIN_HEIGHT;
	bg->position.x = CENTER_X;
	bg->position.y = CENTER_Y;

	texture = Texture::Add(L"Textures/hp_back.png");
	for (UINT i = 0; i < 3; i++)
	{
		menu_back[i] = NEW Render2D();
		menu_back[i]->SetTexture(texture);
		menu_back[i]->scale.x = 400.0f;
		menu_back[i]->scale.y = 200.0f;
		menu_back[i]->position.x = WIN_WIDTH - 100.0f;
		menu_back[i]->position.y = WIN_HEIGHT * 0.8f - 150.f * i;
	}
	for (UINT i = 0; i < 3; i++)
	{
		menu[i] = NEW Render2D();
		menu[i]->scale.x = 200.0f;
		menu[i]->scale.y = 50.0f;
		menu[i]->position.x = WIN_WIDTH - 120.0f;
		menu[i]->position.y = WIN_HEIGHT * 0.8f - 150.f * i + 50;
	}
	menu[0]->SetTexture(Texture::Add(L"Textures/Play.png"));
	menu[1]->SetTexture(Texture::Add(L"Textures/MapTool.png"));
	menu[2]->SetTexture(Texture::Add(L"Textures/Quit.png"));
	ShowCursor(true);
	select_menu_num = -1;
	ENVIRONMENT->ChangeFreeCam();
}

TitleScene::~TitleScene()
{
	SAFE_DELETE(bg);
	SAFE_DELETE(menu_back[0]);
	SAFE_DELETE(menu_back[1]);
	SAFE_DELETE(menu_back[2]);
	SAFE_DELETE(menu[0]);
	SAFE_DELETE(menu[1]);
	SAFE_DELETE(menu[2]);
}

void TitleScene::Init()
{
	SOUND->Stop("bgm");
	SOUND->Stop("bgm2");
	ShowCursor(true);
	isGuiRender = false;
	select_menu_num = -1;
	ENVIRONMENT->ChangeFreeCam();
}

void TitleScene::Update()
{
	bg->Update();

	if (MOUSEPOS.y <= 127.f && MOUSEPOS.y >= 55.f
		&& MOUSEPOS.x >=1021)
	{
		select_menu_num = 0;
	}
	else if (MOUSEPOS.y <= 277.f && MOUSEPOS.y >= 205.f
		&& MOUSEPOS.x >= 1021)
	{
		select_menu_num = 1;
	}
	else if (MOUSEPOS.y <= 427.f && MOUSEPOS.y >= 355.f
		&& MOUSEPOS.x >= 1021)
	{
		select_menu_num = 2;
	}
	else
	{
		select_menu_num = -1;
	}
	if (KEY_DOWN(VK_UP))
	{
		if (select_menu_num > 0)
		{
			select_menu_num--;
		}
		else
		{
			select_menu_num = 0;
		}
	}
	if (KEY_DOWN(VK_DOWN))
	{
		if (select_menu_num < 2)
		{
			select_menu_num++;
		}
	}

	if (KEY_DOWN(VK_RETURN)||KEY_DOWN(VK_LBUTTON))
	{
		switch (select_menu_num)
		{
		case 0:
		{
			GoPlay();
		}
		break;

		case 1:
		{
			GoMapTool();
		}
		break;

		case 2:
		{
			DestroyWindow(hWnd);
		}
		break;
		default:
			break;
		}
	}
	for (UINT i = 0; i < 3; i++)
	{
		if (select_menu_num == i)
		{
			if (menu_back[i]->scale.x <= 550)
			{
				menu_back[i]->scale.x += 500.f * DELTA;
			}
			if (menu_back[i]->scale.x > 550)
			{
				menu_back[i]->scale.x = 550;
			}
		}
		else
		{
			if (menu_back[i]->scale.x >= 400)
			{
				menu_back[i]->scale.x -= 500.f * DELTA;
			}
			if (menu_back[i]->scale.x < 400)
			{
				menu_back[i]->scale.x = 400;
			}
		}
	}
	for (Render2D* r : menu_back)
	{
		r->Update();
	}
	for (Render2D* r : menu)
	{
		r->Update();
	}
}

void TitleScene::PreRender()
{
}

void TitleScene::Render()
{
}

void TitleScene::PostRender()
{
	bg->Render();
	for (Render2D* r : menu_back)
	{
		r->Render();
	}
	for (Render2D* r : menu)
	{
		r->Render();
	}
	ImGui::SliderFloat3("mouse", reinterpret_cast<float*>(&MOUSEPOS), 0, 2000);
}

void TitleScene::GoMapTool()
{
	SCENE->ChangeScene("Map");
	ENVIRONMENT->ChangeFreeCam();
}

void TitleScene::GoPlay()
{
	SCENE->ChangeScene("Play");
	ENVIRONMENT->ChangeFPSCam();
}

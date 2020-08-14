#include "Framework.h"
#include "EndScene.h"

EndScene::EndScene()
{
	Texture* texture = Texture::Add(L"Textures/gameover.png");
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
	menu[0]->SetTexture(Texture::Add(L"Textures/restart.png"));
	menu[1]->SetTexture(Texture::Add(L"Textures/title.png"));
	menu[2]->SetTexture(Texture::Add(L"Textures/Quit.png"));
	
	
}

EndScene::~EndScene()
{
	SAFE_DELETE(bg);
	SAFE_DELETE(menu_back[0]);
	SAFE_DELETE(menu_back[1]);
	SAFE_DELETE(menu_back[2]);
	SAFE_DELETE(menu[0]);
	SAFE_DELETE(menu[1]);
	SAFE_DELETE(menu[2]);
}

void EndScene::Init()
{
	SOUND->Stop("bgm");
	SOUND->Stop("bgm2");
	ShowCursor(true);
	isGuiRender = false;
	select_menu_num = -1;
	ENVIRONMENT->ChangeFreeCam();
}

void EndScene::Update()
{
	bg->Update();

	if (MOUSEPOS.y <= 127.f && MOUSEPOS.y >= 55.f
		&& MOUSEPOS.x >= 1021)
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

	if (KEY_DOWN(VK_RETURN) || KEY_DOWN(VK_LBUTTON))
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
			GoTitle();
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

void EndScene::PreRender()
{
}

void EndScene::Render()
{
}

void EndScene::PostRender()
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
	PlayScene::NumberTexturesRender(PlayScene::stageNum,
		{ WIN_WIDTH*0.6f,WIN_HEIGHT * 0.15f,0 }, { 200.f,200.f ,1 });

}

void EndScene::GoTitle()
{
	SCENE->ChangeScene("Title");
	ENVIRONMENT->ChangeFreeCam();
}

void EndScene::GoPlay()
{
	SCENE->ChangeScene("Play");
	ENVIRONMENT->ChangeFPSCam();
}

#include "Framework.h"
#include "PlayScene.h"

vector<BoxCollider*> PlayScene::colliders;
bool PlayScene::isPlay = true;
bool PlayScene::isClear = false;
int PlayScene::stageNum = -1;
NumberTexture* PlayScene::mNumberTextures[];
PlayScene::PlayScene()
{

	terrain = NEW Terrain();
	aStar = NEW AStar(40, 40);
	aStar->Setup(terrain);
	enemyManager = NEW EnemyManager(15);

	SetModel();
	SetMap();

	Texture* aim_texture = Texture::Add(L"Textures/aim2.png");
	render2D = NEW Render2D();
	render2D->SetTexture(aim_texture);
	render2D->position.x = CENTER_X;
	render2D->position.y = CENTER_Y;
	render2D->scale = { 50,50,1 };

	fpsObj = NEW FPSModel();
	enemyManager->SetTarget(fpsObj->GetColliderTransform());
	enemyManager->SetAStar(aStar);
	

	aStar->Update();
	for (ModelInstance* model : models)
	{
		model->Update();
	}
	for (BoxCollider* collider : colliders)
	{
		aStar->SetObstacleByBox(collider);
	}
	skybox = NEW CubeSky();

	Vector3 numberStartPos = { CENTER_X + 250.0f,WIN_HEIGHT * 0.1f,0 };

	for (UINT i = 0; i < 10; i++)
	{
		mNumberTextures[i] = NEW NumberTexture();
		mNumberTextures[i]->transform->position = numberStartPos;
		mNumberTextures[i]->transform->position.x -= 50 * i;
	}

	SOUND->Add("bgm", "Assets/Sounds/bgm.mp3", true);
	SOUND->Add("bgm2", "Assets/Sounds/bgm2.mp3", true);

	stage_loop = NEW Loop(100, false);
	shop_loop = NEW Loop(20, false);

	stage_loop->loopFunc = bind(&PlayScene::StageClear, this);
	shop_loop->loopFunc = bind(&PlayScene::StageStart, this);
}

PlayScene::~PlayScene()
{
	SAFE_DELETE(fpsObj);
	SAFE_DELETE(terrain);
	SAFE_DELETE(aStar);
	SAFE_DELETE(render2D);
	SAFE_DELETE(enemyManager);
	SAFE_DELETE(stage_loop);
	SAFE_DELETE(shop_loop);
	SAFE_DELETE(skybox);
	
	for(ModelInstance* model : models)
	{
		SAFE_DELETE(model);
	}
	
	for (NumberTexture* numberTexture : mNumberTextures)
	{
		SAFE_DELETE(numberTexture);
	}
}

void PlayScene::Init()
{
	ENVIRONMENT->ChangeFPSCam();
	D_LIGHT->data.ambient = Float4(0.05f, 0.05f, 0.05f, 1);
	D_LIGHT->data.ambientFloor = Float4(0.05f, 0.05f, 0.05f, 1);
	fpsObj->Init();
	enemyManager->StageClear();
	ShowCursor(false);
	isGuiRender = false;
	stageNum = 0;
	StageClear();
}

void PlayScene::Update()
{
	if(FPSModel::hp <= 0)
	{
		SCENE->ChangeScene("End");
	}
	if(KEY_DOWN(VK_F8))
	{
		stageNum = 9;
	}

	shop_loop->Update();
	stage_loop->Update();
	

	terrain->Update();
	aStar->Update();
	enemyManager->Update();
	fpsObj->Update();
	for (ModelInstance* model : models)
	{
		model->Update();
	}
	render2D->Update();

	if (fpsObj->GetIsAttack())
	{
		enemyManager->HitCollisionCheck(CAMERA->GetRay(), fpsObj->GetAtk());
		fpsObj->SetIsAttack(false);
	}
}

void PlayScene::PreRender()
{
}

void PlayScene::Render()
{
	
	skybox->Render();

	enemyManager->Render();
	terrain->Render();
	aStar->Render();
	for (ModelInstance* model : models)
	{
		model->Render();
	}

	if (!isPlay)
	{
		return;
	}
	fpsObj->Render();
}

void PlayScene::PostRender()
{
	enemyManager->PostRender();
	if (isClear)
	{
		int time = 20;
		time -= static_cast<int>(shop_loop->curTime);
		PlayScene::NumberTexturesRender(time,
			{ CENTER_X,WIN_HEIGHT * 0.9f,0 }, { 70.0f,70.0f ,1 });
	}
	else
	{
		int time = 100 - static_cast<int>(stage_loop->curTime);
		PlayScene::NumberTexturesRender(time,
			{ CENTER_X,WIN_HEIGHT * 0.9f,0 }, { 60.0f,60.0f ,1 });
	}
	NumberTexturesRender(stageNum,
		{ WIN_WIDTH*0.1f,WIN_HEIGHT * 0.9f,0 }, { 70.0f,70.0f ,1 });
	render2D->Render();
	fpsObj->PostRender();
}

vector<BoxCollider*> PlayScene::GetColliders()
{
	return colliders;
}

void PlayScene::SetNumberTextures(UINT number, Vector3 pos, Vector3 scale)
{
	string str;
	str = to_string(number);
	float numberScale = mNumberTextures[0]->numbers[0]->scale.x;
	for (int index = str.size() - 1; index >= 0; index--)
	{
		mNumberTextures[str.size() - 1 - index]->number_index = str.c_str()[index] - '0';
		if (str.size() % 2 == 0)
		{
			float resultPos = index - str.size() * 0.5f;
			float result = numberScale * 0.5f * scale.x + (numberScale * scale.x * resultPos);
			mNumberTextures[str.size() - 1 - index]->transform->scale = scale;
			mNumberTextures[str.size() - 1 - index]->transform->position.x = pos.x + result;
			mNumberTextures[str.size() - 1 - index]->transform->position.y = pos.y;
		}
		else
		{
			float resultPos = index - (str.size() - 1) * 0.5f;
			float result = (numberScale * scale.x * resultPos);
			mNumberTextures[str.size() - 1 - index]->transform->scale = scale;
			mNumberTextures[str.size() - 1 - index]->transform->position.x = pos.x + result;
			mNumberTextures[str.size() - 1 - index]->transform->position.y = pos.y;
		}
	}
	for (int index = str.size(); index < 10; index++)
	{
		mNumberTextures[index]->number_index = -1;
	}
	for (NumberTexture* number : mNumberTextures)
	{
		number->Update();
	}
}

void PlayScene::NumberTexturesRender(UINT number, Vector3 pos, Vector3 scale)
{
	SetNumberTextures(number, pos, scale);
	for (NumberTexture* number_texture : mNumberTextures)
	{
		number_texture->Render();
	}
}

void PlayScene::SetModel()
{
	ModelInstance* model = NEW ModelInstance("Hangar_v4");
	model->SetDiffuseMap(L"Textures/Section6/Hangar_v4.tga");
	models.push_back(model);

	model = NEW ModelInstance("Hangar_v1");
	model->SetDiffuseMap(L"Textures/Section6/Hangar_v1.tga");
	models.push_back(model);

	model = NEW ModelInstance("Barrel");
	model->SetDiffuseMap(L"Textures/Section6/Barrel_v1.tga");
	models.push_back(model);


	model = NEW ModelInstance("Dumpsters_v1");
	model->SetDiffuseMap(L"Textures/Section6/Dumpsters_v1.tga");
	models.push_back(model);

	model = NEW ModelInstance("Concrete_fence_v1");
	model->SetDiffuseMap(L"Textures/Section6/Concrete_fence_v1.tga");
	models.push_back(model);

	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 0,0,36.0f + i * 74.0f };
	}
	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 511,0,36.0f + i * 74.0f };
	}
	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 36.0f + i * 74.0f,0,0 };
		transform->rotation.y = XM_PI * 0.5f;
	}
	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 36.0f + i * 74.0f,0,511 };
		transform->rotation.y = XM_PI * 0.5f;
	}
}

void PlayScene::SetMap()
{
	enemyManager->LoadSpawnPoints();
	LoadMap();
}

void PlayScene::LoadMap()
{
	const string file = "SaveData/MapTool/mapToolData.xml";

	XmlDocument* document = NEW XmlDocument();
	document->LoadFile(file.c_str());

	XmlElement* root = document->FirstChildElement();

	XmlElement* objectsNode = root->FirstChildElement();
	XmlElement* objectNode = objectsNode->FirstChildElement();

	do
	{
		string name = objectNode->Name();
		Transform* transform = nullptr;
		BoxCollider* collider = nullptr;
		for (ModelInstance* model : models)
		{
			if (model->GetName() == name)
			{
				transform = model->AddTransform();
				collider = model->GetLastCollider();
			}
		}
		colliders.push_back(collider);

		XmlElement* node = objectNode->FirstChildElement();
		transform->position.x = node->FloatAttribute("X");
		transform->position.y = node->FloatAttribute("Y");
		transform->position.z = node->FloatAttribute("Z");
		node = node->NextSiblingElement();

		transform->scale.x = node->FloatAttribute("X");
		transform->scale.y = node->FloatAttribute("Y");
		transform->scale.z = node->FloatAttribute("Z");
		node = node->NextSiblingElement();

		transform->rotation.x = node->FloatAttribute("X");
		transform->rotation.y = node->FloatAttribute("Y");
		transform->rotation.z = node->FloatAttribute("Z");

		objectNode = objectNode->NextSiblingElement();

	} while (objectNode != nullptr);

	XmlElement* collidersNode = objectsNode->NextSiblingElement();
	XmlElement* colliderNode = collidersNode->FirstChildElement();

	UINT index = 0;
	do
	{
		Transform* transform;
		Vector3 scale;
		XmlElement* scaleNode = colliderNode->FirstChildElement();
		scale.x = scaleNode->FloatAttribute("X");
		scale.y = scaleNode->FloatAttribute("Y");
		scale.z = scaleNode->FloatAttribute("Z");
		models[index]->SetCollidersScale(scale);

		Vector3 offset;
		XmlElement* offsetNode = scaleNode->NextSiblingElement();
		offset.x = offsetNode->FloatAttribute("X");
		offset.y = offsetNode->FloatAttribute("Y");
		offset.z = offsetNode->FloatAttribute("Z");
		models[index]->SetCollidersOffset(offset);

		colliderNode = colliderNode->NextSiblingElement();
		index++;
	} while (colliderNode != nullptr);


	delete document;

}

void PlayScene::StageClear()
{
	shop_loop->StartLoop();
	enemyManager->StageClear();
	fpsObj->ClearStage();
	stageNum++;
	isClear = true;
	
	SOUND->Stop("bgm2");
	SOUND->Play("bgm", 0.5f);
}

void PlayScene::StageStart()
{
	stage_loop->StartLoop();
	isClear = false;
	
	SOUND->Stop("bgm");
	SOUND->Play("bgm2", 0.3f);
}

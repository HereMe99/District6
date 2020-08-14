#include "Framework.h"
#include "EnemyManager.h"

EnemyManager::EnemyManager(UINT enemyNum)
{
	isAttack = false;
	max_enemy_num = enemyNum;
	cur_enemy_num = 0;
	
	AniInstance* model = NEW AniInstance("NormalZombie/NormalZombie");
	model->AddAnimation("NormalZombie/ZombieIdle", true);
	model->AddAnimation("NormalZombie/NormalZombie", true);
	model->AddAnimation("NormalZombie/ZombieAttack", true);
	model->AddAnimation("NormalZombie/ZombieHit", false);
	model->AddAnimation("NormalZombie/ZombieDeath", false);
	model->GetIntBuffer()->data.indices[0] = 0;
	models.push_back(model);

	model = NEW AniInstance("LightZombie/ZombieRun");
	model->AddAnimation("LightZombie/ZombieIdle", true);
	model->AddAnimation("LightZombie/ZombieRun", true);
	model->AddAnimation("LightZombie/ZombieAttack", true);
	model->AddAnimation("LightZombie/ZombieHit", false);
	model->AddAnimation("LightZombie/ZombieDeath", false);
	model->GetIntBuffer()->data.indices[0] = 0;
	models.push_back(model);

	model = NEW AniInstance("HeavyZombie/ZombieIdle");
	model->AddAnimation("HeavyZombie/ZombieIdle", true);
	model->AddAnimation("HeavyZombie/ZombieWalking", true);
	model->AddAnimation("HeavyZombie/ZombieAttack", true);
	model->AddAnimation("HeavyZombie/ZombieHit", false);
	model->AddAnimation("HeavyZombie/ZombieDeath", false);
	//model->GetIntBuffer()->data.indices[0] = 0;
	models.push_back(model);
	
	for (UINT i = 0; i < enemyNum; i++)
	{
		Enemy* enemy = NEW Enemy(EnemyKind::NORMAL_ZOMBIE);
		enemy->SetModel(models[0]);
		enemy->GetTransform()->scale = { 0.06f,0.06f,0.06f };
		models[0]->SetAniSpeed(i, 1.5f);
		enemys.push_back(enemy);
	}
	for (UINT i = 0; i < enemyNum; i++)
	{
		Enemy* enemy = NEW Enemy(EnemyKind::LIGHT_ZOMBIE);
		enemy->SetModel(models[1]);
		enemy->GetTransform()->scale = { 0.06f,0.06f,0.06f };
		//models[1]->SetAniSpeed(i, 1.0f);
		enemys.push_back(enemy);
	}
	for (UINT i = 0; i < enemyNum; i++)
	{
		Enemy* enemy = NEW Enemy(EnemyKind::HEAVY_ZOMBIE);
		enemy->SetModel(models[2]);
		enemy->GetTransform()->scale = { 0.06f,0.06f,0.06f };
		models[2]->SetAniSpeed(i, 2.f);
		enemys.push_back(enemy);
	}

	enemy_spawn_loop = NEW Loop(2.0f, true);
	enemy_spawn_loop->loopFunc = bind(&EnemyManager::SpawnEnemy, this);
	enemy_spawn_loop->StartLoop();

	hit = NEW Render2D();
	Texture* texture = Texture::Add(L"Textures/hit.png");
	hit->SetTexture(texture);

	hit->position = { CENTER_X,CENTER_Y,0 };
	hit->scale = { WIN_WIDTH,WIN_HEIGHT,0 };
}

EnemyManager::~EnemyManager()
{
	for (Enemy* enemy : enemys)
	{
		SAFE_DELETE(enemy);
	}

	for (EnemySpawnPoint* spawnPoint : spawnPoints)
	{
		SAFE_DELETE(spawnPoint);
	}
	for (AniInstance* model : models)
	{
		SAFE_DELETE(model);
	}
	SAFE_DELETE(hit);
	SAFE_DELETE(enemy_spawn_loop);
}

void EnemyManager::Update()
{
	for (AniInstance* model : models)
	{
		model->Update();
	}

	isAttack = false;
	if (PlayScene::isClear)
		return;
	for (Enemy* enemy : enemys)
	{
		enemy->Update();
		if (enemy->GetIsAttack())
		{
			isAttack = true;
		}
	}
	enemy_spawn_loop->Update();
	hit->Update();
}

void EnemyManager::Render()
{
	for (AniInstance* model : models)
	{
		model->Render();
	}

	if (PlayScene::isClear)
		return;
	
	for (Enemy* enemy : enemys)
	{
		enemy->Render();
	}
	for (EnemySpawnPoint* spawnPoint : spawnPoints)
	{
		spawnPoint->Render();
	}
}

void EnemyManager::PostRender()
{
	if(isAttack)
	{
		hit->Render();
	}
}

void EnemyManager::SpawnEnemy()
{
	if (cur_enemy_num >= max_enemy_num)
		return;

	while (true)
	{
		int spawnNum = Random(0, spawnPoints.size() - 1);
		int enemyNum = Random(0, enemys.size() - 1);
		if (Distance(spawnPoints[spawnNum]->position, FPSModel::position) >= 50)
		{
			if (enemys[enemyNum]->Activate(spawnPoints[spawnNum]->position))
			{
				enemy_spawn_loop->isLimit = Random(0.0f, 1.0f);
				cur_enemy_num++;
				return;
			}
		}
	}


}

void EnemyManager::SetTarget(Transform* target)
{
	for (Enemy* enemy : enemys)
	{
		enemy->SetTarget(target);
	}
}

void EnemyManager::SetAStar(AStar* aStar)
{
	this->aStar = aStar;
	for (Enemy* enemy : enemys)
	{
		enemy->SetAStar(aStar);
	}
}


void EnemyManager::AddSpawnPoint(Vector3 position)
{
	SphereCollider* sphere_collider = NEW SphereCollider(2, 5, 5);
	spawnPoints.push_back(NEW EnemySpawnPoint(position));
}

void EnemyManager::SaveSpawnPoints()
{
	const string savePath = "SaveData/SpawnPoints/spawnPointData.xml";
	Path::CreateFolders(savePath);
	Xml::XMLDocument* document = NEW Xml::XMLDocument();

	Xml::XMLElement* root = document->NewElement("SpawnPoints");
	document->LinkEndChild(root);

	for (EnemySpawnPoint* spawnPoint : spawnPoints)
	{
		Xml::XMLElement* node = document->NewElement("SpawnPoint");
		root->LinkEndChild(node);

		Xml::XMLElement* element = document->NewElement("position");
		element->SetAttribute("X", spawnPoint->position.x);
		element->SetAttribute("Y", spawnPoint->position.y);
		element->SetAttribute("Z", spawnPoint->position.z);
		node->LinkEndChild(element);
	}

	document->SaveFile(savePath.c_str());

	delete document;
}

void EnemyManager::LoadSpawnPoints()
{
	const string file = "SaveData/SpawnPoints/spawnPointData.xml";

	XmlDocument* document = NEW XmlDocument();
	document->LoadFile(file.c_str());

	XmlElement* root = document->FirstChildElement();
	XmlElement* spawnNode = root->FirstChildElement();
	
	if(spawnNode == nullptr)
	{
		return;
	}
	
	do
	{
		Vector3 position;
		XmlElement* node = spawnNode->FirstChildElement();
		position.x = node->FloatAttribute("X");
		position.y = node->FloatAttribute("Y");
		position.z = node->FloatAttribute("Z");

		EnemySpawnPoint* spawnPoint = NEW EnemySpawnPoint(position);
		spawnPoints.push_back(spawnPoint);

		spawnNode = spawnNode->NextSiblingElement();

	} while (spawnNode != nullptr);

	delete document;
}

void EnemyManager::SpawnPointsUpdate()
{
	for (EnemySpawnPoint* spawn_point : spawnPoints)
	{
		spawn_point->Update();
	}
}

void EnemyManager::SpawnPointsRender()
{
	for (EnemySpawnPoint* spawn_point : spawnPoints)
	{
		spawn_point->Render();
	}
}


bool EnemyManager::HitCollisionCheck(Ray ray, float atk)
{
	float min = FLT_MAX;
	int hit_index = -1;
	//UINT index = 0;
	for (int i = 0; i < enemys.size(); i++)
	{
		if (!enemys[i]->GetIsActive() || enemys[i]->GetIsDeath())
		{
			continue;
		}
		float distance = enemys[i]->HitCollisionCheck(ray);
		if (distance == -1.f)
		{
			continue;
		}
		if (distance < min)
		{
			min = distance;
			hit_index = i;
		}
	}
	if (hit_index == -1)
		return false;

	if (enemys[hit_index]->Hit(atk))
	{
		cur_enemy_num--;
	}

	return true;
}

void EnemyManager::StageClear()
{
	cur_enemy_num = 0;

	for (Enemy* enemy : enemys)
	{
		enemy->StageClear();
	}
}

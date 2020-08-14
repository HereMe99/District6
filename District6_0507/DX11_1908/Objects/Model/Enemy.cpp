#include "Framework.h"
#include "Enemy.h"

Enemy::Enemy(const EnemyKind enemyKind)
	: enemyKind(enemyKind), distanceToTarget(0.0f), isAttack(false), isHit(false), isActive(false)
{
	switch (enemyKind)
	{
	case NORMAL_ZOMBIE:
		moveSpeed = 17.f;
		hp = 5;
		attackRange = 10.0f;
		recognitionRange = 150.0f;
		collider = NEW SphereCollider(3.0f);
		break;
	case LIGHT_ZOMBIE:
		moveSpeed = 30.f;
		hp = 3;
		attackRange = 10.0f;
		recognitionRange = 200.0f;
		collider = NEW SphereCollider(2.0f);
		break;
	case HEAVY_ZOMBIE:
		moveSpeed = 10.f;
		hp = 10;
		attackRange = 10.0f;
		recognitionRange = 160.0f;
		collider = NEW SphereCollider(4.0f);
		break;
	default:;
	}
	pathFindLoop = NEW Loop(2.0f, true);
	deathLoop = NEW Loop(5.0f, false);
	soundLoop = NEW Loop(2.0f, true);
	deathLoop->loopFunc = bind(&Enemy::Deactivate, this);
	pathFindLoop->loopFunc = bind(&Enemy::PathFind, this);
	soundLoop->loopFunc = bind(&Enemy::SoundLoop, this);
	pathFindLoop->StartLoop();
	soundLoop->StartLoop();
	isMove = false;
	isDeath = false;
	SOUND->Add("zombieIdle", "Assets/Sounds/zombie_idle.wav", false);
	SOUND->Add("zombieAttack", "Assets/Sounds/zombieAttack.ogg", false);
	SOUND->Add("zombieHit", "Assets/Sounds/zombieHit.wav", false);
}

Enemy::~Enemy()
{
	SAFE_DELETE(soundLoop);
	SAFE_DELETE(collider);
	SAFE_DELETE(pathFindLoop);
	SAFE_DELETE(deathLoop);
	
}

void Enemy::Update()
{
	if (!isActive)
		return;
	deathLoop->Update();

	if (isDeath)
		return;
	soundLoop->Update();
	pathFindLoop->Update();

	distanceToTarget = Distance(transform->position, targetTransform->position);
	CheckAction();
	collider->Update();
}

void Enemy::Render()
{
	if (!isActive)
		return;

	collider->Render();
}


void Enemy::SetTransform(Transform* transform)
{
	this->transform = transform;
	collider->SetTarget(transform);
}

void Enemy::SetAStar(AStar* aStar)
{
	this->aStar = aStar;
}

void Enemy::SetTerrain(Terrain* terrain)
{
	this->terrain = terrain;
}

void Enemy::SetTarget(Transform* targetTransform)
{
	this->targetTransform = targetTransform;
}

void Enemy::SetModel(AniInstance* model)
{
	this->model = model;
	this->transform = model->AddTransform();
	collider->SetTarget(transform);
	switch (enemyKind)
	{
	case NORMAL_ZOMBIE:
		model->GetLastCollider()->scale = { 0.3f,1.0f,0.3f };
		hit_collider = model->GetLastCollider();
		break;
	case LIGHT_ZOMBIE:
		model->GetLastCollider()->scale = { 0.3f,1.0f,0.3f };
		hit_collider = model->GetLastCollider();
		break;
	case HEAVY_ZOMBIE:
		model->GetLastCollider()->scale = { 0.3f,1.0f,0.3f };
		hit_collider = model->GetLastCollider();
		break;
	default:;
	}
	instanceIndex = model->GetInstanceSize() - 1;
}



Transform* Enemy::GetTransform()
{
	return transform;
}

bool Enemy::Activate(Vector3 spawnPos)
{
	if (isActive)
	{
		return false;
	}
	isActive = true;
	isHit = false;
	isAttack = false;
	isMove = false;
	isDeath = false;
	switch (enemyKind)
	{
	case NORMAL_ZOMBIE:
		moveSpeed = 15.f + PlayScene::stageNum;
		hp = 5 + PlayScene::stageNum * 2.f;
		attackRange = 10.0f;
		recognitionRange = 150.0f;
		break;
	case LIGHT_ZOMBIE:
		moveSpeed = 25.f + PlayScene::stageNum*2.f;
		hp = 3 + PlayScene::stageNum * 1.5f;
		attackRange = 10.0f;
		recognitionRange = 200.0f;
		break;
	case HEAVY_ZOMBIE:
		moveSpeed = 10.0f + PlayScene::stageNum;
		hp = 10 + PlayScene::stageNum * 5;
		attackRange = 10.0f;
		recognitionRange = 160.0f;
		break;
	default:;
	}
	model->PlayClip(instanceIndex, 0);
	pathFindLoop->StartLoop();
	transform->position = spawnPos;
	return true;
}

void Enemy::Deactivate()
{
	isActive = false;
	isAttack = false;
}

void Enemy::StageClear()
{
	if (!isActive || isDeath)
		return;

	isDeath = true;
	isAttack = false;
	deathLoop->StartLoop();
	model->PlayClip(instanceIndex, 4, 5.f, 5.f);
}

void Enemy::CheckAction()
{	
	if (distanceToTarget <= recognitionRange || isHit)
	{
		if (distanceToTarget <= attackRange)
		{
			//공격 상태
			isMove = false;
			Attack();
		}
		else
		{
			//목적지로 이동
			isAttack = false;
			Move();
		}
	}
	else
	{
		//대기상태
		isMove = false;
		isAttack = false;
		if (!isHit && !isMove && !isAttack)
			model->PlayClip(instanceIndex, 0);
	}
	
}

void Enemy::Attack()
{
	FPSModel::hp -= 20.f * DELTA + 5.f * DELTA * static_cast<float>(PlayScene::stageNum);
	isAttack = true;
	model->PlayClip(instanceIndex, 2);
}

void Enemy::Move()
{
	if (path.empty())
		return;
	if (!isMove)
	{
		model->PlayClip(instanceIndex, 1);
	}
	Vector3 dest = path.back();

	Vector3 direction = dest - transform->position;
	velocity = direction.Normal();
	transform->rotation.y = -(float)atan2(velocity.z, velocity.x);
	transform->rotation.y -= XM_PI * 0.5f;

	if (direction.Length() < 0.3f)
		path.pop_back();


	float magnitude = velocity.Length();

	if (magnitude > 1.0f)
		velocity.Normalize();

	if (magnitude > 0.1f)
	{
		transform->position += velocity * moveSpeed * DELTA;

		velocity = XMVectorLerp(velocity.data, XMVectorZero(), 3.0f * DELTA);
	}
	else
	{

	}
	isMove = true;
	CollisionCheck();
}

void Enemy::CollisionCheck()
{

	for (BoxCollider* objCollider : PlayScene::GetColliders())
	{
		objCollider->CollisionPush(collider);
	}

}

float Enemy::HitCollisionCheck(Ray ray) const
{
	float distance = 0.f;
	if (hit_collider->IsCollision(ray, &distance))
	{
		return distance;
	}
	return -1.0f;
}

void Enemy::PathFind()
{
	if (distanceToTarget > recognitionRange)
	{
		return;
	}


	path.clear();
	Vector3 destPos = targetTransform->position;
	destPos.y = 0.0f;
	//terrain->ComputePicking(&destPos);

	Ray ray;
	ray.position = transform->position;
	ray.direction = (destPos - transform->position).Normal();

	const float distance = Distance(ray.position, destPos);

	if (aStar->IsCollisionObstacle(ray, distance))
	{
		int startIndex = aStar->FindCloseNode(transform->position);
		int endIndex = aStar->FindCloseNode(destPos);

		aStar->Reset();

		path = aStar->FindPath(startIndex, endIndex);

		aStar->MakeDirectPath(transform->position, destPos, path);

		path.insert(path.begin(), destPos);

		int pathSize = path.size();

		while (path.size() > 2)
		{
			vector<Vector3> tempPath;
			for (int i = 1; i < path.size() - 1; i++)
				tempPath.push_back(path[i]);

			Vector3 start = path.back();
			Vector3 end = path.front();

			aStar->MakeDirectPath(start, end, tempPath);

			path.clear();
			path.push_back(end);

			for (Vector3 temp : tempPath)
				path.push_back(temp);

			path.push_back(start);

			if (pathSize == path.size())
				break;
			else
				pathSize = path.size();
		}
	}
	else
	{
		pathFindLoop->curTime = pathFindLoop->limitTime;
		path.push_back(destPos);
	}
}

void Enemy::SoundLoop()
{
	if (isAttack)
	{
		SOUND->Play("ZombieAttack", 1.0f);
	}
	else
	{
		float v = recognitionRange - distanceToTarget;
		v /= recognitionRange * 1.5f;
		SOUND->Play("zombieIdle", v);
	}
}

bool Enemy::Hit(float atk)
{
	hp -= atk;
	isHit = true;
	isMove = false;
	isAttack = false;
	SOUND->Play("zombieHit");
	if (hp <= 0.f)
	{
		isDeath = true;
		deathLoop->StartLoop();
		model->PlayClip(instanceIndex, 4, 5.f, 5.f);
		FPSModel::money += 100;
		return true;
	}
	else
	{
		model->PlayClip(instanceIndex, 3, 5.f, 5.f);
		return false;
	}
}


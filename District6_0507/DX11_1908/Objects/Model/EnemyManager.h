#pragma once

struct EnemySpawnPoint
{
	Vector3 position;
	SphereCollider* sphereCollider;
	EnemySpawnPoint(Vector3 spawnPoint)
		:position(spawnPoint)
	{
		sphereCollider = NEW SphereCollider(2, 5, 5);
		sphereCollider->position = spawnPoint;
	}
	~EnemySpawnPoint()
	{
		SAFE_DELETE(sphereCollider);
	}

	void Update() const
	{
		sphereCollider->SetColor(Float4(1, 0, 1, 1));
		sphereCollider->Update();
	}

	void Render() const
	{
		sphereCollider->Render();
	}

};


class EnemyManager
{
public:
	EnemyManager(UINT enemyNum);
	~EnemyManager();

	void Update();
	void Render();
	void PostRender();
	void SpawnEnemy();
	void SetTarget(Transform* target);
	void SetAStar(AStar* aStar);
	void AddSpawnPoint(Vector3 position);
	void SaveSpawnPoints();
	void LoadSpawnPoints();
	void SpawnPointsUpdate();
	void SpawnPointsRender();
	

	
	bool HitCollisionCheck(Ray ray, float atk);
	void StageClear();
	void StageStart();
private:

	int max_enemy_num;
	int cur_enemy_num;
	Render2D* hit;
	vector<Enemy*> enemys;
	vector<EnemySpawnPoint*> spawnPoints;
	vector<AniInstance*> models;

	AStar* aStar;
	Loop* enemy_spawn_loop;
	bool isAttack;
	//bool is_clear;
};

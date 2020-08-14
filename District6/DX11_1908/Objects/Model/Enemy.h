#pragma once

enum EnemyKind
{
	NORMAL_ZOMBIE,
	LIGHT_ZOMBIE,
	HEAVY_ZOMBIE
};

class Enemy
{
public:
	Enemy(EnemyKind enemyKind);
	~Enemy();

	void Update();
	void Render();

	void SetTransform(Transform* transform);
	void SetAStar(AStar* aStar);
	void SetTerrain(Terrain* terrain);
	void SetTarget(Transform* targetTransform);
	void SetModel(AniInstance* model);
	
	Transform* GetTransform();
	bool Activate(Vector3 spawnPos);
	void Deactivate();
	float HitCollisionCheck(Ray ray) const;
	bool Hit(float atk);
	bool GetIsActive() const { return isActive; }
	bool GetIsDeath() const { return isDeath; }
	void StageClear();
	bool GetIsAttack() const { return isAttack; }
private:

	void CheckAction();
	void Attack();
	void Move();
	void CollisionCheck();
	void PathFind();
	void SoundLoop();
public:
	EnemyKind enemyKind;
	
private:
	Vector3 velocity;
	AStar* aStar;
	Terrain* terrain;
	SphereCollider* collider;
	BoxCollider* hit_collider;
	AniInstance* model;
	Transform* targetTransform;
	Transform* transform;
	vector<Vector3> path;
	Loop* pathFindLoop;
	Loop* deathLoop;
	Loop* soundLoop;
	
	float distanceToTarget;
	float attackRange;
	float recognitionRange;
	float moveSpeed;
	float max_hp;
	float hp;

	//bool isAttack;
	bool isMove;
	bool isAttack;
	bool isHit;
	bool isActive;
	bool isDeath;
	
	UINT instanceIndex;
};

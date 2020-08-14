#pragma once


class AStar;

class Kaya
{
private:
	enum AnimState
	{
		IDLE,
		RUN,
		ATTACK
	}state;

	Transform* transform;
	Transform* headTransform;
	ModelAnimator* model;

	float moveSpeed;
	float rotSpeed;

	float accelation;
	float deceleration;

	Vector3 velocity;

	Vector3 destPos;
	vector<Vector3> path;

	ModelBone* headBone;
	Terrain* terrain;
	AStar* aStar;
	int light_info_num;
	
	
public:
	Kaya();
	~Kaya();

	void Update(); 
	void Render();

	Transform* GetTransform() { return transform; }
	Transform* GetHeadTransform() { return headTransform; }

	Model* GetModel() { return model->GetModel(); }

	void SetTerrain(Terrain* value) { terrain = value; }
	void SetAStar(AStar* value) { aStar = value; }
private:
	void Input();
	void Move();
	void MovePath();
	void Rotate();

	void SetAnimation(AnimState value);
	void SetIdle();

	void ReadData();
};
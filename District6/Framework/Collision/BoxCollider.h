#pragma once

struct Obb
{
	Vector3 position;

	Vector3 axisX;
	Vector3 axisY;
	Vector3 axisZ;

	Vector3 halfSize;
};

class BoxCollider : public Collider
{
private:
	Vector3 minBox;
	Vector3 maxBox;	

public:
	BoxCollider(Float3 min = Float3(-0.5f, -0.5f, -0.5f),
		Float3 max = Float3(0.5f, 0.5f, 0.5f));
	~BoxCollider();

	virtual bool IsCollision(Collider* collider) override;
	virtual bool IsCollision(IN Ray ray, OUT float* distance = nullptr, OUT Vector3* contact = nullptr) override;
	virtual void CollisionPush(Collider* collider) override;

	Vector3 GetMin();
	Vector3 GetMax();

	void SetBox(Float3 min, Float3 max);

	Obb GetObb();


	bool IsPointCollision(Vector3 position);
	bool IsBoxCollision(BoxCollider* collider);
	bool IsSphereCollision(class SphereCollider* collider);

	void IsBoxCollisionPush(BoxCollider* collider);
	void IsSphereCollisionPush(class SphereCollider* collider);
	
	bool SeperateAxis(Vector3 position, Vector3 direction, Obb box1, Obb box2);
};


#pragma once

struct Ray
{
	Vector3 position;
	Vector3 direction;
};

class Collider : public Transform
{
protected:
	Shader* shader;
	Transform* target;
	ColorBuffer* colorBuffer;

	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

	vector<Vertex> vertices;
	vector<UINT> indices;
	static bool is_dev_mode;
public:
	Vector3 offset;
	Collider();
	virtual ~Collider();

	virtual bool IsCollision(Collider* collider) = 0;
	virtual void CollisionPush(Collider* collider) = 0;
	virtual bool IsCollision(IN Ray ray, OUT float* distance = nullptr,
		OUT Vector3* contact = nullptr) = 0;
	static void ChangeDevMode();

	void Update();
	void Render();

	
	void SetColor(Float4 color) { colorBuffer->data.color = color; }

	inline void SetTarget(Transform* transform);
};
inline void Collider::SetTarget(Transform* transform)
{
	target = transform;
}

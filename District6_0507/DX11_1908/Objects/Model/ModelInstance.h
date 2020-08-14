#pragma once

class ModelInstance
{
private:
	string name;
	ModelRender* model;
	IntBuffer* intBuffer;
	vector<BoxCollider*> colliders;
public:
	ModelInstance(string name);
	~ModelInstance();

	void Update();
	void Render();

	static void Export(string name);
	inline void SetColliderBox(int boxIndex, Float3 min, Float3 max);

	inline IntBuffer* GetIntBuffer();
	inline string GetName();
	inline BoxCollider* GetLastCollider();
	inline void SetDiffuseMap(wstring fileName) const;
	inline void SetNormalMap(wstring fileName) const;
	inline void SetSpecularMap(wstring fileName) const;

	void SetCollidersScale(Vector3 scale);
	void SetCollidersOffset(Vector3 offset);
	
	inline Transform* AddTransform();
	void DeleteTransform(vector<BoxCollider*>& worldColliders, vector<Transform*>& worldTransforms);
};

inline IntBuffer* ModelInstance::GetIntBuffer()
{
	return intBuffer;
}

inline string ModelInstance::GetName()
{
	return name;
}

inline BoxCollider* ModelInstance::GetLastCollider()
{
	return colliders.back();
}

inline void ModelInstance::SetDiffuseMap(const wstring fileName) const
{
	model->GetModel()->SetDiffuseMap(fileName);
}

inline void ModelInstance::SetNormalMap(const wstring fileName) const
{
	model->GetModel()->SetNormalMap(fileName);
	intBuffer->data.indices[1] = 1;
}

inline void ModelInstance::SetSpecularMap(const wstring fileName) const
{
	model->GetModel()->SetSpecularMap(fileName);
	intBuffer->data.indices[0] = 1;
}
inline Transform* ModelInstance::AddTransform()
{
	Transform* transform = model->AddTransform();
	BoxCollider* collider = NEW BoxCollider({-10,-10,-10},{10,10,10});
	collider->SetTarget(transform);
	colliders.push_back(collider);
	return transform;
}



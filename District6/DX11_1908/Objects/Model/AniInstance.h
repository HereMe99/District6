#pragma once

class AniInstance
{
private:
	string name;
	AnimationModel* model;
	
	vector<BoxCollider*> colliders;
	vector<Transform*> transforms;
public:
	AniInstance(string name);
	~AniInstance();

	void Update();
	void Render();

	void Export(string name);

	void AddAnimation(string aniName,bool isLoop);

	inline void PlayClip(UINT instanceNum, UINT clipNum, 
		float fadeIn = 0.f, float fadeOut = 0.f);

	
	inline string GetName();
	inline BoxCollider* GetLastCollider();
	inline void SetDiffuseMap(wstring fileName,UINT matIndex) const;
	inline void SetNormalMap(wstring fileName, UINT matIndex) const;
	inline void SetSpecularMap(wstring fileName, UINT matIndex) const;
	inline IntBuffer* GetIntBuffer();
	
	inline UINT GetInstanceSize();
	inline void SetAniSpeed(UINT instanceID,float aniSpeed);
	
	inline Transform* AddTransform();
};
inline void AniInstance::PlayClip(UINT instanceNum, UINT clipNum, float fadeIn ,float fadeOut)
{
	model->PlayAni(instanceNum, clipNum, fadeIn, fadeOut);
}

inline string AniInstance::GetName()
{
	return name;
}

inline BoxCollider* AniInstance::GetLastCollider()
{
	return colliders.back();
}
inline IntBuffer* AniInstance::GetIntBuffer()
{
	return model->GetIntBuffer();
}

inline void AniInstance::SetDiffuseMap(wstring fileName, UINT matIndex) const
{
	model->SetDiffuseMap(fileName, matIndex);
}

inline void AniInstance::SetNormalMap(wstring fileName, UINT matIndex) const
{
	model->SetNormalMap(fileName, matIndex);
	
}

inline void AniInstance::SetSpecularMap(wstring fileName, UINT matIndex) const
{
	model->SetSpecularMap(fileName,matIndex);

}
inline UINT AniInstance::GetInstanceSize()
{
	return model->GetInstanceSize();
}

inline void AniInstance::SetAniSpeed(const UINT instanceID, const float aniSpeed)
{
	model->SetSpeed(instanceID, aniSpeed);
}



inline Transform* AniInstance::AddTransform()
{
	Transform* transform = model->AddInstance();
	transforms.push_back(transform);
	BoxCollider* collider = NEW BoxCollider({ -10,-10,-10 }, { 10,10,10 });
	collider->SetTarget(transform);
	colliders.push_back(collider);
	return transform;
}



#pragma once
class NumberTexture
{
public:
	Render2D* numbers[10];
	Transform* transform;
	int number_index;

	NumberTexture()
	{
		number_index = -1;
		transform = NEW Transform();
		for (UINT i = 0; i < 10; i++)
		{
			Texture* temp = Texture::Add(L"Textures/Number/number_" + to_wstring(i) + L".png");
			numbers[i] = NEW Render2D();
			numbers[i]->SetTexture(temp);
			numbers[i]->SetParent(transform->GetWorldPtr());
		}

	}
	~NumberTexture()
	{
		for (Render2D* number : numbers)
		{
			SAFE_DELETE(number);
		}
		SAFE_DELETE(transform);
	}
	void Update()
	{
		transform->UpdateWorld();
		for (Render2D* number : numbers)
		{
			number->Update();
		}
	}
	void Render()
	{
		if (number_index == -1)
		{
			return;
		}
		numbers[number_index]->Render();
	}
};

class PlayScene : public Scene
{
	
//Function
public:
	PlayScene();
	~PlayScene();
	void Init() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	static vector<BoxCollider*> GetColliders();
	static void SetNumberTextures(UINT number, Vector3 pos, Vector3 scale);
	static void NumberTexturesRender(UINT number, Vector3 pos, Vector3 scale);
private:
	void SetModel();
	void SetMap();
	void LoadMap();

	void StageClear();
	void StageStart();
////////////////////////////////////////
	
public:
	static int stageNum;
	static bool isPlay;
	static NumberTexture* mNumberTextures[10];
	static bool isClear;
private:
	CubeSky* skybox;
	Texture* aim_texture;
	AStar* aStar;
	EnemyManager* enemyManager;
	Render2D* render2D;
	Terrain* terrain;
	FPSModel* fpsObj;
	Loop* stage_loop;
	Loop* shop_loop;
	CubeSky* sky;
	vector<ModelInstance*> models;
	static vector<BoxCollider*> colliders;
};

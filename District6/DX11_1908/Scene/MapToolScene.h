#pragma once

struct MapToolSaveType
{
	Transform* transform;
	string name;
};

class MapToolScene : public Scene
{
private:
	AStar* aStar;
	//EnemyManager* enemyManager;
	Terrain* terrain;
	//FPSModel* fps;
	vector<ModelInstance*> models;
	vector<Transform*> transforms;
	vector<EnemySpawnPoint*> spawnPoints;
	vector<Vector3> colliderScales;
	vector<Vector3> colliderOffsets;
	vector<BoxCollider*> colliders;
	
	int selectObjectNum;
	vector<MapToolSaveType> saveTypes;

	Transform* lastTransform;
	
	BoxCollider* selectCollider;
	//map<string,vector<Transform*>> saveMaps;
	bool objMode;
	
	float selectRotation;
	bool isSetObstacle;
	
public:
	MapToolScene();   
	~MapToolScene();

	

	// Scene을(를) 통해 상속됨
	void Init() override;
	virtual void Update() override;
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	
	vector<BoxCollider*> GetColliders();
private:
	void SetModels();
	void SaveSpawnPoints();
	void LoadSpawnPoints();
	void SaveMap();
	void LoadMap();
};


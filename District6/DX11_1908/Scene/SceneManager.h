#pragma once

class SceneManager
{
private:
	Scene* currentScene;
	string currentSceneName;

	map<string, Scene*>totalScene;

	static SceneManager* instance;

	SceneManager();
	~SceneManager();
public:
	static SceneManager* GetInstance()
	{
		if (instance == nullptr)
			instance = NEW SceneManager;

		return instance;
	}

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void Release();

	Scene* AddScene(string key, Scene* scene);
	void ChangeScene(string key);
	Scene* FindScene(string key);
	string GetSceneName() { return currentSceneName; }
};


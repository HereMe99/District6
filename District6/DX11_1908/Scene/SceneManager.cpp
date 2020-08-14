#include "Framework.h"
#include "SceneManager.h"

SceneManager* SceneManager::instance = nullptr;

SceneManager::SceneManager()
{
	currentScene = nullptr;
	currentSceneName = "";
}


SceneManager::~SceneManager()
{
}

void SceneManager::Update()
{
	if (currentScene != nullptr)
		currentScene->Update();
}

void SceneManager::PreRender()
{
	if (currentScene != nullptr)
		currentScene->PreRender();
}

void SceneManager::Render()
{
	if (currentScene != nullptr)
		currentScene->Render();
}

void SceneManager::PostRender()
{
	if (currentScene != nullptr)
		currentScene->PostRender();
}


void SceneManager::Release()
{
	for (auto scene : totalScene)
	{
		delete scene.second;
	}
	totalScene.clear();

	delete instance;
}

Scene* SceneManager::AddScene(string key, Scene* scene)
{
	if (!scene)
		return nullptr;

	totalScene.insert({ key,scene });
	currentScene = scene;
	return scene;
}

void SceneManager::ChangeScene(string key)
{
	auto scene = totalScene.find(key);

	if (scene == totalScene.end())
		return;

	if (scene->second == currentScene)
		return;

	scene->second->Init();

	if (currentScene != nullptr)
		currentScene->Release();

	currentScene = scene->second;
	currentSceneName = key;
}

Scene* SceneManager::FindScene(string key)
{
	auto scene = totalScene.find(key);

	if (scene == totalScene.end())
		return nullptr;

	if (scene->second == currentScene)
		return nullptr;

	return scene->second;
}

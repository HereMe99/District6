#include "Framework.h"
#include "MapToolScene.h"

MapToolScene::MapToolScene()
	:terrain(NEW Terrain()), selectObjectNum(0), selectRotation(0.0f)
	, lastTransform(nullptr), selectCollider(nullptr), objMode(true)
{

	SetModels();

	aStar = NEW AStar(40, 40);
	aStar->Setup(terrain);

	isSetObstacle = false;



	for (BoxCollider* collider : colliders)
	{
		aStar->SetObstacleByBox(collider);
	}
	aStar->Update();
}

MapToolScene::~MapToolScene()
{
	delete terrain;

	SAFE_DELETE(aStar);

	for (ModelInstance* model : models)
	{
		delete model;
	}
	for (EnemySpawnPoint* spawn_point : spawnPoints)
	{
		SAFE_DELETE(spawn_point);
	}
}



void MapToolScene::Init()
{
	ENVIRONMENT->ChangeFreeCam();
	D_LIGHT->data.ambient = Float4(1, 1, 1, 1);
	D_LIGHT->data.ambientFloor = Float4(1, 1, 1, 1);
	isGuiRender = true;
}

void MapToolScene::Update()
{
	if (KEY_DOWN(VK_F7))
	{
		objMode = !objMode;
	}

	if (KEY_DOWN(VK_F3))
	{
		Collider::ChangeDevMode();
	}
	terrain->Update();
	aStar->Update();
	for (EnemySpawnPoint* spawn_point : spawnPoints)
	{
		spawn_point->Update();
	}

	if (KEY_DOWN(VK_RIGHT))
	{
		selectRotation += XM_PI * 0.5f;
		if (selectRotation > 2 * XM_PI)
		{
			selectRotation -= 2 * XM_PI;
		}
	}

	if (KEY_DOWN(VK_LEFT))
	{
		selectRotation -= XM_PI * 0.5f;
		if (selectRotation < 0)
		{
			selectRotation += 2 * XM_PI;
		}
	}

	if(KEY_UP(VK_LBUTTON))
	{
		isSetObstacle = true;
	}
	
	if (KEY_DOWN(VK_MBUTTON))
	{
		if (objMode)
		{
			Ray ray = CAMERA->GetRay();
			bool isCheck = false;
			float distance = 0.f;
			float min_distance = FLT_MAX;
			int index = 0;
			for (auto collider : colliders)
			{
				if (collider->IsCollision(ray, &distance))
				{
					if (distance < min_distance)
					{
						selectCollider = collider;
						lastTransform = transforms[index];
						min_distance = distance;
						isCheck = true;
					}
				}
				index++;
			}
			if (!isCheck)
			{
				
				Transform* transform = models[selectObjectNum]->AddTransform();
				transforms.push_back(transform);
				transform->rotation.y = selectRotation;
				selectCollider = models[selectObjectNum]->GetLastCollider();
				colliders.push_back(selectCollider);
				selectCollider->scale = colliderScales[selectObjectNum];
				selectCollider->offset = colliderOffsets[selectObjectNum];
				terrain->ComputePicking(&transform->position);
				lastTransform = transform;
				MapToolSaveType saveType;
				saveType.name = models[selectObjectNum]->GetName();
				saveType.transform = transform;
				saveTypes.push_back(saveType);
				isSetObstacle = true;
			}
		}
		else
		{
			Vector3 position;
			terrain->ComputePicking(&position);
			spawnPoints.push_back(NEW EnemySpawnPoint(position));
		}
	}

	if (KEY_DOWN(VK_UP))
	{
		if (selectObjectNum < models.size() - 1)
		{
			selectObjectNum++;
			lastTransform = nullptr;
			selectCollider = nullptr;
		}
	}

	if (KEY_DOWN(VK_DOWN))
	{
		if (selectObjectNum > 0)
		{
			selectObjectNum--;
			lastTransform = nullptr;
			selectCollider = nullptr;
		}
	}

	if (KEY_PRESS(VK_CONTROL) && KEY_DOWN('Z'))
	{
		models[selectObjectNum]->DeleteTransform(colliders, transforms);
		lastTransform = nullptr;
		for (UINT i = saveTypes.size(); i >= 0; i--)
		{
			if (saveTypes[i].name == models[selectObjectNum]->GetName())
			{
				saveTypes.erase(saveTypes.begin() + i);
				isSetObstacle = true;
				break;
			}
		}
	}
	if (lastTransform != nullptr)
	{
		if (KEY_PRESS('I'))
		{
			lastTransform->position.z += 10 * DELTA;
		}
		if (KEY_PRESS('K'))
		{
			lastTransform->position.z -= 10 * DELTA;
		}if (KEY_PRESS('J'))
		{
			lastTransform->position.x -= 10 * DELTA;
		}if (KEY_PRESS('L'))
		{
			lastTransform->position.x += 10 * DELTA;
		}

	}

	for (ModelInstance* model : models)
	{
		model->Update();
	}
	for (EnemySpawnPoint* spawn_point : spawnPoints)
	{
		spawn_point->Update();
	}

	if (isSetObstacle)
	{
		aStar->Init();
		for(auto collider:colliders)
		{
			aStar->SetObstacleByBox(collider);
		}
		isSetObstacle = false;
	}
}

void MapToolScene::PreRender()
{
}

void MapToolScene::Render()
{
	terrain->Render();
	aStar->Render();
	for (ModelInstance* model : models)
	{
		model->Render();
	}
	for (EnemySpawnPoint* spawn_point : spawnPoints)
	{
		spawn_point->Render();
	}
}

void MapToolScene::PostRender()
{
	terrain->PostRender();
	ImGui::Text("MapToolInfo");
	if (ImGui::Button("Save", ImVec2(50, 50)))
	{
		SaveMap();
	}
	if (lastTransform != nullptr)
	{
		ImGui::SliderFloat3("rot", reinterpret_cast<float*>(&lastTransform->rotation), 0.0f, 2.0f * XM_PI);
		ImGui::DragFloat3("pos", reinterpret_cast<float*>(&lastTransform->position), -100.0f, 100.0f);
		ImGui::SliderFloat3("colliderScale", reinterpret_cast<float*>(&colliderScales[selectObjectNum])
			, 0.0f, 5.0f);
		ImGui::SliderFloat3("colliderOffset", reinterpret_cast<float*>(&colliderOffsets[selectObjectNum]), -5.0f, 5.0f);
		selectCollider->scale = colliderScales[selectObjectNum];
		selectCollider->offset = colliderOffsets[selectObjectNum];
		//selectCollider->SetBox(minBox, maxBox);
	}
	ImGui::SliderFloat("selectRotation", &selectRotation, 0.0f, 2.0f * XM_PI);
	ImGui::SliderInt("selectObjectNum", &selectObjectNum, 0, models.size() - 1);
	ImGui::Button("Test", { 60,30 });

	for (UINT i = 0; i < LIGHT->data.lightCount; i++)
	{
		ImGui::SliderInt("Type" + i, (int*)&LIGHT->data.lights[i].type, 0, 3);
		ImGui::ColorEdit4("Color" + i, (float*)&LIGHT->data.lights[i].color);
		ImGui::SliderFloat3("Position" + i, (float*)&LIGHT->data.lights[i].position, -100, 100);
		ImGui::SliderFloat("Range" + i, &LIGHT->data.lights[i].range, 0, 100);
		ImGui::SliderFloat3("Direction" + i, (float*)&LIGHT->data.lights[i].direction, -1, 1);
		ImGui::SliderFloat("Outer" + i, &LIGHT->data.lights[i].outer, 0, 180);
		ImGui::SliderFloat("Inner" + i, &LIGHT->data.lights[i].inner, 0, 180);
		ImGui::SliderFloat("Length" + i, &LIGHT->data.lights[i].length, 0, 100);
		ImGui::SliderFloat("Power" + i, &LIGHT->data.lights[i].power, 1, 10);
	}
}

vector<BoxCollider*> MapToolScene::GetColliders()
{
	return colliders;
}

void MapToolScene::SetModels()
{
	ModelInstance* model = new ModelInstance("Hangar_v4");
	model->SetDiffuseMap(L"Textures/Section6/Hangar_v4.tga");
	models.push_back(model);
	colliderScales.push_back({ 1,1,1 });
	colliderOffsets.push_back({ 0,0,0 });

	model = new ModelInstance("Hangar_v1");
	model->SetDiffuseMap(L"Textures/Section6/Hangar_v1.tga");
	models.push_back(model);
	colliderScales.push_back({ 1,1,1 });
	colliderOffsets.push_back({ 0,0,0 });

	model = new ModelInstance("Barrel");
	model->SetDiffuseMap(L"Textures/Section6/Barrel_v1.tga");
	models.push_back(model);
	colliderScales.push_back({ 1,1,1 });
	colliderOffsets.push_back({ 0,0,0 });


	model = new ModelInstance("Dumpsters_v1");
	model->SetDiffuseMap(L"Textures/Section6/Dumpsters_v1.tga");
	models.push_back(model);
	colliderScales.push_back({ 1,1,1 });
	colliderOffsets.push_back({ 0,0,0 });

	model = new ModelInstance("Concrete_fence_v1");
	model->SetDiffuseMap(L"Textures/Section6/Concrete_fence_v1.tga");
	models.push_back(model);
	colliderScales.push_back({ 1,1,1 });
	colliderOffsets.push_back({ 0,0,0 });

	colliderScales[0] = { 1.667f, 3.651f, 3.413f };
	colliderScales[1] = { 3.9f, 4.2f, 3.0f };
	colliderScales[2] = { 0.159f, 0.5f, 0.159f };
	colliderScales[3] = { 1.27f, 0.873f,  0.9f };
	colliderScales[4] = { 0.159f, 1.27f, 3.8f };

	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 0,0,36.0f + i * 74.0f };
		MapToolSaveType saveType;
		saveType.name = model->GetName();
		saveType.transform = transform;
		model->GetLastCollider()->scale = colliderScales[4];
		//saveTypes.push_back(saveType);
	}
	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 511,0,36.0f + i * 74.0f };
		MapToolSaveType saveType;
		saveType.name = model->GetName();
		saveType.transform = transform;
		model->GetLastCollider()->scale = colliderScales[4];
		//saveTypes.push_back(saveType);
	}
	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 36.0f + i * 74.0f,0,0 };
		transform->rotation.y = XM_PI * 0.5f;
		MapToolSaveType saveType;
		saveType.name = model->GetName();
		saveType.transform = transform;
		model->GetLastCollider()->scale = colliderScales[4];
		//saveTypes.push_back(saveType);
	}
	for (int i = 0; i < 7; i++)
	{
		Transform* transform = model->AddTransform();
		transform->position = { 36.0f + i * 74.0f,0,511 };
		transform->rotation.y = XM_PI * 0.5f;
		MapToolSaveType saveType;
		saveType.name = model->GetName();
		saveType.transform = transform;
		model->GetLastCollider()->scale = colliderScales[4];
		//saveTypes.push_back(saveType);
	}

}

void MapToolScene::LoadSpawnPoints()
{
	const string file = "SaveData/SpawnPoints/spawnPointData.xml";

	XmlDocument* document = new XmlDocument();
	document->LoadFile(file.c_str());

	XmlElement* root = document->FirstChildElement();
	XmlElement* spawnNode = root->FirstChildElement();
	do
	{
		Vector3 position;
		XmlElement* node = spawnNode->FirstChildElement();
		position.x = node->FloatAttribute("X");
		position.y = node->FloatAttribute("Y");
		position.z = node->FloatAttribute("Z");

		EnemySpawnPoint* spawnPoint = NEW EnemySpawnPoint(position);
		spawnPoints.push_back(spawnPoint);

		spawnNode = spawnNode->NextSiblingElement();

	} while (spawnNode != nullptr);

	delete document;
}

void MapToolScene::SaveSpawnPoints()
{
	const string savePath = "SaveData/SpawnPoints/spawnPointData.xml";
	Path::CreateFolders(savePath);
	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLElement* root = document->NewElement("SpawnPoints");
	document->LinkEndChild(root);

	for (EnemySpawnPoint* spawnPoint : spawnPoints)
	{
		Xml::XMLElement* node = document->NewElement("SpawnPoint");
		root->LinkEndChild(node);

		Xml::XMLElement* element = document->NewElement("position");
		element->SetAttribute("X", spawnPoint->position.x);
		element->SetAttribute("Y", spawnPoint->position.y);
		element->SetAttribute("Z", spawnPoint->position.z);
		node->LinkEndChild(element);
	}

	document->SaveFile(savePath.c_str());

	delete document;
}

void MapToolScene::SaveMap()
{
	SaveSpawnPoints();
	const string savePath = "SaveData/MapTool/mapToolData.xml";
	Path::CreateFolders(savePath);
	Xml::XMLDocument* document = new Xml::XMLDocument();

	Xml::XMLElement* root = document->NewElement("Map");
	document->LinkEndChild(root);

	Xml::XMLElement* name = document->NewElement("Objects");
	root->LinkEndChild(name);
	for (MapToolSaveType saveType : saveTypes)
	{
		Xml::XMLElement* node = document->NewElement(saveType.name.c_str());
		//node->SetAttribute("name", saveType.name.c_str());
		name->LinkEndChild(node);

		Xml::XMLElement* element = document->NewElement("position");
		element->SetAttribute("X", saveType.transform->position.x);
		element->SetAttribute("Y", saveType.transform->position.y);
		element->SetAttribute("Z", saveType.transform->position.z);
		node->LinkEndChild(element);
		element = document->NewElement("scale");
		element->SetAttribute("X", saveType.transform->scale.x);
		element->SetAttribute("Y", saveType.transform->scale.y);
		element->SetAttribute("Z", saveType.transform->scale.z);
		node->LinkEndChild(element);
		element = document->NewElement("rotation");
		element->SetAttribute("X", saveType.transform->rotation.x);
		element->SetAttribute("Y", saveType.transform->rotation.y);
		element->SetAttribute("Z", saveType.transform->rotation.z);
		node->LinkEndChild(element);
	}

	name = document->NewElement("Colliders");
	root->LinkEndChild(name);
	for (UINT i = 0; i < colliderScales.size(); i++)
	{
		Xml::XMLElement* node = document->NewElement("Collider");
		name->LinkEndChild(node);

		Xml::XMLElement* element = document->NewElement("scale");
		element->SetAttribute("X", colliderScales[i].x);
		element->SetAttribute("Y", colliderScales[i].y);
		element->SetAttribute("Z", colliderScales[i].z);
		node->LinkEndChild(element);

		element = document->NewElement("offset");
		element->SetAttribute("X", colliderOffsets[i].x);
		element->SetAttribute("Y", colliderOffsets[i].y);
		element->SetAttribute("Z", colliderOffsets[i].z);
		node->LinkEndChild(element);
	}

	document->SaveFile(savePath.c_str());

	delete document;
}

void MapToolScene::LoadMap()
{
	LoadSpawnPoints();

	const string file = "SaveData/MapTool/mapToolData.xml";

	XmlDocument* document = new XmlDocument();
	document->LoadFile(file.c_str());

	XmlElement* root = document->FirstChildElement();

	XmlElement* objectsNode = root->FirstChildElement();
	XmlElement* objectNode = objectsNode->FirstChildElement();

	do
	{
		string name = objectNode->Name();
		Transform* transform = nullptr;
		BoxCollider* collider = nullptr;
		for (ModelInstance* model : models)
		{
			if (model->GetName() == name)
			{
				transform = model->AddTransform();
				collider = model->GetLastCollider();
			}
		}
		colliders.push_back(collider);

		XmlElement* node = objectNode->FirstChildElement();
		transform->position.x = node->FloatAttribute("X");
		transform->position.y = node->FloatAttribute("Y");
		transform->position.z = node->FloatAttribute("Z");
		node = node->NextSiblingElement();

		transform->scale.x = node->FloatAttribute("X");
		transform->scale.y = node->FloatAttribute("Y");
		transform->scale.z = node->FloatAttribute("Z");
		node = node->NextSiblingElement();

		transform->rotation.x = node->FloatAttribute("X");
		transform->rotation.y = node->FloatAttribute("Y");
		transform->rotation.z = node->FloatAttribute("Z");

		objectNode = objectNode->NextSiblingElement();

	} while (objectNode != nullptr);

	XmlElement* collidersNode = objectsNode->NextSiblingElement();
	XmlElement* colliderNode = collidersNode->FirstChildElement();

	UINT index = 0;
	do
	{
		Transform* transform;
		Vector3 scale;
		XmlElement* scaleNode = colliderNode->FirstChildElement();
		scale.x = scaleNode->FloatAttribute("X");
		scale.y = scaleNode->FloatAttribute("Y");
		scale.z = scaleNode->FloatAttribute("Z");
		models[index]->SetCollidersScale(scale);

		Vector3 offset;
		XmlElement* offsetNode = scaleNode->NextSiblingElement();
		offset.x = offsetNode->FloatAttribute("X");
		offset.y = offsetNode->FloatAttribute("Y");
		offset.z = offsetNode->FloatAttribute("Z");
		models[index]->SetCollidersOffset(offset);

		colliderNode = colliderNode->NextSiblingElement();
		index++;
	} while (colliderNode != nullptr);


	delete document;
}







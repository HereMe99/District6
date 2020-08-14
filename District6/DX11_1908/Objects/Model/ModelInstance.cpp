#include "Framework.h"
#include "ModelInstance.h"

ModelInstance::ModelInstance(string name)
	: name(name)
{
	//추출한번했으면 주석처리할것
	//Export(name);
	intBuffer = NEW IntBuffer();
	string path = name + "/" + name;
	model = NEW ModelRender(L"ModelInstancing");
	model->ReadMaterial(path);

	model->ReadMesh(path);
}

ModelInstance::~ModelInstance()
{
	delete model;
	delete intBuffer;
	for(BoxCollider* collider :colliders)
	{
		delete collider;
	}
}

void ModelInstance::Update()
{
	model->Update();
	for (BoxCollider* collider : colliders)
	{
		collider->Update();
	}
}

void ModelInstance::Render()
{
	intBuffer->SetPSBuffer(10);
	model->Render();
	for (BoxCollider* collider : colliders)
	{
		collider->Render();
	}
}

void ModelInstance::Export(const string name)
{

	
	ModelReader* reader = NEW ModelReader();
	reader->ReadFile("ModelData/Models/" + name + ".fbx");
	reader->ExportMaterial(name + "/" + name,false);
	reader->ExportMesh(name + "/" + name,false);
	delete reader;

}

void ModelInstance::SetColliderBox(int boxIndex, Float3 min, Float3 max)
{
	colliders[boxIndex]->SetBox(min, max);
}


void ModelInstance::SetCollidersScale(Vector3 scale)
{
	for(BoxCollider* collider : colliders)
	{
		collider->scale = scale;
	}
}

void ModelInstance::SetCollidersOffset(Vector3 offset)
{
	for (BoxCollider* collider : colliders)
	{
		collider->offset = offset;
	}
}

void ModelInstance::DeleteTransform(vector<BoxCollider*>& worldColliders, vector<Transform*>& worldTransforms)
{
	if (model->GetTransforms()->size() < 1)
	{
		return;
	}
	SAFE_DELETE(model->GetTransforms()->back());
	model->GetTransforms()->pop_back();

	for(int i = 0; i < worldColliders.size(); i++)
	{
		if(worldColliders[i] == colliders.back())
		{
			worldTransforms.erase(worldTransforms.begin() + i);
			worldColliders.erase(worldColliders.begin() + i);
		}
	}
	
	SAFE_DELETE(colliders.back());
	colliders.back() = nullptr;
	colliders.pop_back();
}

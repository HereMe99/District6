#include "Framework.h"
#include "AniInstance.h"

AniInstance::AniInstance(string name)
	:name(Path::GetFileName(name))
{
	Export(name + ".fbx");
	Path::Replace(&name, "/", "_");
	this->name = Path::GetFileName(name);
	model = NEW AnimationModel("Assets/Models/" + this->name + "/" + this->name + ".mesh");

}

AniInstance::~AniInstance()
{
	delete model;

	/*for(Transform* transform : transforms)
	{
		SAFE_DELETE(transform);
	}*/
	for (BoxCollider* collider : colliders)
	{
		delete collider;
	}
}

void AniInstance::Update()
{
	model->Update();
	for (BoxCollider* collider : colliders)
	{
		collider->Update();
	}
}

void AniInstance::Render()
{

	model->Render();
	for (BoxCollider* collider : colliders)
	{
		collider->Render();
	}
}

void AniInstance::Export(string name)
{
	string path = "ModelData/Models/" + name;
	Path::Replace(&name, "/", "_");
	name = Path::GetFileNameWithoutExtension(name);
	string outpath = "Assets/Models/" + name + "/" + name + ".mesh";
	if (!Path::ExistFile(outpath))
	{
		AssimpConverter* converter = NEW AssimpConverter();
		converter->ConvertMesh(path, outpath);
		SAFE_DELETE(converter);
	}
}

void AniInstance::AddAnimation(string aniName, bool isLoop)
{
	string path = "ModelData/Models/" + aniName + ".fbx";
	string name = aniName;
	Path::Replace(&aniName, "/", "_");
	
	string outpath = "Assets/Models/" + aniName + "/anims/" + aniName + ".anim";
	if (!Path::ExistFile(outpath))
	{
		AssimpConverter* converter = NEW AssimpConverter();
		converter->ConvertMesh(path);
		converter->SaveAnimation("Assets/Models/" + aniName + "/anims/" + aniName + ".anim");
		SAFE_DELETE(converter);
	}

	model->LoadAnimation("Assets/Models/" + aniName + "/anims/" + aniName + ".anim", 1, isLoop);
}



#include "Framework.h"

Model::Model()
{
}

Model::~Model()
{
	for (auto material : materials)
		delete material.second;

	for (auto bone : bones)
		delete bone;

	for (auto mesh : meshes)
		delete mesh;

	for (auto clip : clips)
		delete clip;
}

void Model::ReadMaterial(string file)
{
	file = "ModelData/Materials/" + file + ".mat";

	wstring folder = Path::ToWString(Path::GetDirectoryName(file));

	XmlDocument* document = NEW XmlDocument();
	document->LoadFile(file.c_str());

	XmlElement* root = document->FirstChildElement();
	XmlElement* matNode = root->FirstChildElement();

	do
	{
		XmlElement* node = matNode->FirstChildElement();

		Material* material = NEW Material();
		material->name = node->GetText();

		node = node->NextSiblingElement();

		if (node->GetText())
		{
			wstring file = Path::ToWString(node->GetText());
			material->SetDiffuseMap(folder + file);
		}

		node = node->NextSiblingElement();

		if (node->GetText())
		{
			wstring file = Path::ToWString(node->GetText());
			material->SetSpecularMap(folder + file);
		}

		node = node->NextSiblingElement();

		if (node->GetText())
		{
			wstring file = Path::ToWString(node->GetText());
			material->SetNormalMap(folder + file);
		}

		Float4 color;

		node = node->NextSiblingElement();		
		color.x = node->FloatAttribute("R");
		color.y = node->FloatAttribute("G");
		color.z = node->FloatAttribute("B");
		color.w = node->FloatAttribute("A");
		material->GetBuffer()->data.ambient = color;

		node = node->NextSiblingElement();
		color.x = node->FloatAttribute("R");
		color.y = node->FloatAttribute("G");
		color.z = node->FloatAttribute("B");
		color.w = node->FloatAttribute("A");
		material->GetBuffer()->data.diffuse = color;

		node = node->NextSiblingElement();
		color.x = node->FloatAttribute("R");
		color.y = node->FloatAttribute("G");
		color.z = node->FloatAttribute("B");
		color.w = node->FloatAttribute("A");
		material->GetBuffer()->data.specular = color;

		materials.insert({ material->name, material });

		matNode = matNode->NextSiblingElement();
	} while (matNode != nullptr);

	delete document;
}

void Model::ReadMesh(string file)
{
	file = "ModelData/Meshes/" + file + ".mesh";
	BinaryReader* r = NEW BinaryReader(file);

	UINT count = r->UInt();

	for (UINT i = 0; i < count; i++)
	{
		ModelBone* bone = NEW ModelBone();

		bone->index = r->Int();
		bone->name = r->String();
		
		bone->parentIndex = r->Int();
		bone->transform = XMLoadFloat4x4(&r->Matrix());

		bones.push_back(bone);
	}

	count = r->UInt();

	for (UINT i = 0; i < count; i++)
	{
		ModelMesh* mesh = NEW ModelMesh();
		mesh->name = r->String();
		mesh->boneIndex = r->Int();

		mesh->materialName = r->String();

		{//VertexData
			UINT count = r->UInt();

			mesh->vertexCount = count;
			mesh->vertices = NEW ModelVertex[count];

			void* ptr = static_cast<void*>(mesh->vertices);
			r->Byte(&ptr, sizeof(ModelVertex) * count);
		}

		{//IndexData
			UINT count = r->UInt();

			mesh->indexCount = count;
			mesh->indices = NEW UINT[count];

			void* ptr = static_cast<void*>(mesh->indices);
			r->Byte(&ptr, sizeof(UINT) * count);
		}		

		meshes.push_back(mesh);
	}

	delete r;

	BindBone();
	BindMesh();
}

void Model::ReadClip(string file)
{
	file = "ModelData/Clips/" + file + ".clip";

	BinaryReader* r = NEW BinaryReader(file);

	ModelClip* clip = NEW ModelClip();

	clip->name = r->String();
	clip->duration = r->Float();
	clip->frameRate = r->Float();
	clip->frameCount = r->UInt();

	UINT keyFramesCount = r->UInt();
	for (UINT i = 0; i < keyFramesCount; i++)
	{
		KeyFrame* keyFrame = NEW KeyFrame();
		keyFrame->boneName = r->String();

		UINT size = r->UInt();
		if (size > 0)
		{
			keyFrame->transforms.resize(size);

			void* ptr = (void*)keyFrame->transforms.data();
			r->Byte(&ptr, sizeof(KeyTransform) * size);
		}

		clip->keyFrameMap[keyFrame->boneName] = keyFrame;
	}
	clips.push_back(clip);

	delete r;
}

void Model::BindBone()
{
	root = bones[0];

	for (ModelBone* bone : bones)
	{
		if (bone->parentIndex > -1)
		{
			bone->parent = bones[bone->parentIndex];
			bone->parent->children.push_back(bone);
		}
		else
			bone->parent = nullptr;
	}
}

void Model::BindMesh()
{
	for (auto mesh : meshes)
	{
		for (auto bone : bones)
		{
			if (mesh->boneIndex == bone->index)
			{
				mesh->bone = bone;
				break;
			}
		}

		mesh->Binding(this);
	}
}

void Model::SetShader(wstring file)
{
	for (ModelMesh* mesh : meshes)
		mesh->SetShader(file);
}

void Model::SetShader(Shader* shader)
{
	for (ModelMesh* mesh : meshes)
		mesh->SetShader(shader);
}

void Model::SetDiffuseMap(wstring file)
{
	for (ModelMesh* mesh : meshes)
		mesh->GetMaterial()->SetDiffuseMap(file);
}

void Model::SetSpecularMap(wstring file)
{
	for (ModelMesh* mesh : meshes)
		mesh->GetMaterial()->SetSpecularMap(file);
}

void Model::SetNormalMap(wstring file)
{
	for (ModelMesh* mesh : meshes)
		mesh->GetMaterial()->SetNormalMap(file);
}

void Model::Attach(wstring shader, Model* model, int parentBoneIndex, Transform* transform)
{
	//Copy Material
	for (auto material : model->materials)
	{
		Material* NEWMaterial = NEW Material(shader);

		NEWMaterial->name = material.second->name;
		NEWMaterial->GetBuffer()->data = material.second->GetBuffer()->data;

		if (material.second->GetDiffuseMap() != nullptr)
			NEWMaterial->SetDiffuseMap(material.second->GetDiffuseMap()->GetFile());

		if (material.second->GetSpecularMap() != nullptr)
			NEWMaterial->SetSpecularMap(material.second->GetSpecularMap()->GetFile());

		if (material.second->GetNormalMap() != nullptr)
			NEWMaterial->SetNormalMap(material.second->GetNormalMap()->GetFile());

		materials.insert({ NEWMaterial->name, NEWMaterial });
	}

	vector<pair<int, int>> changes;

	{//Copy Bone
		ModelBone* parentBone = bones[parentBoneIndex];

		for (ModelBone* bone : model->bones)
		{
			ModelBone* NEWBone = NEW ModelBone();
			NEWBone->name = bone->name;
			NEWBone->transform = bone->transform;

			if (transform != nullptr)
				NEWBone->transform = NEWBone->transform * transform->GetWorld();

			if (bone->parent != nullptr)
			{
				int parentIndex = bone->parentIndex;

				for (pair<int, int>& temp : changes)
				{
					if(temp.first == parentIndex)
					{
						NEWBone->parentIndex = temp.second;
						NEWBone->parent = bones[NEWBone->parentIndex];
						NEWBone->parent->children.push_back(NEWBone);

						break;
					}
				}
			}
			else
			{
				NEWBone->parentIndex = parentBoneIndex;
				NEWBone->parent = parentBone;
				NEWBone->parent->children.push_back(NEWBone);
			}

			NEWBone->index = bones.size();
			changes.push_back(pair<int, int>(bone->index, NEWBone->index));

			bones.push_back(NEWBone);
		}
	}

	{//Copy Mesh
		for (ModelMesh* mesh : model->meshes)
		{
			ModelMesh* NEWMesh = NEW ModelMesh();
			for (pair<int, int>& temp : changes)
			{
				if (temp.first == mesh->boneIndex)
				{
					NEWMesh->boneIndex = temp.second;
					break;
				}
			}

			NEWMesh->bone = bones[NEWMesh->boneIndex];
			NEWMesh->name = mesh->name;
			NEWMesh->materialName = mesh->materialName;

			NEWMesh->vertexCount = mesh->vertexCount;
			NEWMesh->indexCount = mesh->indexCount;

			UINT verticesSize = NEWMesh->vertexCount * sizeof(ModelVertex);
			NEWMesh->vertices = NEW ModelVertex[NEWMesh->vertexCount];
			memcpy_s(NEWMesh->vertices, verticesSize, mesh->vertices, verticesSize);

			UINT indicesSize = NEWMesh->indexCount * sizeof(UINT);
			NEWMesh->indices = NEW UINT[NEWMesh->indexCount];
			memcpy_s(NEWMesh->indices, indicesSize, mesh->indices, indicesSize);

			NEWMesh->Binding(this);
			NEWMesh->SetShader(shader);

			meshes.push_back(NEWMesh);
		}
	}
}

#include "Framework.h"

AssimpConverter::AssimpConverter()
{
}

AssimpConverter::~AssimpConverter()
{
	ResetMeshs();
	ResetAnimations();
}

void AssimpConverter::ConvertMesh(const string path, const string outPath)
{
	assert(Path::ExistFile(path));
	meshPath = path;
	name = Path::GetFileNameWithoutExtension(Path::GetFileName(path));
	string outName= Path::GetFileNameWithoutExtension(Path::GetFileName(outPath));
	if (outPath.empty())
	{
		this->outPath = "Assets/Models/" + name + "/" + name + ".mesh";
	}
	else
	{
		if(outName.empty())
			this->outPath = outPath + name + ".mesh";
		else if(outName == outPath)
			this->outPath = "Assets/Models/" + outName + "/" + outName + ".mesh";
		else
			this->outPath = outPath;
	}

	LoadMesh();

	if (!meshs.size())
		return;

	SaveMesh();
}


void AssimpConverter::LoadAnimation(const string path)
{
	if (!hierarchyNodes.size())
		return;

	assert(Path::ExistFile(path));

	ResetAnimations();

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_MakeLeftHanded);

	if (scene->HasAnimations())
	{
		animName = Path::GetFileNameWithoutExtension(Path::GetFileName(path));
		AddAnimation(scene);
	}
}

void AssimpConverter::SaveAnimation(string path, UINT takeNum)
{
	if (takeNum >= animations.size())
		return;

	if (path == "")
	{
		if(animations.size() == 1)
			path = "Assets/Models/" + name + "/anims/" + animName + ".anim";
		else
			path = "Assets/Models/" + name + "/anims/" + animName + animations[takeNum].name + ".anim";
	}

	Path::CreateFolders(Path::GetDirectoryName(path));

	BinaryWriter* w = NEW BinaryWriter(path);

	w->Float(animations[takeNum].tickPerSec);
	w->Float(animations[takeNum].duration);

	w->UInt((UINT)animations[takeNum].keyframes.size());
	for (auto keyframe : animations[takeNum].keyframes)
	{
		w->UInt((UINT)keyframe.size());
		for (auto key : keyframe)
		{
			w->Byte(&key.translate, sizeof(XMFLOAT3));
			w->Float(key.keyType);
			w->Byte(&key.quaternion, sizeof(XMFLOAT4));
			w->Byte(&key.scale, sizeof(XMFLOAT3));
		}
	}
	SAFE_DELETE(w);
}


void AssimpConverter::LoadMesh()
{
	ResetMeshs();

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(meshPath, aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality);
	assert(scene != NULL);

	NodeHeirarchy(scene->mRootNode, -1);
	InitBones(scene);
	InitMesh(scene);

	if (scene->HasAnimations())
	{
		animName = name;
		AddAnimation(scene);
	}
}

void AssimpConverter::SaveMesh()
{
	Path::CreateFolders(Path::GetDirectoryName(outPath));

	BinaryWriter* w = NEW BinaryWriter(outPath);

	w->UInt((UINT)hierarchyNodes.size());
	for (auto node : hierarchyNodes)
	{
		w->String(node.name);
		w->Byte(&node.translate, sizeof(XMFLOAT3));
		w->Byte(&node.preQuaternion, sizeof(XMFLOAT4));
		w->Byte(&node.quaternion, sizeof(XMFLOAT4));
		w->Byte(&node.scale, sizeof(XMFLOAT3));
		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, node.local);
		w->Float4x4(matrix);
		XMStoreFloat4x4(&matrix, node.world);
		w->Float4x4(matrix);
		w->Float4x4(node.offset);
		w->Int(node.parentID);
	}

	w->UInt((UINT)meshs.size());
	for (auto mesh : meshs)
	{
		w->UInt(mesh.ID);
		w->UInt(mesh.matrialID);
		w->UInt((UINT)mesh.vertices.size());
		w->Byte(mesh.vertices.data(), sizeof(VertexType) * (UINT)mesh.vertices.size());
		w->UInt((UINT)mesh.indices.size());
		w->Byte(mesh.indices.data(), sizeof(UINT) * (UINT)mesh.indices.size());
	}

	w->UInt((UINT)materials.size());
	for (auto mat : materials)
	{
		w->String(mat.name);

		w->Byte(&mat.diffuse, sizeof(XMFLOAT3));
		w->Byte(&mat.ambient, sizeof(XMFLOAT3));
		w->Byte(&mat.specular, sizeof(XMFLOAT3));
		w->Byte(&mat.emissive, sizeof(XMFLOAT3));
		w->Byte(&mat.tranparent, sizeof(XMFLOAT3));
		w->Byte(&mat.reflective, sizeof(XMFLOAT3));

		w->Float(mat.opacity);
		w->Float(mat.transparentfactor);
		w->Float(mat.bumpscaling);
		w->Float(mat.shininess);
		w->Float(mat.reflectivity);
		w->Float(mat.shininessstrength);
		w->Float(mat.refracti);

		w->String(mat.diffusefile);
		w->String(mat.specularfile);
		w->String(mat.ambientfile);
		//w->String(mat.emissivefile);
		//w->String(mat.heightfile);
		w->String(mat.normalfile);
		w->String(mat.shininessfile);
		/*
		w->String(mat.opacityfile);
		w->String(mat.displacementfile);
		w->String(mat.lightMapfile);
		w->String(mat.reflectionfile);
		//pbr
		w->String(mat.basecolorfile);
		w->String(mat.normalcamerafile);
		w->String(mat.emissioncolorfile);
		w->String(mat.metalnessfile);
		w->String(mat.diffuseroughnessfile);
		w->String(mat.ambientocculsionfile);*/
	}
	SAFE_DELETE(w);
}

void AssimpConverter::ResetMeshs()
{
	vector<HierarchyNode>().swap(hierarchyNodes);
	//map<string,UINT>().swap(hierarchyMap);
	hierarchyMap.clear();
	//bones.clear();
	//boneMap.clear();
	for (Mesh& i : meshs)
	{
		vector<VertexType>().swap(i.vertices);
		vector<UINT>().swap(i.indices);
	}
	vector<Mesh>().swap(meshs);
	vector<Material>().swap(materials);
}

void AssimpConverter::NodeHeirarchy(const aiNode* node, int value)
{
	string nodeName(node->mName.data);
	int parentID = value;
	size_t idx = nodeName.find("_$AssimpFbx$_");
	string subName;
	if (string::npos != idx)
	{
		subName = nodeName.substr(0, idx);
		if (hierarchyMap.count(subName) == 0)
		{
			HierarchyNode NEWNode;
			NEWNode.name = subName;
			NEWNode.parentID = value;
			hierarchyNodes.emplace_back(NEWNode);
			parentID = (int)hierarchyNodes.size() - 1;
			for (UINT i = 0; i < node->mNumMeshes; i++)
			{
				meshMap[node->mMeshes[i]] = (UINT)parentID;
			}
			hierarchyMap.emplace(subName, parentID);
		}
		XMVECTOR trans, quat, scale;
		XMMatrixDecompose(&scale, &quat, &trans, XMMatrixTranspose(XMMATRIX(node->mTransformation[0])));
		if (nodeName.find("$_Translation") > idx && string::npos != nodeName.find("$_Translation"))
			XMStoreFloat3(&hierarchyNodes[hierarchyMap[subName]].translate, trans);
		if (nodeName.find("$_PreRotation") > idx && string::npos != nodeName.find("$_PreRotation"))
			XMStoreFloat4(&hierarchyNodes[hierarchyMap[subName]].preQuaternion, quat);
		if (nodeName.find("$_Rotation") > idx && string::npos != nodeName.find("$_Rotation"))
			XMStoreFloat4(&hierarchyNodes[hierarchyMap[subName]].quaternion, quat);
		if (nodeName.find("$_Scaling") > idx && string::npos != nodeName.find("$_Scaling"))
			XMStoreFloat3(&hierarchyNodes[hierarchyMap[subName]].scale, scale);
	}
	else
	{
		if(hierarchyMap.count(nodeName) == 0)
		{
			HierarchyNode NEWNode;
			NEWNode.name = nodeName;
			NEWNode.parentID = value;
			XMVECTOR trans, quat, scale;
			XMMatrixDecompose(&scale, &quat, &trans, XMMatrixTranspose(XMMATRIX(node->mTransformation[0])));
			XMStoreFloat3(&NEWNode.translate, trans);
			XMStoreFloat4(&NEWNode.quaternion, quat);
			XMStoreFloat3(&NEWNode.scale, scale);
			hierarchyNodes.emplace_back(NEWNode);
			parentID = (int)hierarchyNodes.size() - 1;
			for (UINT i = 0; i < node->mNumMeshes; i++)
			{
				meshMap[node->mMeshes[i]] = (UINT)parentID;
			}
			hierarchyMap.emplace(nodeName, parentID);
		}
		else
		{
			XMVECTOR trans, quat, scale, pretrans, prequat, prescale;
			XMMatrixDecompose(&scale, &quat, &trans, XMMatrixTranspose(XMMATRIX(node->mTransformation[0])));
			pretrans = XMLoadFloat3(&hierarchyNodes[hierarchyMap[nodeName]].translate);
			prequat = XMLoadFloat4(&hierarchyNodes[hierarchyMap[nodeName]].quaternion);
			prescale = XMLoadFloat3(&hierarchyNodes[hierarchyMap[nodeName]].scale);
			XMStoreFloat3(&hierarchyNodes[hierarchyMap[nodeName]].translate, trans + pretrans);
			XMStoreFloat4(&hierarchyNodes[hierarchyMap[nodeName]].quaternion, XMQuaternionMultiply(quat, prequat));
			XMStoreFloat3(&hierarchyNodes[hierarchyMap[nodeName]].scale, scale * prescale);
			parentID = (int)hierarchyNodes.size() - 1;
		}
		hierarchyNodes[hierarchyMap[nodeName]].local = XMMatrixTransformation(XMVectorZero(), XMQuaternionIdentity(), 
			XMLoadFloat3(&hierarchyNodes[hierarchyMap[nodeName]].scale), XMVectorZero(), XMQuaternionMultiply(XMLoadFloat4(&hierarchyNodes[hierarchyMap[nodeName]].quaternion),
			XMLoadFloat4(&hierarchyNodes[hierarchyMap[nodeName]].preQuaternion)), XMLoadFloat3(&hierarchyNodes[hierarchyMap[nodeName]].translate));
		hierarchyNodes[hierarchyMap[nodeName]].world = hierarchyNodes[hierarchyMap[nodeName]].local;
		if (hierarchyNodes[hierarchyMap[nodeName]].parentID > -1)
		{
			hierarchyNodes[hierarchyMap[nodeName]].world *= hierarchyNodes[hierarchyNodes[hierarchyMap[nodeName]].parentID].world;
		}
	}

	for (UINT i = 0; i < node->mNumChildren; i++) {
		NodeHeirarchy(node->mChildren[i], parentID);
	}
}

//하이어라키상 메쉬에 적용되는 본이 있는 경우 메쉬마다 다른 오프셋메트릭스를 메쉬의 월드 인버스 매트릭스로 상쇄해서 1개의 오프셋으로 만들기
void AssimpConverter::InitBones(const aiScene* scene)
{
	for (UINT m = 0; m < scene->mNumMeshes; m++)
	{
		const aiMesh* mesh = scene->mMeshes[m];

		if (mesh->mNumBones == 0 || mesh->mNumVertices == 0)
			continue;

		XMMATRIX invWorld = XMMatrixIdentity();
		if (meshMap.count(m) > 0)
			invWorld = XMMatrixInverse(NULL, hierarchyNodes[meshMap[m]].world);

		for (UINT i = 0; i < mesh->mNumBones; i++) {
			string name(mesh->mBones[i]->mName.data);

			if (hierarchyMap.count(name) > 0)
			{
				//hierarchyNodes[hierarchyMap[name]].isBone = 1;
				XMStoreFloat4x4(&hierarchyNodes[hierarchyMap[name]].offset, XMMatrixMultiply(invWorld, XMMatrixTranspose(XMMATRIX(mesh->mBones[i]->mOffsetMatrix[0]))));
			}
		}
	}
}

void AssimpConverter::InitMesh(const aiScene* scene)
{
	for (UINT m = 0; m < scene->mNumMeshes; m++) {
		const aiMesh* mesh = scene->mMeshes[m];
		
		if (mesh->mNumVertices == 0 && meshMap.count(m) == 0)//메쉬이름대신 하이어라키상 메쉬 아이디와 일치하는것이 있는지 분류
			continue;

		Mesh meshData;

		meshData.ID = meshMap[m];
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			VertexType vertex;

			if (!mesh->mVertices)	break;
			memcpy(&vertex.position, &mesh->mVertices[i], sizeof(XMFLOAT3));

			if (mesh->mNormals)
				memcpy(&vertex.normal, &mesh->mNormals[i], sizeof(XMFLOAT3));
			

			if (mesh->mTangents)
				memcpy(&vertex.tangent, &mesh->mTangents[i], sizeof(XMFLOAT3));
			

			if (mesh->mTextureCoords[0])
				memcpy(&vertex.uv, &mesh->mTextureCoords[0][i], sizeof(XMFLOAT2));
			
			vertex.weight = { 0.f, 0.f, 0.f, 0.f };
			vertex.boneid.x = 0;
			vertex.boneid.y = 0;
			vertex.boneid.z = 0;
			vertex.boneid.w = 0;

			meshData.vertices.emplace_back(vertex);
		}
		if (meshData.vertices.empty())
			continue;

		BindSkin(mesh, meshData.vertices);

		for (UINT i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (UINT j = 0; j < face.mNumIndices; j++) {
				meshData.indices.emplace_back((UINT)face.mIndices[j]);
			}
		}
		meshData.matrialID = InitMaterials(scene, mesh->mMaterialIndex);

		meshs.emplace_back(meshData);
	}
}

//하이어라키상의 인덱스로 본아이디 부여
void AssimpConverter::BindSkin(const aiMesh* mesh, vector<VertexType>& vertices)
{
	bool noWeight=true;

	for (UINT i = 0; i < mesh->mNumBones; i++) {
		string boneName(mesh->mBones[i]->mName.data);

		if (hierarchyMap.count(boneName) == 0)
			continue;
		
		for (UINT j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			UINT vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;

			if (vertices[vertexID].weight.x == 0) {
				noWeight = false;
				vertices[vertexID].boneid.x = hierarchyMap[boneName];
				vertices[vertexID].weight.x = weight;
			}
			else if (vertices[vertexID].weight.y == 0) {
				vertices[vertexID].boneid.y = hierarchyMap[boneName];
				vertices[vertexID].weight.y = weight;
			}
			else if (vertices[vertexID].weight.z == 0) {
				vertices[vertexID].boneid.z = hierarchyMap[boneName];
				vertices[vertexID].weight.z = weight;
			}
			else if (vertices[vertexID].weight.w == 0) {
				vertices[vertexID].boneid.w = hierarchyMap[boneName];
				vertices[vertexID].weight.w = weight;
			}
		}
	}
	//노멀라이즈 및 토탈웨이트 1로 맞추기
	if (noWeight)
		return;
	for (VertexType v : vertices)
	{
		int wSize = 0;
		float wList[4] = { v.weight.x,v.weight.y,v.weight.z,v.weight.w };
		float sumWeight = 0.f;
		for (UINT i = 0; i < 4; i++)
		{
			if (wList[i] == 0) break;
			sumWeight += wList[i];
			wSize++;
		}
		float lastWeight = 0.f;
		for (int i = 0; i < wSize - 1; i++)
		{
			wList[i] = wList[i] / sumWeight;
			lastWeight += wList[i];
		}
		if (wSize > 0)
			wList[wSize - 1] = 1.f - lastWeight;
		v.weight.x = wList[0];
		v.weight.y = wList[1];
		v.weight.z = wList[2];
		v.weight.w = wList[3];
	}
}

//initmesh에서 메쉬가 생성될때 해당 메쉬가 메터리얼을 갖는 경우 중복되지 않게 생성 메쉬가 메터리얼을 가질 경우만 생성해서 씬의 불필요한 메터리얼은 제거
UINT AssimpConverter::InitMaterials(const aiScene* scene, UINT index)
{
	if (scene->mNumMaterials == 0)
		return 0;

	//메터리얼의 만들어지는 순서가 메쉬생성 기준이기때문에 씬에서의 메터리얼 아이디 기준으로 비교한다
	auto nodeit = find_if(materials.begin(), materials.end(), [index](const Material mat)->bool { return mat.ID == index; });
	if (nodeit != materials.end())
		return (UINT)distance(materials.begin(), nodeit);

	Material NEWMat;
	aiString  texture_path;
	aiColor3D color;
	float value;

	string name(scene->mMaterials[index]->GetName().data);
	NEWMat.name = name;
	NEWMat.ID = index;

	scene->mMaterials[index]->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	string texPath(texture_path.C_Str());
	if (texPath.size() > 0)
	{
		NEWMat.diffusefile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
	NEWMat.diffuse = { color[0], color[1], color[2] };

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_SPECULAR, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.specularfile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_COLOR_SPECULAR, color);
	NEWMat.specular = { color[0], color[1], color[2] };

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_AMBIENT, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.ambientfile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_COLOR_AMBIENT, color);
	NEWMat.ambient = { color[0], color[1], color[2] };

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_EMISSIVE, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.emissivefile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_COLOR_EMISSIVE, color);
	NEWMat.emissive = { color[0], color[1], color[2] };

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_HEIGHT, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.heightfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_NORMALS, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.normalfile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_BUMPSCALING, value);
	NEWMat.bumpscaling = value;

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_SHININESS, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.shininessfile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_SHININESS, value);
	NEWMat.shininess = value;
	scene->mMaterials[index]->Get(AI_MATKEY_SHININESS_STRENGTH, value);
	NEWMat.shininessstrength = value;

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_OPACITY, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.opacityfile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
	NEWMat.tranparent= { color[0], color[1], color[2] };

	scene->mMaterials[index]->Get(AI_MATKEY_OPACITY, value);
	NEWMat.opacity = value;
	scene->mMaterials[index]->Get(AI_MATKEY_TRANSPARENCYFACTOR, value);
	NEWMat.transparentfactor = value;

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_DISPLACEMENT, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.displacementfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_LIGHTMAP, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.lightMapfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_REFLECTION, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.reflectionfile = SaveTexture(scene, texPath);
	}
	scene->mMaterials[index]->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
	NEWMat.reflective = { color[0], color[1], color[2] };

	scene->mMaterials[index]->Get(AI_MATKEY_REFLECTIVITY, value);//반사율
	NEWMat.reflectivity = value;
	scene->mMaterials[index]->Get(AI_MATKEY_REFRACTI, value);//굴절률
	NEWMat.refracti = value;

	//maya PBR start - FXShader : Stingray PBS
	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.basecolorfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.normalcamerafile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_EMISSION_COLOR, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.emissioncolorfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_METALNESS, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.metalnessfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.diffuseroughnessfile = SaveTexture(scene, texPath);
	}

	texture_path.Clear();
	scene->mMaterials[index]->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texture_path, NULL, NULL, NULL, NULL, NULL);
	texPath = texture_path.C_Str();
	if (texPath.size() > 0)
	{
		NEWMat.ambientocculsionfile = SaveTexture(scene, texPath);
	}
	//maya PBR end

	materials.emplace_back(NEWMat);

	return (UINT)(materials.size() - 1);
}

string AssimpConverter::SaveTexture(const aiScene* scene, string file)
{
	if (file.length() == 0)
		return "";

	const aiTexture* texture = scene->GetEmbeddedTexture(file.c_str());

	string path = "";
	if (texture)
	{
		path = Path::GetDirectoryName(outPath) + "/Textures/" + Path::GetFileNameWithoutExtension(file) + ".png";

		if (texture->mHeight < 1)
		{
			Path::CreateFolders(Path::GetDirectoryName(path));

			BinaryWriter w(path);
			w.Byte(texture->pcData, texture->mWidth);
		}
		else
		{
			Image image;

			image.width = texture->mWidth;
			image.height = texture->mHeight;
			image.pixels = reinterpret_cast<uint8_t*>(texture->pcData);
			image.format = DXGI_FORMAT_R8G8B8A8_UNORM;
			image.rowPitch = image.width * 4;

			image.slicePitch = image.width * image.height * 4;

			Path::CreateFolders(Path::GetDirectoryName(path));

			SaveToWICFile(image, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_PNG),
				Path::ToWString(path).c_str());
		}
	}
	else
	{
		file = Path::GetDirectoryName(meshPath) +"/"+ file;
		Path::Replace(&file, "\\", "/");
		if(!Path::ExistFile(file))
			return "";

		path = Path::GetDirectoryName(outPath) + "/Textures/" + Path::GetFileName(file);

		Path::CreateFolders(Path::GetDirectoryName(path));

		CopyFileA(file.c_str(), path.c_str(), FALSE);
	}

	return path;
}

void AssimpConverter::ResetAnimations()
{
	for (auto& i : animations)
	{
		for (auto j : i.keyframes)
		{
			vector<Key>().swap(j);
		}
		vector<vector<Key>>().swap(i.keyframes);
	}
	vector<Animation>().swap(animations);
}

void AssimpConverter::AddAnimation(const aiScene* scene)
{
	for (UINT i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = scene->mAnimations[i];

		Animation ani;

		ani.name = anim->mName.data;
		ani.tickPerSec = (float)anim->mTicksPerSecond;
		ani.duration = (float)anim->mDuration;

		UINT frameCount = (UINT)anim->mDuration + 1;//제일 끝프레임이 벗어날경우 없을지도 모르겠다 듀레이션으로 나눈 나머지 값과 floor와 frac으로 블랜드 하기에...확인 필요

		for (UINT t = 0; t < frameCount; t++)
		{
			vector<Key> keyframe;
			for (HierarchyNode i : hierarchyNodes)
			{
				Key key;
				key.translate = i.translate;
				key.quaternion = i.quaternion;
				key.scale = i.scale;
				key.keyType = 0.f;//키프레임맵 생성시 3x4에 대해 트랜슬레이트 3개의 플롯을 쓰고 빈자리에 키타입을 작성 컴퓨트에서 해당 키타입으로 필요없는 계산 예외처리
				keyframe.emplace_back(key);
			}
			for (UINT i = 0; i < anim->mNumChannels; i++)
			{
				const aiNodeAnim* nodeAnim = anim->mChannels[i];
				string nodeName(nodeAnim->mNodeName.data);

				if (hierarchyMap.count(nodeName) > 0)
				{
					UINT index = hierarchyMap[nodeName];
					CalcInterpolatedScaling(nodeAnim, t, keyframe[index].scale);
					CalcInterpolatedQuaternion(nodeAnim, t, keyframe[index].quaternion);
					CalcInterpolatedPosition(nodeAnim, t, keyframe[index].translate);
					keyframe[index].keyType = 2;
				}
				else {
					size_t idx = nodeName.find("_$AssimpFbx$_");
					if (string::npos != idx)
					{
						string subName = nodeName.substr(0, idx);
						//auto nodeit = find_if(hierarchyNodes.begin(), hierarchyNodes.end(), [subName](const HierarchyNode hNode)->bool { return hNode.name == subName; });
						//if (nodeit != hierarchyNodes.end())
						if (hierarchyMap.count(subName) > 0)
						{
							//UINT index = (UINT)distance(hierarchyNodes.begin(), nodeit);
							UINT index = hierarchyMap[subName];
							if (nodeName.find("$_Scaling") > idx && string::npos != nodeName.find("$_Scaling"))
								CalcInterpolatedScaling(nodeAnim, t, keyframe[index].scale);

							if (nodeName.find("$_Rotation") > idx && string::npos != nodeName.find("$_Rotation"))
								CalcInterpolatedQuaternion(nodeAnim, t, keyframe[index].quaternion);

							if (nodeName.find("$_Translation") > idx && string::npos != nodeName.find("$_Translation"))
								CalcInterpolatedPosition(nodeAnim, t, keyframe[index].translate);
							keyframe[index].keyType = 2;
						}
					}
				}
			}
			ani.keyframes.emplace_back(keyframe);
		}
		for (UINT i = 0; i < hierarchyNodes.size(); i++)
		{
			int parentID = hierarchyNodes[i].parentID;

			if(ani.keyframes[0][i].keyType == 0)
				while (parentID > -1)
				{
					if (ani.keyframes[0][parentID].keyType > 1.f)
					{
						ani.keyframes[0][i].keyType = 1;
						break;
					}
					parentID = hierarchyNodes[parentID].parentID;
				}

			for (UINT t = 1; t < frameCount; t++)
			{
				ani.keyframes[t][i].keyType = ani.keyframes[0][i].keyType;
			}
		}
		animations.emplace_back(ani);
	}
}

void AssimpConverter::CalcInterpolatedPosition(const aiNodeAnim* nodeAnim, const UINT frame, XMFLOAT3& trans)
{
	if (nodeAnim->mNumPositionKeys == 0)
		return;
	if (nodeAnim->mNumPositionKeys == 1)
	{
		memcpy_s(&trans, sizeof(trans), &nodeAnim->mPositionKeys[0].mValue, sizeof(nodeAnim->mPositionKeys[0].mValue));
		return;
	}

	UINT index;
	for (index = 0; index < nodeAnim->mNumPositionKeys - 1; )
	{
		if (frame < nodeAnim->mPositionKeys[index + 1].mTime)
			break;
		index++;
	}
	if (index > nodeAnim->mNumPositionKeys - 1)
	{
		memcpy_s(&trans, sizeof(trans), &nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mValue, sizeof(nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mValue));
		return;
	}

	float Factor = ((float)frame - (float)nodeAnim->mPositionKeys[index].mTime) / (float)(nodeAnim->mPositionKeys[index + 1].mTime - nodeAnim->mPositionKeys[index].mTime);
	Factor = min(max(Factor, 0.0f), 1.0f);
	XMVECTOR Start, End;
	memcpy_s(&Start, sizeof(Start), &nodeAnim->mPositionKeys[index].mValue, sizeof(nodeAnim->mPositionKeys[index].mValue));
	memcpy_s(&End, sizeof(End), &nodeAnim->mPositionKeys[index + 1].mValue, sizeof(nodeAnim->mPositionKeys[index + 1].mValue));
	XMStoreFloat3(&trans, XMVectorLerp(Start, End, Factor));
}

void AssimpConverter::CalcInterpolatedQuaternion(const aiNodeAnim* nodeAnim, const UINT frame, XMFLOAT4& quat)
{
	if (nodeAnim->mNumRotationKeys == 0)
		return;
	if (nodeAnim->mNumRotationKeys == 1)
	{
		quat = { nodeAnim->mRotationKeys[0].mValue.x, nodeAnim->mRotationKeys[0].mValue.y, nodeAnim->mRotationKeys[0].mValue.z, nodeAnim->mRotationKeys[0].mValue.w };
		return;
	}

	UINT index;
	for (index = 0; index < nodeAnim->mNumRotationKeys - 1; )
	{
		if (frame < nodeAnim->mRotationKeys[index + 1].mTime)
			break;
		index++;
	}
	if (index >= nodeAnim->mNumRotationKeys - 1)
	{
		quat = { nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue.x,
			nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue.y,
			nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue.z,
			nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue.w };
		return;
	}

	float Factor = ((float)frame - (float)nodeAnim->mRotationKeys[index].mTime) / (float)(nodeAnim->mRotationKeys[index + 1].mTime - nodeAnim->mRotationKeys[index].mTime);
	Factor = min(max(Factor, 0.0f), 1.0f);
	XMVECTOR Start, End;
	Start = { nodeAnim->mRotationKeys[index].mValue.x,
		nodeAnim->mRotationKeys[index].mValue.y,
		nodeAnim->mRotationKeys[index].mValue.z,
		nodeAnim->mRotationKeys[index].mValue.w };
	End = { nodeAnim->mRotationKeys[index + 1].mValue.x,
		nodeAnim->mRotationKeys[index + 1].mValue.y,
		nodeAnim->mRotationKeys[index + 1].mValue.z,
		nodeAnim->mRotationKeys[index + 1].mValue.w };
	XMStoreFloat4(&quat, XMQuaternionSlerp(Start, End, Factor));
}

void AssimpConverter::CalcInterpolatedScaling(const aiNodeAnim* nodeAnim, const UINT frame, XMFLOAT3& scale)
{
	if (nodeAnim->mNumScalingKeys == 0)
		return;
	if (nodeAnim->mNumScalingKeys == 1)
	{
		memcpy_s(&scale, sizeof(scale), &nodeAnim->mScalingKeys[0].mValue, sizeof(nodeAnim->mScalingKeys[0].mValue));
		return;
	}

	UINT index;
	for (index = 0; index < nodeAnim->mNumScalingKeys - 1; )
	{
		if (frame < nodeAnim->mScalingKeys[index + 1].mTime)
			break;
		index++;
	}
	if (index >= nodeAnim->mNumScalingKeys - 1)
	{
		memcpy_s(&scale, sizeof(scale), &nodeAnim->mScalingKeys[nodeAnim->mNumScalingKeys - 1].mValue, sizeof(nodeAnim->mScalingKeys[nodeAnim->mNumScalingKeys - 1].mValue));
		return;
	}

	float Factor = ((float)frame - (float)nodeAnim->mScalingKeys[index].mTime) / (float)(nodeAnim->mScalingKeys[index + 1].mTime - nodeAnim->mScalingKeys[index].mTime);
	Factor = min(max(Factor, 0.0f), 1.0f);
	XMVECTOR Start, End;
	memcpy_s(&Start, sizeof(Start), &nodeAnim->mScalingKeys[index].mValue, sizeof(nodeAnim->mScalingKeys[index].mValue));
	memcpy_s(&End, sizeof(End), &nodeAnim->mScalingKeys[index + 1].mValue, sizeof(nodeAnim->mScalingKeys[index + 1].mValue));
	XMStoreFloat3(&scale, XMVectorLerp(Start, End, Factor));
}

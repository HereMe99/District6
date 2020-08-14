#pragma once

class AnimationModel
{
private:
	struct HierarchyNode
	{
		XMFLOAT3 translate = { 0.f,0.f,0.f };
		XMFLOAT4 preQuaternion = { 0.f,0.f,0.f,1.f };
		XMFLOAT4 quaternion = { 0.f,0.f,0.f,1.f };
		XMFLOAT3 scale = { 1.f,1.f,1.f };
		int parentID = -1;
		XMFLOAT4X4 local;
		XMFLOAT4X4 world;
		XMFLOAT4X4 offset;
		HierarchyNode()
		{
			XMStoreFloat4x4(&local, XMMatrixIdentity());
			XMStoreFloat4x4(&world, XMMatrixIdentity());
			XMStoreFloat4x4(&offset, XMMatrixIdentity());
		}
	};

	struct VertexType
	{
		XMFLOAT3 position = { 0.f,0.f,0.f };
		XMFLOAT2 uv = { 0.f,0.f };
		XMFLOAT3 normal = { 0.f,0.f,0.f };
		XMFLOAT3 tangent = { 0.f,0.f,0.f };
		XMUINT4 boneid = { 0,0,0,0 };
		XMFLOAT4 weight = { 0.f,0.f,0.f,0.f };
	};

	struct InstanceType
	{
		XMFLOAT4 attr = { 0.f,0.f,0.f,0.f };
	};

	struct Mesh
	{
		UINT matrialID = 0;
		UINT indexCount = 0;
		ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
		ComPtr<ID3D11Buffer> indexBuffer = nullptr;
		ComPtr<ID3D11Buffer> meshBuffer = nullptr;//메쉬 하이어라키 위치버퍼
	};

	struct MaterialData
	{
		XMFLOAT3 diffuse = { 0.f,0.f,0.f };
		FLOAT bumpscaling = 0.f;
		XMFLOAT3 ambient = { 0.f,0.f,0.f };
		FLOAT opacity = 0.f;
		XMFLOAT3 specular = { 0.f,0.f,0.f };
		FLOAT shininess = 0.f;
		XMFLOAT3 emissive = { 0.f,0.f,0.f };
		FLOAT shininessstrength = 0.f;
		XMFLOAT3 tranparent = { 1.f,1.f,1.f };
		FLOAT transparentfactor = 0.f;
		XMFLOAT3 reflective = { 0.f,0.f,0.f };
		FLOAT reflectivity = 0.f;

		FLOAT refracti = 0.f;
		INT opaque = 1;
		INT reflector = 0;
		INT hasDiffuseMap = 0;

		INT hasSpecularMap = 0;
		INT hasAmbientMap = 0;
		INT hasEmissiveMap = 0;
		INT hasHeightMap = 0;

		INT hasNormalMap = 0;
		INT hasShininessMap = 0;
		INT hasOpacityMap = 0;
		INT hasDisplacementMap = 0;

		INT hasLightMapMap = 0;
		INT hasReflectionMap = 0;
		INT hasBasecolorMap = 0;
		INT hasNormalcameraMap = 0;

		INT hasEmissioncolorMap = 0;
		INT hasMetalnessMap = 0;
		INT hasDiffuseroughnessMap = 0;
		INT hasAmbientocculsionMap = 0;

		//TODO : 메터리얼 변수값 나중에 셰이더마다 필요한것만 네이밍 규칙정해서 일괄로 처리하자
	};

	struct Material
	{
		MaterialData matData;
		ComPtr<ID3D11Buffer> matBuffer = nullptr;//다이나믹으로 컨트롤가능 메터리얼 데이터

		Texture* diffusesrv = nullptr;
		Texture* specularsrv = nullptr;
		Texture* normalsrv = nullptr;
		Texture* ambientsrv = nullptr;

	};

	struct KEYFRAME
	{
		float tickPerSec = 0.f;
		float duration = 0.f;
		float blendLoop = 0.f;
		bool isLoop = true;
		vector< vector<XMFLOAT3X4>> keyframes;
	};

	struct ANIMATIONCTRL
	{
		float speed = 1.f;
		float fadeIn = 0.f;
		float fadeOut = 0.f;
	};

	struct ANIMATION
	{
		int clip = 0;
		float curtime = 0.f;
		float duration = 1.f;
		float blendLoop = 0.f;
	};

	struct INSTANCEANIMATION
	{
		ANIMATION cur;
		ANIMATION next;

		float blendFactor = 0.f;
	};

public:
	AnimationModel(string path);
	~AnimationModel();

	Transform* AddInstance();

	void LoadAnimation(const string path, float blendLoop = 1.f, bool isLoop = true);

	void Update();
	void PreRender();
	void Render();
	void PostRender();

	void PlayAni(UINT instanceID, int index, float fadeIn = 0.f, float  fadeOut = 0.f)
	{
		if (keyFrames.size() <= index || (keyFrames[(UINT)inatanceAnims[instanceID].cur.clip].isLoop && inatanceAnims[instanceID].cur.clip == index))
			return;

		inatanceAnims[instanceID].next = inatanceAnims[instanceID].cur;
		inatanceAnims[instanceID].cur.clip = index;
		inatanceAnims[instanceID].cur.curtime = 0.f;
		inatanceAnims[instanceID].cur.duration = keyFrames[index].duration;
		inatanceAnims[instanceID].cur.blendLoop = keyFrames[index].blendLoop;
		inatanceAnims[instanceID].blendFactor = 0.f;
		inatanceAnimCtrl[instanceID].fadeIn = fadeIn < keyFrames[index].duration ? fadeIn : keyFrames[index].duration - 1;
		inatanceAnimCtrl[instanceID].fadeOut = keyFrames[index].isLoop ? 0.f : (fadeOut < keyFrames[index].duration ? fadeOut : keyFrames[index].duration - 1);
	}
	void StopAni() { }
	void PauseAni() { }

	void SetSpeed(UINT instanceID, float speed) { inatanceAnimCtrl[instanceID].speed = speed; }
	void SetDiffuseMap(wstring filePath, UINT matIndex);
	void SetSpecularMap(wstring filePath, UINT matIndex);
	void SetAmbientMap(wstring filePath, UINT matIndex);
	void SetNormalMap(wstring filePath, UINT matIndex);


	void SetEndEvent(int index, function<void()> Event) { }
	void SetNextAction(int index, function<void(int)> Event) { }

	void SetTexture(string name, string attr, string texPath);

	ID3D11ShaderResourceView* GetTexture(string name)
	{
		if (materialMap.count(name))
			return materials[materialMap[name]].diffusesrv->GetSRV();
		return nullptr;
	}
	UINT GetInstanceSize()
	{
		return inatanceAnims.size();
	}
	IntBuffer* GetIntBuffer()
	{
		return intBuffer;
	}
private:
	void LoadModel(const string path);

	void CreateAnimSRV();

	void AnimUpdate();

	Shader* shader;
	ID3D11ComputeShader* computeShader;
	IntBuffer* intBuffer;

	//컴퓨트 셰이더용 하이어라키 베이스 파트
	vector<HierarchyNode> hierarchyNodes; //인덱스를 통해서는 직접조작
	map<string, UINT> hierarchyMap;//하이어라키 맵을 이용해서 이름으로 찾아 조작
	ID3D11ShaderResourceView* hierarchyBufferSrv = nullptr;//하이어라키버퍼 : preQuat, local(키없이 메트릭스 반환시), world(상위에 키가없을시 월드메트릭스), offset, parentID, enable(컴퓨트 불필요한 스레드계산제거)
	UINT nodeCount;

	//메쉬 랜더링 파트
	vector<Mesh> meshs;
	vector<Material> materials;
	map<string, UINT> materialMap;
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	//인스턴싱용 파츠
	UINT instanceStride = sizeof(InstanceType);
	vector<Transform*> instance;
	vector<XMFLOAT4X4> instanceMatrix;
	ComPtr<ID3D11Buffer> instanceBuffer = nullptr;//버텍스 또는 픽셀제어용 어트리뷰트만 생성해둠
	ComPtr<ID3D11Texture1D> transformTex = nullptr;//인스턴스마다의 트랜스폼 변경용 1D텍스쳐 인스턴스 버퍼에 넣어도 되지만 일단 텍스쳐로 처리
	ComPtr<ID3D11ShaderResourceView> transformSRV = nullptr;//위의 텍스쳐를 버텍스 셰이더로 보내는 용도

	//컴퓨트 셰이더용 인스턴싱 애니메이션 파트
	vector<ANIMATIONCTRL> inatanceAnimCtrl;//인스턴스용 애니메이션 컨트롤(스피드,페이드인아웃,이벤트 등)
	vector<INSTANCEANIMATION> inatanceAnims;//인스턴스용 애니메이션 데이터
	ComPtr<ID3D11Buffer> inatanceAnimBuffer = nullptr;//컴퓨트 셰이더용 버퍼
	ComPtr<ID3D11ShaderResourceView> inatanceAnimSRV = nullptr;//위의 텍스쳐를 컴퓨트 셰이더로 보내는 용도

	//애니메이션 키프레임 데이터 파트
	vector<KEYFRAME> keyFrames;//불러들인 전체 애니메이션 저장용 벡터

	//컴퓨트 셰이더용 버퍼 파트
	ComPtr<ID3D11ShaderResourceView> animSRV = nullptr;//위의 키프레임 데이터를 갖는 애니메이션 SRV
	ComPtr<ID3D11UnorderedAccessView> hierarchyMatrixUAV = nullptr;//컴퓨트 셰이더 임시 저장용 UAV
	vector<XMFLOAT4X4> hierarchyMatrix;//컴퓨트 셰이더 RW버퍼 리턴시 결과값 저장용 매트릭스 벡터
	ComPtr<ID3D11UnorderedAccessView> hierarchyUAV = nullptr;//컴퓨트 셰이더용 RW버퍼 2DArray로 위의 벡터에서 받은 값을 기본으로 가진상태로 만들어짐
	ComPtr<ID3D11ShaderResourceView> hierarchySRV = nullptr;//버텍스 셰이더용 하이어라키 SRV
};

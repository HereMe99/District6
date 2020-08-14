#include "Framework.h"

AnimationModel::AnimationModel(string path)
{
	intBuffer = NEW IntBuffer();
	assert(Path::ExistFile(path));
	LoadModel(path);
	computeShader = SHADER->ComputeShader(L"../Framework/SelfShader/HLSL/ModelCS.hlsl", "main");
	shader = Shader::Add(L"ModelAni");

}

AnimationModel::~AnimationModel()
{
	SAFE_DELETE(intBuffer);
	SAFE_RELEASE(hierarchyBufferSrv);
	vector<HierarchyNode>().swap(hierarchyNodes);
	map<string, UINT>().swap(hierarchyMap);
	hierarchyMap.clear();
	vector<Mesh>().swap(meshs);
	//vector<Material>().swap(materials);
	//map<string, UINT>().swap(materialMap);
	materialMap.clear();
	for (auto& i : instance)
		SAFE_DELETE(i);
	vector<Transform*>().swap(instance);
	vector<XMFLOAT4X4>().swap(instanceMatrix);
	for (auto& i : keyFrames)
	{
		for (UINT k = 0; k < i.keyframes.size(); k++)
			vector<XMFLOAT3X4>().swap(i.keyframes[k]);
		vector< vector<XMFLOAT3X4>>().swap(i.keyframes);
	}
	vector<KEYFRAME>().swap(keyFrames);
	vector<XMFLOAT4X4>().swap(hierarchyMatrix);
}

Transform* AnimationModel::AddInstance()
{
	Transform* NEWTransform = NEW Transform();

	//인스턴스별 월드 맵 콘스트버퍼어레이보다 맵으로 하는게 사용량을 늘릴수 있어서 맵사용. 비슷한 스트럭드버퍼도 맵으로 들어가기때문에 염두해볼만함 대신 트랜스포즈 시켜서 보내야함
	instance.emplace_back(NEWTransform);
	instanceMatrix.emplace_back(XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
	D3D11_TEXTURE1D_DESC transformDesc = {};
	transformDesc.Width = 4 * (UINT)instance.size();
	transformDesc.ArraySize = 1;
	transformDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	transformDesc.Usage = D3D11_USAGE_DYNAMIC;
	transformDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	transformDesc.MipLevels = 1;
	transformDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	transformDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA transformData;
	transformData.pSysMem = instanceMatrix.data();
	transformData.SysMemPitch = 0;
	transformData.SysMemSlicePitch = 0;

	V(DEVICE->CreateTexture1D(&transformDesc, &transformData, transformTex.ReleaseAndGetAddressOf()));
	Path::SetDebugName(transformTex.Get(), "transformTex");
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourcedesc = {};
	shaderResourcedesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderResourcedesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	shaderResourcedesc.Texture1D.MipLevels = 1;

	V(DEVICE->CreateShaderResourceView(transformTex.Get(), &shaderResourcedesc, transformSRV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(transformSRV.Get(), "transformSRV");

	//GPU캐시대신 노드갯수에 맞는 언오더드맵. 갯수를 미리 정해 놓는 캐시 맵과 차이가 없음 더 적은량을 정해둬서 그런듯. 캐시는 갯수를 미리 여유있게 안하면 의미 없어짐
	D3D11_TEXTURE2D_DESC hierarchyMatrixTextureDesc;
	hierarchyMatrixTextureDesc.Width = 4 * nodeCount;
	hierarchyMatrixTextureDesc.Height = (UINT)instance.size();//인스턴싱갯수
	hierarchyMatrixTextureDesc.ArraySize = 1;
	hierarchyMatrixTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hierarchyMatrixTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	hierarchyMatrixTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hierarchyMatrixTextureDesc.MipLevels = 1;
	hierarchyMatrixTextureDesc.CPUAccessFlags = 0;
	hierarchyMatrixTextureDesc.MiscFlags = 0;
	hierarchyMatrixTextureDesc.SampleDesc.Count = 1;
	hierarchyMatrixTextureDesc.SampleDesc.Quality = 0;

	ComPtr<ID3D11Texture2D> hierarchyMatrixTex = nullptr;
	V(DEVICE->CreateTexture2D(&hierarchyMatrixTextureDesc, nullptr, hierarchyMatrixTex.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyMatrixTex.Get(), "hierarchyMatrixTex");

	D3D11_UNORDERED_ACCESS_VIEW_DESC hierarchyMatrixUavDesc;//컴퓨트 셰이더에서 쓰기를 할 셰이더 리소스, 버텍스 셰이더 리소스랑 값이 공유된다.
	hierarchyMatrixUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	hierarchyMatrixUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	hierarchyMatrixUavDesc.Texture2D.MipSlice = 0;
	V(DEVICE->CreateUnorderedAccessView(hierarchyMatrixTex.Get(), &hierarchyMatrixUavDesc, hierarchyMatrixUAV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyMatrixUAV.Get(), "hierarchyMatrixUAV");

	//srv와 uav는 데이터를 공유한다. 하이어라키srv는 애니메이션과 offset을 계산한 결과값 메트릭스만 가진다.
	//매트릭스구조는 U = 메트릭스 행 * 하이어라키넘버 V=10프레임 이전것까지 기록한다
	//srv라 트랜스포즈 불필요
	D3D11_TEXTURE2D_DESC hierarchyTextureDesc;
	hierarchyTextureDesc.Width = 4 * nodeCount;
	hierarchyTextureDesc.Height = 10;
	hierarchyTextureDesc.ArraySize = (UINT)instance.size();
	hierarchyTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hierarchyTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	hierarchyTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hierarchyTextureDesc.MipLevels = 1;
	hierarchyTextureDesc.CPUAccessFlags = 0;
	hierarchyTextureDesc.MiscFlags = 0;
	hierarchyTextureDesc.SampleDesc.Count = 1;
	hierarchyTextureDesc.SampleDesc.Quality = 0;

	UINT ins = (UINT)hierarchyMatrix.size() / nodeCount / 10;//메쉬 생성시 만들어진 매트릭스로 전부 넣어준다.
	for (; ins < (UINT)instance.size(); ins++)
	{
		for (UINT i = 0; i < 10; i++)
		{
			for (UINT j = 0; j < nodeCount; j++)
			{
				hierarchyMatrix.emplace_back(hierarchyMatrix[j]);
			}
		}
	}

	D3D11_SUBRESOURCE_DATA* hierarchyTextureData = NEW D3D11_SUBRESOURCE_DATA[hierarchyTextureDesc.ArraySize];//하이어라키srv는 초기 하이어라키 메트릭스 및 offset적용 매트릭스 넣어주기
	for (UINT i = 0; i < hierarchyTextureDesc.ArraySize; i++)
	{
		hierarchyTextureData[i].pSysMem = hierarchyMatrix.data();
		hierarchyTextureData[i].SysMemPitch = nodeCount * sizeof(XMFLOAT4X4);
		hierarchyTextureData[i].SysMemSlicePitch = sizeof(XMFLOAT4X4) * nodeCount * 10;
	}

	ComPtr<ID3D11Texture2D> hierarchyTex = nullptr;
	V(DEVICE->CreateTexture2D(&hierarchyTextureDesc, hierarchyTextureData, hierarchyTex.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyTex.Get(), "hierarchyTex");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;//버텍스 셰이더에서 사용할 셰이더 리소스
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = (UINT)instance.size();
	V(DEVICE->CreateShaderResourceView(hierarchyTex.Get(), &srvDesc, hierarchySRV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchySRV.Get(), "hierarchySRV");

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;//컴퓨트 셰이더에서 쓰기를 할 셰이더 리소스, 버텍스 셰이더 리소스랑 값이 공유된다.
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.ArraySize = (UINT)instance.size();
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	V(DEVICE->CreateUnorderedAccessView(hierarchyTex.Get(), &uavDesc, hierarchyUAV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyUAV.Get(), "hierarchyUAV");

	SAFE_DELETE_ARRAY(hierarchyTextureData);

	InstanceType* instancies = NEW InstanceType[instance.size()];

	D3D11_BUFFER_DESC instanceBufferDesc;
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType) * (UINT)instance.size();
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	instanceData.pSysMem = instancies;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	V(DEVICE->CreateBuffer(&instanceBufferDesc, &instanceData, instanceBuffer.ReleaseAndGetAddressOf()));
	Path::SetDebugName(instanceBuffer.Get(), "instanceBuffer");

	SAFE_DELETE_ARRAY(instancies);

	INSTANCEANIMATION anim;
	if (!keyFrames.empty())
	{
		anim.cur.duration = keyFrames.front().duration;
		anim.cur.blendLoop = keyFrames.front().blendLoop;
	}
	inatanceAnims.emplace_back(anim);
	ANIMATIONCTRL ctrl;
	inatanceAnimCtrl.emplace_back(ctrl);

	D3D11_BUFFER_DESC inatanceAnimBufferDesc = {};//애니메이션 버퍼
	inatanceAnimBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inatanceAnimBufferDesc.ByteWidth = sizeof(INSTANCEANIMATION) * (UINT)inatanceAnims.size();
	inatanceAnimBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inatanceAnimBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inatanceAnimBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	inatanceAnimBufferDesc.StructureByteStride = sizeof(INSTANCEANIMATION);

	D3D11_SUBRESOURCE_DATA inatanceAnimBufferData;
	inatanceAnimBufferData.pSysMem = inatanceAnims.data();
	inatanceAnimBufferData.SysMemPitch = 0;
	inatanceAnimBufferData.SysMemSlicePitch = 0;

	V(DEVICE->CreateBuffer(&inatanceAnimBufferDesc, &inatanceAnimBufferData, inatanceAnimBuffer.ReleaseAndGetAddressOf()));
	Path::SetDebugName(inatanceAnimBuffer.Get(), "inatanceAnimBuffer");

	D3D11_SHADER_RESOURCE_VIEW_DESC inatanceAnimBufferSrvDesc = {};
	inatanceAnimBufferSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	inatanceAnimBufferSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	inatanceAnimBufferSrvDesc.BufferEx.FirstElement = 0;
	inatanceAnimBufferSrvDesc.BufferEx.NumElements = (UINT)inatanceAnims.size();

	V(DEVICE->CreateShaderResourceView(inatanceAnimBuffer.Get(), &inatanceAnimBufferSrvDesc, inatanceAnimSRV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(inatanceAnimSRV.Get(), "inatanceAnimSRV");

	return NEWTransform;
}

void AnimationModel::LoadAnimation(const string path, float blendLoop, bool isLoop)
{
	assert(Path::ExistFile(path));
	BinaryReader* r = NEW BinaryReader(path);

	KEYFRAME keyFrame;

	keyFrame.tickPerSec = r->Float();
	keyFrame.duration = r->Float();
	keyFrame.blendLoop = blendLoop > keyFrame.duration ? keyFrame.duration : blendLoop;
	keyFrame.isLoop = isLoop;

	UINT keyCount = r->UInt();
	for (UINT i = 0; i < keyCount; i++)
	{
		UINT count = r->UInt();
		assert(count == nodeCount);
		vector<XMFLOAT3X4> Key;
		for (UINT j = 0; j < count; j++)
		{
			XMFLOAT3X4 data = { 0,0,0,0,0,0,0,1,1,1,1,1 };
			void* ptr = &data.m[0];
			r->Byte(&ptr, sizeof(XMFLOAT3));
			data._14 = r->Float();
			ptr = &data.m[1];
			r->Byte(&ptr, sizeof(XMFLOAT4));
			ptr = &data.m[2];
			r->Byte(&ptr, sizeof(XMFLOAT3));
			Key.emplace_back(data);
		}
		keyFrame.keyframes.emplace_back(Key);
	}
	keyFrames.emplace_back(keyFrame);

	SAFE_DELETE(r);

	CreateAnimSRV();
}

void AnimationModel::Update()
{
	UINT ind = 0;
	for (Transform* i : instance)//인스턴스별 트랜스폼 월드 업데이트
	{
		i->UpdateWorld();
		XMStoreFloat4x4(&instanceMatrix[ind], i->GetWorld());
		ind++;
	}

	AnimUpdate();
}

void AnimationModel::Render()
{


	IASetPT();
	intBuffer->SetPSBuffer(10);

	D3D11_MAPPED_SUBRESOURCE subResource;//인스턴스 월드맵 갱신
	DC->Map(transformTex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	memcpy(subResource.pData, instanceMatrix.data(), sizeof(XMFLOAT4X4) * instance.size());
	DC->Unmap(transformTex.Get(), 0);

	DC->VSSetShaderResources(6, 1, transformSRV.GetAddressOf());
	DC->VSSetShaderResources(7, 1, hierarchySRV.GetAddressOf());
	shader->Set();
	for (int i = 0; i < 2; i++)
	{
		for (Mesh mesh : meshs)
		{
			if ((!materials[mesh.matrialID].matData.opaque && i == 0) || (materials[mesh.matrialID].matData.opaque && i == 1))
				continue;

			DC->VSSetConstantBuffers(11, 1, mesh.meshBuffer.GetAddressOf());

			if (materials[mesh.matrialID].diffusesrv != nullptr)
				materials[mesh.matrialID].diffusesrv->Set(0);
			if (materials[mesh.matrialID].specularsrv != nullptr)
				materials[mesh.matrialID].specularsrv->Set(1);
			if (materials[mesh.matrialID].normalsrv != nullptr)
				materials[mesh.matrialID].normalsrv->Set(2);
			if (materials[mesh.matrialID].ambientsrv != nullptr)
				materials[mesh.matrialID].ambientsrv->Set(5);

			DC->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
			DC->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


			DC->DrawIndexedInstanced(mesh.indexCount, static_cast<UINT>(instance.size()), 0, 0, 0);
		}
	}
}

void AnimationModel::SetDiffuseMap(const wstring filePath, const UINT matIndex)
{
	materials[matIndex].diffusesrv = Texture::Add(filePath);
}

void AnimationModel::SetSpecularMap(const wstring filePath, const UINT matIndex)
{
	materials[matIndex].specularsrv = Texture::Add(filePath);
}

void AnimationModel::SetAmbientMap(const wstring filePath, const UINT matIndex)
{
	materials[matIndex].ambientsrv = Texture::Add(filePath);

}

void AnimationModel::SetNormalMap(const wstring filePath, const UINT matIndex)
{
	materials[matIndex].normalsrv = Texture::Add(filePath);

}

void AnimationModel::LoadModel(const string path)
{
	BinaryReader* r = NEW BinaryReader(path);

	nodeCount = r->UInt();

	struct HierarchyDATA
	{
		XMFLOAT4 preQuat = { 0,0,0,1 };
		XMFLOAT4X4 local = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
		XMFLOAT4X4 world = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
		XMFLOAT4X4 offset = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

		int parentID = -1;
		UINT enable = 0;
	};

	vector<HierarchyDATA> hierarchyDatas(nodeCount);//버퍼용 초기셋팅 1번만
	for (UINT i = 0; i < nodeCount; i++)
	{
		HierarchyNode node;

		string name = r->String();
		void* ptr = &node.translate;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		ptr = &node.preQuaternion;
		r->Byte(&ptr, sizeof(XMFLOAT4));
		ptr = &node.quaternion;
		r->Byte(&ptr, sizeof(XMFLOAT4));
		ptr = &node.scale;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		//r->Byte(reinterpret_cast<void**>(&node.translate), sizeof(XMFLOAT3));
		//r->Byte(reinterpret_cast<void**>(&node.preQuaternion), sizeof(XMFLOAT4));
		//r->Byte(reinterpret_cast<void**>(&node.quaternion), sizeof(XMFLOAT4));
		//r->Byte(reinterpret_cast<void**>(&node.scale), sizeof(XMFLOAT3));
		node.local = r->Matrix();
		node.world = r->Matrix();
		node.offset = r->Matrix();
		node.parentID = r->Int();

		hierarchyNodes.emplace_back(node);
		hierarchyMap.emplace(name, i);

		hierarchyDatas[i].preQuat = node.preQuaternion;
		XMStoreFloat4x4(&hierarchyDatas[i].local, XMMatrixTranspose(XMLoadFloat4x4(&node.local)));
		XMStoreFloat4x4(&hierarchyDatas[i].world, XMMatrixTranspose(XMLoadFloat4x4(&node.world)));
		XMStoreFloat4x4(&hierarchyDatas[i].offset, XMMatrixTranspose(XMLoadFloat4x4(&node.offset)));
		hierarchyDatas[i].parentID = node.parentID;
		hierarchyDatas[i].enable = 1;
	}
	//하이어라키버퍼 : preQuat, local(키없이 메트릭스 반환시), world(상위에 키가없을시 페어런트메트릭스), offset, parentID, enable(컴퓨트 불필요한 스레드계산제거시)
	D3D11_BUFFER_DESC hierarchyBufferDesc = {};//하이어라키버퍼에서의 위치
	hierarchyBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;//일단 변경할일 없다 치고 월드나 값을 변경할수도 있음
	hierarchyBufferDesc.ByteWidth = sizeof(HierarchyDATA) * nodeCount;
	hierarchyBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	hierarchyBufferDesc.CPUAccessFlags = 0;
	hierarchyBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	hierarchyBufferDesc.StructureByteStride = sizeof(HierarchyDATA);

	D3D11_SUBRESOURCE_DATA hierarchyBufferData;
	hierarchyBufferData.pSysMem = hierarchyDatas.data();
	hierarchyBufferData.SysMemPitch = 0;
	hierarchyBufferData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> hierarchyBuffer = nullptr;
	V(DEVICE->CreateBuffer(&hierarchyBufferDesc, &hierarchyBufferData, hierarchyBuffer.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyBuffer.Get(), "hierarchyBuffer");

	D3D11_SHADER_RESOURCE_VIEW_DESC hierarchyBufferSrvDesc = {};
	hierarchyBufferSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	hierarchyBufferSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	hierarchyBufferSrvDesc.BufferEx.FirstElement = 0;
	hierarchyBufferSrvDesc.BufferEx.NumElements = nodeCount;

	V(DEVICE->CreateShaderResourceView(hierarchyBuffer.Get(), &hierarchyBufferSrvDesc, &hierarchyBufferSrv));
	//Path::SetDebugName(hierarchyBufferSrv->Get(), "hierarchyBufferSrv");

	//하이어라키 오프셋 결과 매트릭스 생성
	for (UINT i = 0; i < 10; i++)
	{
		for (UINT j = 0; j < nodeCount; j++)
		{
			XMFLOAT4X4 temp;
			if (i == 0)
			{
				XMStoreFloat4x4(&temp, XMMatrixMultiply(XMLoadFloat4x4(&hierarchyNodes[j].offset), XMLoadFloat4x4(&hierarchyNodes[j].world)));
			}
			else
			{
				temp = hierarchyMatrix[j];
			}
			hierarchyMatrix.emplace_back(temp);
		}
	}
	vector<HierarchyDATA>().swap(hierarchyDatas);

	//GPU캐시대신 노드갯수에 맞는 언오더드맵. 갯수를 미리 정해 놓는 캐시 맵과 차이가 없음 더 적은량을 정해둬서 그런듯. 캐시는 갯수를 미리 여유있게 안하면 의미 없어짐
	D3D11_TEXTURE2D_DESC hierarchyMatrixTextureDesc;
	hierarchyMatrixTextureDesc.Width = 4 * nodeCount;
	hierarchyMatrixTextureDesc.Height = 1;//인스턴싱갯수
	hierarchyMatrixTextureDesc.ArraySize = 1;
	hierarchyMatrixTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hierarchyMatrixTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	hierarchyMatrixTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hierarchyMatrixTextureDesc.MipLevels = 1;
	hierarchyMatrixTextureDesc.CPUAccessFlags = 0;
	hierarchyMatrixTextureDesc.MiscFlags = 0;
	hierarchyMatrixTextureDesc.SampleDesc.Count = 1;
	hierarchyMatrixTextureDesc.SampleDesc.Quality = 0;

	ComPtr<ID3D11Texture2D> hierarchyMatrixTex = nullptr;
	V(DEVICE->CreateTexture2D(&hierarchyMatrixTextureDesc, nullptr, hierarchyMatrixTex.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyMatrixTex.Get(), "hierarchyMatrixTex");
	D3D11_UNORDERED_ACCESS_VIEW_DESC hierarchyMatrixUavDesc;
	hierarchyMatrixUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	hierarchyMatrixUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	hierarchyMatrixUavDesc.Texture2D.MipSlice = 0;
	V(DEVICE->CreateUnorderedAccessView(hierarchyMatrixTex.Get(), &hierarchyMatrixUavDesc, hierarchyMatrixUAV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyMatrixUAV.Get(), "hierarchyMatrixUAV");
	//srv와 uav는 데이터를 공유한다. 하이어라키srv는 애니메이션과 offset을 계산한 결과값 메트릭스만 가진다.
	//매트릭스구조는 U = 메트릭스 행 * 하이어라키넘버 V=10프레임 이전것까지 기록한다
	//srv라 트랜스포즈 불필요
	D3D11_TEXTURE2D_DESC hierarchyTextureDesc;
	hierarchyTextureDesc.Width = 4 * nodeCount;
	hierarchyTextureDesc.Height = 10;
	hierarchyTextureDesc.ArraySize = 1;
	hierarchyTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	hierarchyTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	hierarchyTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hierarchyTextureDesc.MipLevels = 1;
	hierarchyTextureDesc.CPUAccessFlags = 0;
	hierarchyTextureDesc.MiscFlags = 0;
	hierarchyTextureDesc.SampleDesc.Count = 1;
	hierarchyTextureDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA* hierarchyTextureData = NEW D3D11_SUBRESOURCE_DATA[hierarchyTextureDesc.ArraySize];//하이어라키srv는 초기 하이어라키 메트릭스 및 offset적용 매트릭스 넣어주기
	for (UINT i = 0; i < hierarchyTextureDesc.ArraySize; i++)
	{
		hierarchyTextureData[i].pSysMem = hierarchyMatrix.data();
		hierarchyTextureData[i].SysMemPitch = nodeCount * sizeof(XMFLOAT4X4);
		hierarchyTextureData[i].SysMemSlicePitch = sizeof(XMFLOAT4X4) * nodeCount * 10;
	}

	ComPtr<ID3D11Texture2D> hierarchyTex = nullptr;
	V(DEVICE->CreateTexture2D(&hierarchyTextureDesc, hierarchyTextureData, hierarchyTex.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyTex.Get(), "hierarchyTex");
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;//버텍스 셰이더에서 사용할 셰이더 리소스
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = hierarchyTextureDesc.ArraySize;
	V(DEVICE->CreateShaderResourceView(hierarchyTex.Get(), &srvDesc, hierarchySRV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchySRV.Get(), "hierarchySRV");
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;//컴퓨트 셰이더에서 쓰기를 할 셰이더 리소스, 버텍스 셰이더 리소스랑 값이 공유된다.
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.ArraySize = hierarchyTextureDesc.ArraySize;
	uavDesc.Texture2DArray.FirstArraySlice = 0;
	V(DEVICE->CreateUnorderedAccessView(hierarchyTex.Get(), &uavDesc, hierarchyUAV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(hierarchyUAV.Get(), "hierarchyUAV");
	SAFE_DELETE_ARRAY(hierarchyTextureData);

	UINT count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		Mesh mesh;

		struct DATA
		{
			UINT id;
			float padding[3];
		}data;
		data.id = r->UInt();
		//하이어라키버퍼에서의 메쉬위치
		D3D11_BUFFER_DESC mesh_buffer_desc = {};
		mesh_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		mesh_buffer_desc.ByteWidth = sizeof(DATA);
		mesh_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		mesh_buffer_desc.CPUAccessFlags = 0;
		mesh_buffer_desc.MiscFlags = 0;
		mesh_buffer_desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA meshData;
		meshData.pSysMem = &data;
		meshData.SysMemPitch = 0;
		meshData.SysMemSlicePitch = 0;

		V(DEVICE->CreateBuffer(&mesh_buffer_desc, &meshData, mesh.meshBuffer.ReleaseAndGetAddressOf()));
		Path::SetDebugName(mesh.meshBuffer.Get(), "meshBuffer");
		mesh.matrialID = r->UInt();

		UINT vertexCount = r->UInt();
		VertexType* vertices = NEW VertexType[vertexCount];
		r->Byte(reinterpret_cast<void**>(&vertices), sizeof(VertexType)* vertexCount);

		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		V(DEVICE->CreateBuffer(&vertexBufferDesc, &vertexData, mesh.vertexBuffer.ReleaseAndGetAddressOf()));
		Path::SetDebugName(mesh.vertexBuffer.Get(), "vertexBuffer");
		mesh.indexCount = r->UInt();
		UINT* indices = NEW UINT[mesh.indexCount];
		r->Byte(reinterpret_cast<void**>(&indices), sizeof(UINT)* mesh.indexCount);

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = sizeof(UINT) * mesh.indexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		V(DEVICE->CreateBuffer(&indexBufferDesc, &indexData, mesh.indexBuffer.ReleaseAndGetAddressOf()));
		Path::SetDebugName(mesh.indexBuffer.Get(), "indexBuffer");
		meshs.emplace_back(mesh);

		SAFE_DELETE_ARRAY(vertices);
		SAFE_DELETE_ARRAY(indices);
	}

	count = r->UInt();
	for (UINT i = 0; i < count; i++)
	{
		string name = r->String();
		materialMap.emplace(name, i);

		Material mat;
		void* ptr = &mat.matData.diffuse;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		ptr = &mat.matData.ambient;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		ptr = &mat.matData.specular;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		ptr = &mat.matData.emissive;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		ptr = &mat.matData.tranparent;
		r->Byte(&ptr, sizeof(XMFLOAT3));
		ptr = &mat.matData.reflective;
		r->Byte(&ptr, sizeof(XMFLOAT3));


		mat.matData.opacity = r->Float();
		mat.matData.transparentfactor = r->Float();
		mat.matData.bumpscaling = r->Float();
		mat.matData.shininess = r->Float();
		mat.matData.reflectivity = r->Float();
		mat.matData.shininessstrength = r->Float();
		mat.matData.refracti = r->Float();

		mat.diffusesrv = Texture::Add(Path::ToWString(r->String()));
		mat.specularsrv = Texture::Add(Path::ToWString(r->String()));
		if(mat.specularsrv != nullptr)
		{
			intBuffer->data.indices[0] = 1;
		}
		mat.ambientsrv = Texture::Add(Path::ToWString(r->String()));
		if (mat.ambientsrv != nullptr)
		{
			intBuffer->data.indices[2] = 1;
		}
		mat.normalsrv = Texture::Add(Path::ToWString(r->String()));
		if (mat.normalsrv != nullptr)
		{
			intBuffer->data.indices[1] = 1;
		}
		mat.ambientsrv = Texture::Add(Path::ToWString(r->String()));
		if (mat.normalsrv != nullptr)
		{
			intBuffer->data.indices[2] = 1;
		}
		


		if (mat.diffusesrv != NULL)
			mat.matData.hasDiffuseMap = 1;
		if (mat.specularsrv != NULL)
			mat.matData.hasSpecularMap = 1;
		if (mat.ambientsrv != NULL)
			mat.matData.hasAmbientMap = 1;
		D3D11_BUFFER_DESC matBufferDesc = {};
		matBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		matBufferDesc.ByteWidth = sizeof(MaterialData);
		matBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA matData;
		matData.pSysMem = &mat.matData;
		matData.SysMemPitch = 0;
		matData.SysMemSlicePitch = 0;

		V(DEVICE->CreateBuffer(&matBufferDesc, &matData, mat.matBuffer.ReleaseAndGetAddressOf()));
		Path::SetDebugName(mat.matBuffer.Get(), "mat.matBuffer");
		materials.emplace_back(mat);
	}

	SAFE_DELETE(r);
}

void AnimationModel::CreateAnimSRV()
{
	UINT animCount = (UINT)keyFrames.size();
	UINT maxKeyCount = 0;
	for (KEYFRAME i : keyFrames)
	{
		if (i.keyframes.size() > maxKeyCount)
			maxKeyCount = (UINT)i.keyframes.size();
	}

	D3D11_TEXTURE2D_DESC tex2Ddesc = {};
	tex2Ddesc.Width = nodeCount * 3;
	tex2Ddesc.Height = maxKeyCount;
	tex2Ddesc.ArraySize = animCount;
	tex2Ddesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tex2Ddesc.Usage = D3D11_USAGE_IMMUTABLE;
	tex2Ddesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex2Ddesc.MipLevels = 1;
	tex2Ddesc.SampleDesc.Count = 1;

	UINT pageSize = sizeof(XMFLOAT3X4) * nodeCount * maxKeyCount;
	vector< vector<XMFLOAT3X4>> keyData;
	for (UINT anim = 0; anim < animCount; anim++)
	{
		vector<XMFLOAT3X4> data;
		for (UINT key = 0; key < keyFrames[anim].keyframes.size(); key++)
		{
			data.insert(data.begin() + (size_t)nodeCount * key, keyFrames[anim].keyframes[key].begin(), keyFrames[anim].keyframes[key].end());
		}
		data.resize((size_t)nodeCount * maxKeyCount, { 0,0,0,0,0,0,0,1,1,1,1,1 });
		keyData.emplace_back(data);
	}

	D3D11_SUBRESOURCE_DATA* subResource = NEW D3D11_SUBRESOURCE_DATA[animCount];
	for (UINT anim = 0; anim < animCount; anim++)
	{
		subResource[anim].pSysMem = keyData[anim].data();
		subResource[anim].SysMemPitch = nodeCount * sizeof(XMFLOAT3X4);
		subResource[anim].SysMemSlicePitch = pageSize;
	}

	ComPtr<ID3D11Texture2D> animTex = nullptr;
	V(DEVICE->CreateTexture2D(&tex2Ddesc, subResource, animTex.ReleaseAndGetAddressOf()));
	Path::SetDebugName(animTex.Get(), "animTex");
	SAFE_DELETE_ARRAY(subResource);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourcedesc = {};
	shaderResourcedesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderResourcedesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourcedesc.Texture2DArray.MipLevels = 1;
	shaderResourcedesc.Texture2DArray.ArraySize = animCount;

	V(DEVICE->CreateShaderResourceView(animTex.Get(), &shaderResourcedesc, animSRV.ReleaseAndGetAddressOf()));
	Path::SetDebugName(animSRV.Get(), "animSRV");
}

void AnimationModel::AnimUpdate()
{
	//TODO : 애니메이션 타임 방식 정리해야함 AddtionalAnim적용 생각해봐야함.
	if (keyFrames.empty())
		return;

	for (UINT i = 0; i < inatanceAnims.size(); i++)
	{
		inatanceAnims[i].cur.curtime += 
			DELTA * keyFrames[(UINT)inatanceAnims[i].cur.clip].tickPerSec * inatanceAnimCtrl[i].speed;
		if (inatanceAnims[i].cur.curtime > inatanceAnims[i].cur.duration-1)
		{
			if (keyFrames[(UINT)inatanceAnims[i].cur.clip].isLoop)
				inatanceAnims[i].cur.curtime = fmod(inatanceAnims[i].cur.curtime, inatanceAnims[i].cur.duration);
			else
			{
				
				inatanceAnims[i].cur.curtime = inatanceAnims[i].cur.duration-1;
				//    if(event!=nullptr)
				//        event;
				/*ANIMATION tmp;
				tmp = inatanceAnims[i].cur;
				inatanceAnims[i].cur = inatanceAnims[0].cur;*/
				//inatanceAnims[i].next = tmp;
				inatanceAnimCtrl[i].fadeIn = 0.f;
				inatanceAnimCtrl[i].fadeOut = 0.f;
				continue;
			}
			inatanceAnimCtrl[i].fadeIn = 0.f;
		}
		if (inatanceAnims[i].cur.curtime < inatanceAnimCtrl[i].fadeIn)
			inatanceAnims[i].blendFactor = 1.f - (inatanceAnims[i].cur.curtime / inatanceAnimCtrl[i].fadeIn);
		else if (inatanceAnims[i].cur.duration - inatanceAnims[i].cur.curtime < inatanceAnimCtrl[i].fadeOut && keyFrames[(UINT)inatanceAnims[i].cur.clip].isLoop)
		{
			inatanceAnims[i].blendFactor = 1.f - ((inatanceAnims[i].cur.duration - inatanceAnims[i].cur.curtime) / inatanceAnimCtrl[i].fadeOut);
		}
		else
			inatanceAnims[i].blendFactor = 0.f;
		if (inatanceAnims[i].blendFactor > 0.f)
		{
			inatanceAnims[i].next.curtime += DELTA * keyFrames[(UINT)inatanceAnims[i].next.clip].tickPerSec * inatanceAnimCtrl[i].speed;
			if (inatanceAnims[i].next.curtime > inatanceAnims[i].next.duration - 1)
			{
				if (keyFrames[(UINT)inatanceAnims[i].next.clip].isLoop)
					inatanceAnims[i].next.curtime = fmod(inatanceAnims[i].next.curtime, inatanceAnims[i].next.duration);
				else
					inatanceAnims[i].next.curtime = inatanceAnims[i].next.duration - 1;
			}
		}
		else
			inatanceAnims[i].next.curtime = 0.f;
	}

	//셰이더 연결 끊고 해줘야함
	ComPtr<ID3D11ShaderResourceView> emptySRV = nullptr;
	Path::SetDebugName(emptySRV.Get(), "emptySRV");
	DC->VSSetShaderResources(1, 1, emptySRV.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE subResource;
	DC->Map(inatanceAnimBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
	memcpy(subResource.pData, inatanceAnims.data(), sizeof(INSTANCEANIMATION) * inatanceAnims.size());
	DC->Unmap(inatanceAnimBuffer.Get(), 0);

	DC->CSSetShader(computeShader, nullptr, 0);
	//하이어라키버퍼: preQuat, local(키없이 메트릭스 반환시), world(상위에 키가없을시 페어런트메트릭스), offset, parentID, enable(컴퓨트 불필요한 스레드계산제거시)
	DC->CSSetShaderResources(0, 1, &hierarchyBufferSrv);//컴퓨트 셰이더에서 쓰일 하이어라키 정보
	DC->CSSetShaderResources(1, 1, inatanceAnimSRV.GetAddressOf());//인스턴스애니메이션컨트롤 리소스뷰
	DC->CSSetShaderResources(2, 1, animSRV.GetAddressOf());//애니메이션 리소스뷰
	DC->CSSetUnorderedAccessViews(0, 1, hierarchyMatrixUAV.GetAddressOf(), nullptr);//컴퓨트중간 하이어라키 매트릭스용 UAV
	DC->CSSetUnorderedAccessViews(1, 1, hierarchyUAV.GetAddressOf(), nullptr);//결과 UAV
	UINT groupSize = (UINT)ceil(1.0 / 32.0f);//그룹은 인스턴싱갯수로
	DC->Dispatch((UINT)instance.size(), 1, 1);

	DC->CSSetShaderResources(0, 1, emptySRV.GetAddressOf());
	DC->CSSetShaderResources(1, 1, emptySRV.GetAddressOf());
	DC->CSSetShaderResources(2, 1, emptySRV.GetAddressOf());
	ComPtr<ID3D11UnorderedAccessView> emptyUAV = nullptr;
	Path::SetDebugName(emptyUAV.Get(), "emptyUAV");
	DC->CSSetUnorderedAccessViews(0, 1, emptyUAV.GetAddressOf(), nullptr);
	DC->CSSetUnorderedAccessViews(1, 1, emptyUAV.GetAddressOf(), nullptr);
	DC->CSSetShader(nullptr, nullptr, 0);
}



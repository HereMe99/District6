#include "Math.hlsli"

struct HierarchyDATA
{
	float4 preQuat;
	matrix local;
	matrix world;
	matrix offset;
	int parentID;
	uint enable;
};
StructuredBuffer<HierarchyDATA> hierarchy : register(t0);

struct ANIMATION
{
	int clip;
	float curtime;
	float duration;
	float blendLoop;
};

struct INSTANCEANIMATION
{
	ANIMATION cur;
	ANIMATION next;

	float blendFactor;
};

StructuredBuffer<INSTANCEANIMATION> instanceAnim : register(t1);//인스턴싱애니메이션

Texture2DArray animData : register(t2);//애니메이션 데이터

RWTexture2D<float4> hierarchyMat : register(u0);
RWTexture2DArray<float4> output : register(u1);

void LoadkeyFrameData(uint id, ANIMATION clip, out float3 trans, out float4 quat, out float3 scale)
{
	float nextframe = fmod(clip.curtime + 1, clip.duration);
	float factor = frac(clip.curtime);
	trans = float3(animData.Load(int4(id * 3 + 0, floor(clip.curtime), clip.clip, 0)).xyz);
	quat = float4(animData.Load(int4(id * 3 + 1, floor(clip.curtime), clip.clip, 0)));
	scale = float3(animData.Load(int4(id * 3 + 2, floor(clip.curtime), clip.clip, 0)).xyz);
	trans = lerp(trans, animData.Load(int4(id * 3 + 0, floor(nextframe), clip.clip, 0)).xyz, factor);
	quat = slerp(quat, float4(animData.Load(int4(id * 3 + 1, floor(nextframe), clip.clip, 0))), factor);
	scale = lerp(scale, animData.Load(int4(id * 3 + 2, floor(nextframe), clip.clip, 0)).xyz, factor);

	[flatten]
	if (clip.duration - clip.curtime < clip.blendLoop)
	{
		factor = (clip.duration - clip.curtime) / clip.blendLoop;
		trans = lerp(animData.Load(int4(id * 3 + 0, 0, clip.clip, 0)).xyz, trans, factor);
		quat = slerp(float4(animData.Load(int4(id * 3 + 1, 0, clip.clip, 0))), quat, factor);
		scale = lerp(animData.Load(int4(id * 3 + 2, 0, clip.clip, 0)).xyz, scale, factor);
	}
}

void SaveHierarchyMatrix(uint id, uint gid, float4x4 inputMatrix, uint keyType)
{
	hierarchyMat[uint2(id * 4 + 0, gid)] = float4(inputMatrix._11_12_13, float(keyType));
	hierarchyMat[uint2(id * 4 + 1, gid)] = inputMatrix._21_22_23_24;
	hierarchyMat[uint2(id * 4 + 2, gid)] = inputMatrix._31_32_33_34;
	hierarchyMat[uint2(id * 4 + 3, gid)] = inputMatrix._41_42_43_44;
}

uint LoadkeyType(uint id, uint gid)
{
	return uint(hierarchyMat.Load(int3(id * 4 + 0, gid, 0)).w);
}

float4x4 LoadHierarchyMatrix(uint id, uint gid)
{
	return float4x4(float4(hierarchyMat.Load(int3(id * 4 + 0, gid, 0)).xyz, 0),
					hierarchyMat.Load(int3(id * 4 + 1, gid, 0)),
					hierarchyMat.Load(int3(id * 4 + 2, gid, 0)),
					hierarchyMat.Load(int3(id * 4 + 3, gid, 0)));
}

void WriteRWMatrix(uint id, uint gid, float4x4 inputMatrix)
{
	output[uint3(id * 4 + 0, 0, gid)] = inputMatrix._11_12_13_14;
	output[uint3(id * 4 + 1, 0, gid)] = inputMatrix._21_22_23_24;
	output[uint3(id * 4 + 2, 0, gid)] = inputMatrix._31_32_33_34;
	output[uint3(id * 4 + 3, 0, gid)] = inputMatrix._41_42_43_44;
}

[numthreads(32, 32, 1)]
void main(uint ID : SV_GroupIndex, uint3 GID : SV_GroupID)//GID 인스턴싱 인덱스
{
	matrix curmatrix = 0;
	uint keyType = 0;

	if (hierarchy[ID].enable > 0)
	{
		keyType = uint(animData.Load(int4(ID * 3, 0, instanceAnim[GID.x].cur.clip, 0)).w);
		//키타입을 next클립과 비교중 큰쪽변환 키타입 0은 계산없이 RW메트릭스 입력 그대로 씀. 키타입 1은 로컬월드입력. 키타입2는 애니메이션 매트릭스 변환
		[flatten]
		if (instanceAnim[GID.x].cur.clip != instanceAnim[GID.x].next.clip && instanceAnim[GID.x].blendFactor > 0.0F)
			keyType = max(keyType, uint(animData.Load(int4(ID * 3, 0, instanceAnim[GID.x].next.clip, 0)).w));

		[flatten]
		if (keyType > 1)
		{
			float3 trans;
			float4 quat;
			float3 scale;
			LoadkeyFrameData(ID, instanceAnim[GID.x].cur, trans, quat, scale);
			
			//넥스트 타임과의 보간
			[flatten]
			if (instanceAnim[GID.x].cur.clip != instanceAnim[GID.x].next.clip && instanceAnim[GID.x].blendFactor > 0.0F)
			{
				float3 nexttrans;
				float4 nextquat;
				float3 nextscale;
				LoadkeyFrameData(ID, instanceAnim[GID.x].next, nexttrans, nextquat, nextscale);
				trans = lerp(trans, nexttrans, instanceAnim[GID.x].blendFactor);
				quat = slerp(quat, nextquat, instanceAnim[GID.x].blendFactor);
				scale = lerp(scale, nextscale, instanceAnim[GID.x].blendFactor);
			}

			quat = mulQuat(quat, hierarchy[ID].preQuat);

			curmatrix = calcMatrix(trans, quat, scale);
	
		}
		else
		{
			[flatten]
			if (keyType == 1)//키타입이 1이면 로컬을 입력받음
				curmatrix = hierarchy[ID].local;
		}

		SaveHierarchyMatrix(ID, GID.x, curmatrix, keyType);
	}
	
	GroupMemoryBarrier();
	
	[flatten]
	if (hierarchy[ID].enable == 0 || keyType == 0)// || keyType == 0
		return;
	
	int parentID = hierarchy[ID].parentID;
	uint parentKeyType = LoadkeyType(parentID, GID.x);

	while (parentID > -1)
	{
		[flatten]
		if (parentKeyType == 0)//페어런트키타입이 0이되면 월드를 곱해서 루프문을 빠져나감
		{
			curmatrix = mul(curmatrix, hierarchy[parentID].world);
			break;
		}
		curmatrix = mul(curmatrix, LoadHierarchyMatrix(parentID, GID.x));
		parentID = hierarchy[parentID].parentID;
		parentKeyType = LoadkeyType(parentID, GID.x);
	}

	//offset메트릭스 계산. 본이 아닐경우 offset매트릭스는 단위 행렬이기에 변화 없음
	curmatrix = mul(hierarchy[ID].offset, curmatrix);
	
	WriteRWMatrix(ID, GID.x, curmatrix);//2DArray에 Array는 인스턴싱 인덱스와 동일. 2D의 X는 하이어라키와 동일 Y는 모션블러를 위해 남겨둠. 모션블러는 메트릭스 기록전에 Y=0의 값을 순차적으로 밑으로 기록함
	//ex)		1frame				2frame				3frame				4frame				5frame				6frame.......
	//y=0		1frameMatrix		2frameMatrix		3frameMatrix		4frameMatrix		5frameMatrix		6frameMatrix.....
	//y=1		identitymatrix		1frameMatrix		2frameMatrix		3frameMatrix		4frameMatrix		5frameMatrix.....
	//y=2		identitymatrix		identitymatrix		1frameMatrix		2frameMatrix		3frameMatrix		4frameMatrix.....
	//y=3		identitymatrix		identitymatrix		identitymatrix		1frameMatrix		2frameMatrix		3frameMatrix.....
	//y=4		identitymatrix		identitymatrix		identitymatrix		identitymatrix		1frameMatrix		2frameMatrix.....
	//y=5		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix		1frameMatrix.....
	//y=6		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix.....
	//y=7		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix.....
	//y=8		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix.....
	//y=9		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix		identitymatrix.....
	//10프레임까지 기록. 일단 구상은 여기까지. 문제가 될 부분 메트릭스 계산은 안해도 되지만 버텍스 셰이더에서 계산할 것이 많아짐.
	//생각한 차선책 - 버텍스 셰이더 자체에서 현재 버텍스 움직임을 RW에 현재 프레임 단위로 기록 위와 같이 새로움 프레임에 도래할 경우 한장씩 밀어 넣어 기록
	//픽셀셰이더에서 한장씩 밀어넣은 기록에 따라 해당 버텍스 위치와의 이동 거리를 프레임 단위로 벡터맵생성
	//포스트 랜더에서 이미지 전체에 대해 벡터맵을 적용하여 이동된 프레임만큼 모션블러 적용
	//차선책으로 할경우 컴퓨트 셰이더 output맵은 2Darray일 필요가 없어짐. 1DArray나 2D로 사용 가능(x = 하이어라키노드 갯수, y = instance). 대신 버텍스 셰이더에서 2DArray사용해야함(x = 버텍스,y = 프레임, z = instance)
	//단일 프레임으로 앞프레임과의 이동만 계산할경우. 1DArray나 2D로 사용 가능(x = 버텍스, y = instance). 부자연스러운 모션블러
}
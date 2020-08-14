#include "Header.hlsli"

cbuffer Mapping : register(b10)
{
    int isSpecularMap;
    int isNormalMap;
    int isAmbientMap;
}

cbuffer meshIDbuffer : register(b11)
{
    uint meshID;
}

Texture1D transMap : register(t6);
Texture2DArray animationMap : register(t7);

struct VertexInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    uint4 boneid : BLENDINDICES;
    float4 weights : BLENDWEIGHTS;
	
    float4 attr : InstanceAttr;
    uint instanceID : SV_InstanceID;
};

struct PixelInput
{
    float4 position : SV_Position;
    float2 uv : UV;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 normal : NORMAL;
    float3 viewDir : VIEWDIR;
    
    float3 wPosition : POSITION0;
    float3 cPosition : POSITION1;
};

float4x4 hierarchyMatrix(uint id, uint frame, uint instanceID)
{
    return matrix(animationMap.Load(int4(id * 4 + 0, frame, instanceID, 0)),
	animationMap.Load(int4(id * 4 + 1, frame, instanceID, 0)),
	animationMap.Load(int4(id * 4 + 2, frame, instanceID, 0)),
	animationMap.Load(int4(id * 4 + 3, frame, instanceID, 0)));
}


float4x4 transformMatrix(uint instanceID)
{
    return matrix(transMap.Load(int2(instanceID * 4 + 0, 0)),
	transMap.Load(int2(instanceID * 4 + 1, 0)),
	transMap.Load(int2(instanceID * 4 + 2, 0)),
	transMap.Load(int2(instanceID * 4 + 3, 0)));
}
PixelInput VS(VertexInput input)
{
    PixelInput output;

    float4 Pos = input.position;
    float3 Normal = input.normal;
    float3 Tangent = input.tangent;
	
    matrix mesh_world = hierarchyMatrix(meshID, 0, input.instanceID);
    Pos = mul(Pos, mesh_world);
   
    float3x3 mesh_normalSkinWorld = float3x3(normalize(mesh_world._11_12_13), normalize(mesh_world._21_22_23), normalize(mesh_world._31_32_33));
    Normal = mul(Normal, mesh_normalSkinWorld);
    Tangent = mul(Tangent, mesh_normalSkinWorld);
	
	[flatten]
    if (input.weights.x > 0.0f)
    {
        float LastWeight = 1.0f - (input.weights.x + input.weights.y + input.weights.z);
		
        matrix skinWorld = hierarchyMatrix(input.boneid.x, 0, input.instanceID) * input.weights.x
						 + hierarchyMatrix(input.boneid.y, 0, input.instanceID) * input.weights.y
						 + hierarchyMatrix(input.boneid.z, 0, input.instanceID) * input.weights.z
						 + hierarchyMatrix(input.boneid.w, 0, input.instanceID) * LastWeight;
		
        Pos = mul(Pos, skinWorld);
		
        float3x3 normalSkinWorld = float3x3(normalize(skinWorld._11_12_13), normalize(skinWorld._21_22_23), normalize(skinWorld._31_32_33));
        Normal = mul(Normal, normalSkinWorld);
        Tangent = mul(Tangent, normalSkinWorld);
    }
	
    matrix world = transformMatrix(input.instanceID);
    Pos = mul(Pos, world);
    output.wPosition = Pos;

    output.viewDir = normalize(Pos.xyz - invView._41_42_43);
	
    float3x3 normalWorld = float3x3(normalize(world._11_12_13), normalize(world._21_22_23), normalize(world._31_32_33));
    output.position = mul(Pos, view);
    output.position = mul(output.position, projection);
    
    output.uv = input.uv;
    
    output.normal = normalize(mul(Normal, normalWorld));
    output.tangent = normalize(mul(Tangent, normalWorld));
    output.binormal = normalize(cross(Normal, Tangent));
    output.cPosition = CamPos();
    
    return output;
}

float4 PS(PixelInput input) : SV_TARGET
{
   
    float4 albedo = diffuseMap.Sample(linearSamp, input.uv);
    float3 light = normalize(lightDir);
    
    float3 T = normalize(input.tangent);
    float3 B = normalize(input.binormal);
    float3 N = normalize(input.normal);
    
    float3 normal = N;
    
   if (isNormalMap)
    {
        float4 normalMapping = normalMap.Sample(linearSamp, input.uv);
    
        float3x3 TBN = float3x3(T, B, N);
    
        normal = normalMapping.xyz * 2.0f - 1.0f;
        normal = normalize(mul(normal, TBN));
    }
    
    float diffuseIntensity = saturate(dot(normal, -light));
    float4 diffuse = albedo * diffuseIntensity * lightPower;
    
    
    float4 specular = 0;
    if (diffuseIntensity > 0)
    {
        float3 halfWay = normalize(input.viewDir + light);
        specular = saturate(dot(-halfWay, normal));
        
        float4 specularIntensity = 0.0f;
        if (isSpecularMap)
        {
            specularIntensity = specularMap.Sample(linearSamp, input.uv).r;
        }
        
        
        specular = pow(specular, lightSpecExp) * specularIntensity;
    }
    
   // float4 ambient = CalcAmbient(normal, albedo) * ambientMap.Sample(linearSamp, input.uv);
    float4 ambient = CalcAmbient(normal, albedo);
    if (isAmbientMap)
    {
        ambient *= ambientMap.Sample(linearSamp, input.uv).r;
    }
  
    
    
    float4 result = float4((diffuse * mDiffuse + specular * mSpecular + ambient * mAmbient).xyz, 1.0f);
    
    
    for (uint i = 0; i < lightCount; i++)
    {
        [flatten]
        if (lights[i].type == 0)
        {
            result += CalcDirection(normal, albedo, input.wPosition, input.cPosition, lights[i], isSpecularMap);
        }
        else if (lights[i].type == 1)
        {
            result += CalcPoint(normal, albedo, input.wPosition, input.cPosition, lights[i], isSpecularMap);
        }
        else if (lights[i].type == 2)
        {
            result += CalcSpot(normal, albedo, input.wPosition, input.cPosition, lights[i], isSpecularMap);
        }
        else if (lights[i].type == 3)
        {
            result += CalcCapsule(normal, albedo, input.wPosition, input.cPosition, lights[i], isSpecularMap);
        }
    }
    
    return result;
}

float4 PS_Depth(PixelInput input) : SV_TARGET
{
    float depth = input.position.z / input.position.w;
    
    return float4(depth.xxx, 1.0f);
}
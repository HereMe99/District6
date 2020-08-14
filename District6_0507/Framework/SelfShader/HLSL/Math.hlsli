float4 slerp(float4 Q1, float4 Q2, float t)
{
	[flatten]
    if (length(Q1) == 0.0)
    {
		[flatten]
        if (length(Q2) == 0.0)
            return float4(0, 0, 0, 1);

        return Q2;
    }
    else
    {
		[flatten]
        if (length(Q2) == 0.0)
            return Q1;
    }

    float cosHalfAngle = Q1.w * Q2.w + dot(Q1.xyz, Q2.xyz);

	[flatten]
    if (cosHalfAngle >= 1.0 || cosHalfAngle <= -1.0)
        return Q1;
    else if (cosHalfAngle < 0.0)
    {
        Q2.xyz = -Q2.xyz;
        Q2.w = -Q2.w;
        cosHalfAngle = -cosHalfAngle;
    }

    float blendA;
    float blendB;

	[flatten]
    if (cosHalfAngle < 0.99)
    {
        float halfAngle = acos(cosHalfAngle);
        float sinHalfAngle = sin(halfAngle);
        float oneOverSinHalfAngle = 1.0 / sinHalfAngle;
        blendA = sin(halfAngle * (1.0 - t)) * oneOverSinHalfAngle;
        blendB = sin(halfAngle * t) * oneOverSinHalfAngle;
    }
    else
    {
        blendA = 1.0 - t;
        blendB = t;
    }

    float4 result = float4(blendA * Q1.xyz + blendB * Q2.xyz, blendA * Q1.w + blendB * Q2.w);

	[flatten]
    if (length(result) > 0.0)
    {
        return normalize(result);
    }
    return float4(0, 0, 0, 1);
}

float4 mulQuat(float4 Q1, float4 Q2)
{
    return float4(
		(Q2.w * Q1.x) + (Q2.x * Q1.w) + (Q2.y * Q1.z) - (Q2.z * Q1.y),
       (Q2.w * Q1.y) - (Q2.x * Q1.z) + (Q2.y * Q1.w) + (Q2.z * Q1.x),
       (Q2.w * Q1.z) + (Q2.x * Q1.y) - (Q2.y * Q1.x) + (Q2.z * Q1.w),
       (Q2.w * Q1.w) - (Q2.x * Q1.x) - (Q2.y * Q1.y) - (Q2.z * Q1.z));
}

float4x4 calcMatrix(float3 trans, float4 quat, float3 scale)
{
    float4x4 outMatrix;
	
    float sqw = quat.w * quat.w;
    float sqx = quat.x * quat.x;
    float sqy = quat.y * quat.y;
    float sqz = quat.z * quat.z;
    outMatrix._11 = (1.0f - 2.0f * sqy - 2.0f * sqz) * scale.x;
    outMatrix._12 = 2.0f * quat.x * quat.y + 2.0f * quat.z * quat.w;
    outMatrix._13 = 2.0f * quat.x * quat.z - 2.0f * quat.y * quat.w;
    outMatrix._14 = 0.0f;
    outMatrix._21 = 2.0f * quat.x * quat.y - 2.0f * quat.z * quat.w;
    outMatrix._22 = (1.0f - 2.0f * sqx - 2.0f * sqz) * scale.y;
    outMatrix._23 = 2.0f * quat.y * quat.z + 2.0f * quat.x * quat.w;
    outMatrix._24 = 0.0f;
    outMatrix._31 = 2.0f * quat.x * quat.z + 2.0f * quat.y * quat.w;
    outMatrix._32 = 2.0f * quat.y * quat.z - 2.0f * quat.x * quat.w;
    outMatrix._33 = (1.0f - 2.0f * sqx - 2.0f * sqy) * scale.z;
    outMatrix._34 = 0.0f;
    outMatrix._41 = trans.x;
    outMatrix._42 = trans.y;
    outMatrix._43 = trans.z;
    outMatrix._44 = 1.0f;
	
    return outMatrix;
}
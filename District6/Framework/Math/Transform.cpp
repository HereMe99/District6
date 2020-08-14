#include "Framework.h"

Transform::Transform(string tag)
	: tag(tag), position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1),
	 scalePivot(0,0,0),rotPivot(0, 0, 0), parent(nullptr)
{
	world = XMMatrixIdentity();
	worldBuffer = NEW WorldBuffer();
}

Transform::~Transform()
{
	delete worldBuffer;
}

void Transform::UpdateWorld()
{
	world = XMMatrixTransformation(scalePivot.data, XMQuaternionIdentity(), scale.data, rotPivot.data,
		XMQuaternionRotationRollPitchYaw(rotation.GetX(), rotation.GetY(), rotation.GetZ()),
		position.data);

	if (parent != nullptr)
		world *= *parent;

	worldBuffer->SetWorld(world);
}

Vector3 Transform::GetRight()
{		
	return XMVector3Normalize(XMVector3TransformNormal(kRight, world));
}

Vector3 Transform::GetUp()
{
	return XMVector3Normalize(XMVector3TransformNormal(kUp, world));
}

Vector3 Transform::GetForward()
{
	return XMVector3Normalize(XMVector3TransformNormal(kForward, world));
}

void Transform::SetRotPivot(float x, float y, float z)
{
	rotPivot = Vector3(x, y, z);
}

void Transform::SetScalePivot(float x, float y, float z)
{
	scalePivot = Vector3(x, y, z);
}

Vector3 Transform::GetWorldPosition()
{
	return XMVector3Transform(XMVectorZero(), world);
}

void Transform::SetTransform(Matrix matrix)
{
	Vector4 quadScale;
	
	XMMatrixDecompose(&scale.data, &rotation.data, &position.data, matrix);
	
}

void Transform::SetWorldBuffer(UINT slot)
{
	worldBuffer->SetVSBuffer(slot);
}


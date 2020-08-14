#pragma once

class Camera
{
public:
	Vector3 position;
	Vector3 rotation;

protected:
	Vector3 forward;
	Vector3 right;
	Vector3 up;

	Matrix matRotation;
	Matrix matView;

	Vector3 oldPos;
	bool isControl;

public:
	Camera();

	virtual void Update();
	virtual void Move();
	virtual void Rotation();
	virtual void View();

	virtual void PostRender();
	virtual void SetTarget(Transform* value) {}
	virtual void SetOffset(Vector3 offset) {}
	virtual void SetHeight(float value) {}
	bool GetIsControl() { return isControl; }
	Vector3 GetForward() { return forward; }
	Vector3 GetRight() { return right; }
	Vector3 GetUp() { return up; }

	Ray GetRay();

	Matrix GetView() { return matView; }
};
#pragma once
class FPSCam : public Camera
{
private:
	float moveSpeed;
	float rotSpeed;
	Transform* target;
	Vector3 offset;
	float height;

	POINT m_pt;

public:
	FPSCam();

	void Update() override;
	void MouseControl();

	inline void SetTarget(Transform* target) override;
	inline void SetHeight(float value) override;
	inline void SetOffset(Vector3 offset) override;
};

inline void FPSCam::SetTarget(Transform* target)
{
	this->target = target;
}

inline void FPSCam::SetHeight(float value)
{
	height = value;
}

inline void FPSCam::SetOffset(Vector3 offset)
{
	this->offset = offset;
}

#include "Framework.h"
#include "FPSCam.h"



FPSCam::FPSCam()
	: moveSpeed(20), rotSpeed(0.005f), target(nullptr), offset(0, 0, 0)
{
	isControl = true;
	ShowCursor(false);
}

void FPSCam::Update()
{
	if (target == nullptr) 
	{
		return;
	}

	position = target->position;
	position.SetY(position.GetY() + height);
	{//Rotation
		Vector3 value = MOUSEPOS - oldPos;
		oldPos = MOUSEPOS;
	}
	if(KEY_DOWN(VK_F2))
	{
		isControl = !isControl;
		ShowCursor(!isControl);
	}
	if (isControl)
	{
		MouseControl();
		Rotation();
	}
	position += target->GetForward() * offset.z;
	position += target->GetRight() * offset.x;
	View();
}
void FPSCam::MouseControl()
{
	RECT temp;
	const int frame_x = GetSystemMetrics(SM_CXFRAME);
	const int caption_y = GetSystemMetrics(SM_CYCAPTION);
	
	GetWindowRect(hWnd, &temp);
	m_pt.x = CENTER_X + temp.left+frame_x*0.5f;
	m_pt.y = CENTER_Y + temp.top+caption_y*0.5f;
	POINT pt;
	GetCursorPos(&pt);

	//rotY += (pt.x - m_pt.x) * 0.01f;
	rotation.SetX(rotation.GetX() + (pt.y - m_pt.y) * 0.005f);
	if (rotation.GetX() >= 3.141592f * 0.5f)
	{
		rotation.SetX(3.141592f * 0.5f);
	}
	else if (rotation.GetX() <= -3.141592f * 0.4f)
	{
		rotation.SetX(-3.141592f * 0.4f);
	}
	rotation.SetY(rotation.GetY() + (pt.x - m_pt.x) * 0.005f);
	SetCursorPos(m_pt.x, m_pt.y);
	target->rotation.SetY(3.141592f + rotation.GetY());
	//target->rotation.SetY(rotation.GetY());
	target->rotation.SetX(-rotation.GetX());
	target->UpdateWorld();
	//Keyboard::Get()->SetMouse(CENTER_X, CENTER_Y);
	//distance -= Keyboard::Get()->GetWheel() * zoomSpeed;
	//height -= Keyboard::Get()->GetWheel() * zoomSpeed;

	/*if (distance < 1.0f)
		distance = 1.0f;

	if (height < 1.0f)
		height = 1.0f;*/
}

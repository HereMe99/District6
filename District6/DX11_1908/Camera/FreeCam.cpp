#include "Framework.h"

FreeCam::FreeCam()
	: moveSpeed(20), rotSpeed(0.005f)
{	
}

void FreeCam::Update()
{
	{//Move
		if (KEY_PRESS(VK_RBUTTON))
		{
			if (KEY_PRESS('W'))
				position += forward * moveSpeed * DELTA;
			if (KEY_PRESS('S'))
				position -= forward * moveSpeed * DELTA;
			if (KEY_PRESS('A'))
				position -= right * moveSpeed * DELTA;
			if (KEY_PRESS('D'))
				position += right * moveSpeed * DELTA;
			if (KEY_PRESS('Q'))
				position -= up * moveSpeed * DELTA;
			if (KEY_PRESS('E'))
				position += up * moveSpeed * DELTA;
		}

		position += forward * Keyboard::Get()->GetWheel() * moveSpeed * DELTA;
	}

	{//Rotation
		Vector3 value = MOUSEPOS - oldPos;

		if (KEY_PRESS(VK_RBUTTON))
		{
			rotation.SetX(rotation.GetX() + value.GetY() * rotSpeed);
			rotation.SetY(rotation.GetY() + value.GetX() * rotSpeed);

			Rotation();
		}

		oldPos = MOUSEPOS;
	}

	View();
}

#pragma once

struct Loop
{
	enum LoopState
	{
		NONE_ACTIVE,
		LOOP_FALSE,
		LOOP_TRUE
	};
	float limitTime;
	float curTime;

	bool isLimit;
	bool isActive;
	bool isLoop;

	Loop(float limitTime, bool isLoop)
	{
		this->limitTime = limitTime;
		curTime = 0.0f;

		this->isLoop = isLoop;
		isActive = false;
		loopFunc = nullptr;
		isLimit = false;
	}

	~Loop()
	{
	}

	function<void()> loopFunc;

	void Update()
	{
		if (!isActive)
		{
			return;
		}

		if (curTime < limitTime)
		{
			curTime += DELTA * 1.0f;
		}
		else
		{
			isLimit = true;
			curTime = 0.0f;

			if (!isLoop)
			{
				StopTimer();
			}
			
			if (loopFunc != nullptr)
			{
				//함수포인터 실행
				loopFunc();
			}
		}


	}

	void StartLoop()
	{
		isActive = true;
		isLimit = false;
		curTime = 0.0f;
	}

	void StopTimer()
	{
		isActive = false;
		curTime = 0.0f;
	}

};

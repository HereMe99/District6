#pragma once

class UIManager
{
public:

	static UIManager* GetInstance()
	{
		if(instance == nullptr)
		{
			instance = NEW UIManager();
		}
	}

	void AddUITexture(Render2D texture);
	
private:
	static UIManager* instance;
	
	UIManager();
	~UIManager();
};

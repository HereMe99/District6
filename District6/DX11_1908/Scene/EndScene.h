#pragma once

class EndScene : public Scene
{
public:
	EndScene();
	~EndScene();
	void Init() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;

private:
	int select_menu_num;
	Render2D* bg;
	Render2D* menu_back[3];
	Render2D* menu[3];


	void GoTitle();
	void GoPlay();
};

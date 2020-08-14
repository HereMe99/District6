#pragma once

class TitleScene : public Scene
{
public:
	TitleScene();
	~TitleScene();
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


	void GoMapTool();
	void GoPlay();
};

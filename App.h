#pragma once
#include"Window.h"


class App{
public:
	App();

	//Master frame / Message Loop
	int Go();

private:
	void DoFrame();
private:
	Window wnd;
};


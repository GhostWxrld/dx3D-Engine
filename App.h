#pragma once
#include"Window.h"
#include "Timer.h"


class App{
public:
	App();

	//Master frame / Message Loop
	int Go();
	~App();

private:
	void DoFrame();
private:
	Window wnd;
	Timer timer;
	std::vector<std::unique_ptr<class Box>> boxes;
};


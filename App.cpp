#include "App.h"

App::App() :
	wnd(800, 800, L"Dx11 Engine")
{}

int App::Go() {
	while (true) {
		//process all messages pending, but to not block 
		if (const auto eCode = Window::ProcessMessages()) {
			//if return optional has value, means we're 
			return *eCode;
		}
		DoFrame();
	}
}

void App::DoFrame() {
	const float t = timer.Peek();
	std::ostringstream oss;
	oss << "Time Elapsed: " << std::setprecision(1) << std::fixed << t << "s";
	wnd.SetTitle(oss.str()); 
}
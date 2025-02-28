#include "App.h"

App::App() :
	wnd(800, 600, L"Dx11 Engine")
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
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;								//Sin wave color effect
	wnd.Gfx().ClearBuffer(c, c, 1.0f);												
	wnd.Gfx().DrawTestTriangle(
		timer.Peek(), 
		wnd.mouse.GetPosX() / 400.0f - 1.0f,
		-wnd.mouse.GetPosY() / 300.0f + 1.0f
	);

	wnd.Gfx().EndFrame();															//We are currently presenting a frame

}
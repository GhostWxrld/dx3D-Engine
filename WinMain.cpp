#include "Window.h"
#include "WindowsMessageMap.h" 
#include <sstream>

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR	  lpCmdLine,
	int		  nCmdShow
) {
	try {
		//L uses wide string literal
		Window wnd(1280, 800, L"D311 Game Engine");

		//Message pump
		MSG msg;
		BOOL gResult;
		while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//test code
			static int i = 0;
			while ( !wnd.mouse.IsEmpty()){
				const auto e = wnd.mouse.Read();
				switch (e.GetType()) {
				case Mouse::Event::Type::WheelUp:
					i++; {
						std::ostringstream oss;
						oss << "Up:" << i;
						wnd.SetTitle(oss.str());
					}
					break;
				}
			}
		}
		//Check if the GetMessage call itself borked
		if (gResult == -1) {
			return -1;
		}

		//wParam here is the value passed to PostQuitMessage
		return msg.wParam;
	}
	catch (const Exception& e) {
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch(const std::exception& e){
		MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...) {
		MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}
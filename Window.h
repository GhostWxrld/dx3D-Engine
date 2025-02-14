#pragma once
#include "ChiliWin.h"
#include "Exception.h"
#include "Keyboard.h"
#include "Mouse.h"
#include <optional>

class Window{
public:
	class Except : public Exception {
	public:
		Except(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;
		static std::string TranslateErrorCode(HRESULT hr) noexcept;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};
private:
	//singleton manages registration/cleanup of window class
	class WindowClass {
	public:
		static const wchar_t* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;

	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator = (const WindowClass&) = delete;
		static constexpr const wchar_t* wndClassName = L"Direct 3D Engine Window";
		static WindowClass wndClass;
		HINSTANCE hInst; 
	};
public:
	Window(int width, int height, const wchar_t* name);
	~Window();
	Window(const Window&) = delete;
	Window& operator = (const Window&) = delete;
	void SetTitle(const std::string& title);
	static std::optional<int> ProcessMessages();

private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

public:
	Keyboard kbd;
	Mouse mouse;

private:
	int width;
	int height; 
	HWND hWnd;

};

//error exception helping macro
#define CHWND_EXCEPT(hr) Window::Except(__LINE__, __FILE__, hr)
#define CHWND_LAST_EXCEPT() Window::Except(__LINE__, __FILE__, GetLastError()); 


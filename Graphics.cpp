#include "Graphics.h"
#include "dxerr.h"
#include <sstream>

#pragma comment(lib, "d3d11.lib")	//Will work on any machine so best way to link 

//graphics exception checking/throwing macros
#define GFX_THROW_FAILED(hrcall) if (FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__, __FILE__, hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemoveException(__LINE__, __FILE__,(hr))

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException(__LINE__, __FILE__, (hr), infoManager.GetMessages())
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if(FAILED(hr = (hrcall))) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__, __FILE__, (hr), infoManager.GetMessages())
#else
#define GFX_EXCEPT(hr) Graphics::HrException(__LINE__, __FILE__, (hr))
#define GFX_THROW_INFO(hrcall) GFX_THROW_INFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__, __FILE__, (hr))
#endif // !NDEBUG


Graphics::Graphics(HWND hWnd) {

	//Create Swap Chain Description
	DXGI_SWAP_CHAIN_DESC sd = {};													//Buffer
	sd.BufferDesc.Width = 0;														//Buffer Width
	sd.BufferDesc.Height = 0;														// Buffer Height
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;								//
	sd.BufferDesc.RefreshRate.Numerator = 0;										//Buffer Refresh Rate (Set to 0, so choose whatever is there)
	sd.BufferDesc.RefreshRate.Denominator = 0;										// ^
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;							//If its interlaced
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;														//Don't want AA right now so set to 1
	sd.SampleDesc.Quality = 0;														//Again don't want AA (Anti-Aliasing)
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;								//Buffer to be used as render target (Where the pipeline will render the stuff to)
	sd.BufferCount = 1;																//One Back Buffer and 1 front so set to 1
	sd.OutputWindow = hWnd;															//Window to output to
	sd.Windowed = TRUE;																//Yes we want it windowed
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;										//Will probably give best performance in most cases
	sd.Flags = 0;																	//No flags for now

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // !NDEBUG


	//for checking results of d3d functions 
	//Insert the hr variable for the macro since it needs to ensure that it exists in the local scope
	HRESULT hr;

	//Create device and front/back buffers, and swap chains and rendering content
	GFX_THROW_FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	));

	//Gain access to the sub resource in swap chain (back buffer)
	ID3D11Resource* pBackBuffer = nullptr;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer)));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(
		nullptr, 
		nullptr, 
		&pTarget
	));
	pBackBuffer->Release();
}

Graphics::~Graphics(){

	//Order doesn't matter but I like to keep it minor devices first before parent devices
	if (pTarget != nullptr) {
		pTarget->Release();
		pTarget = nullptr;
	}

	if (pContext != nullptr) {
		pContext->Release();
		pContext = nullptr;
	}

	if (pSwap != nullptr) {
		pSwap->Release();
		pSwap = nullptr;
	}

	if (pDevice != nullptr) {
		pDevice->Release();
		pDevice = nullptr;
	}
}

void Graphics::EndFrame(){

	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	if (FAILED(hr = pSwap->Present(1u, 0u))) {
		if (hr == DXGI_ERROR_DEVICE_REMOVED) {
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else {
			throw GFX_EXCEPT(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept {
	const float color[] = { red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTarget, color);
}

//Graphics exception stuff 
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept :
	Exception(line, file),
	hr(hr)
{
	//Join all info messages with newlines into single string
	for (const auto& m : infoMsgs) {
		info += m; 
		info.push_back('\n');
	}
	//Remove final newline if exists
	if (!info.empty()) {
		info.pop_back();
	}

}

const char* Graphics::HrException::what() const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty()) {
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept {
	return "Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept {
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept {
	return DXGetErrorStringA(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept {						//Make buffer for the error description
	char buf[512];
	DXGetErrorDescriptionA(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}

const char* Graphics::DeviceRemovedException::GetType() const noexcept {
	return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
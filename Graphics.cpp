#include "Graphics.h"

#pragma comment(lib, "d3d11.lib")	//Will work on any machine so best way to link 

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

	//Create device and front/back buffers, and swap chains and rendering content
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	);

	//Gain access to the sub resource in swap chain (back buffer)
	ID3D11Resource* pBackBuffer = nullptr;
	pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer));
	pDevice->CreateRenderTargetView(
		pBackBuffer, 
		nullptr, 
		&pTarget
	);
	pBackBuffer->Release();
}

Graphics::~Graphics(){

	//Order doesn't matter but I like to keep it minor devices first before parent devices
	if (pTarget != nullptr) {
		pTarget->Release();
	}

	if (pContext != nullptr) {
		pContext->Release();
	}

	if (pSwap != nullptr) {
		pSwap->Release();
	}

	if (pDevice != nullptr) {
		pDevice->Release();
	}
}

void Graphics::EndFrame(){
	pSwap->Present(1u, 0u);													//Hit the frame inter, no flags
}

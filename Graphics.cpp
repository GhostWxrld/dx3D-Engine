#include "Graphics.h"
#include "dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;


#pragma comment(lib, "d3d11.lib")	//Will work on any machine so best way to link 
#pragma comment(lib, "D3DCompiler.lib")

//graphics exception checking/throwing macros
#define GFX_THROW_FAILED(hrcall) if (FAILED(hr = (hrcall))) throw Graphics::HrException(__LINE__, __FILE__, hr)

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException(__LINE__, __FILE__, (hr), infoManager.GetMessages())
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if(FAILED(hr = (hrcall))) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException(__LINE__, __FILE__, (hr), infoManager.GetMessages())
#define GFX_THROW_INFO_ONLY(call) infoManager.Set(); (call); {auto v = infoManager.GetMessages(); if(!v.empty()) {throw Graphics::InfoException(__LINE__, __FILE__, v);}}
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
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr, 
		&pTarget
	));

	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	//Bind depth state
	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	//Create depth stencil texture
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 800u;
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_INFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	//Create view of depth stencil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV));
	
	// Bind depth stencil view to OM
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());
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
	pContext->ClearRenderTargetView(pTarget.Get(), color);
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::DrawTestTriangle( float angle, float x, float z ){

	HRESULT hr;

	struct Vertex {
		struct {
			float x;
			float y;
			float z;
		} pos;
	};

	//Create vertex buffer (1 2D triangle at center of screen)
	Vertex vertices[] = {
		//Form a Cube
		{	-1.0f, -1.0f, -1.0f},
		{	 1.0f, -1.0f, -1.0f},
		{	-1.0f,  1.0f, -1.0f},
		{	 1.0f,  1.0f, -1.0f},
		{	-1.0f, -1.0f,  1.0f},
		{	 1.0f, -1.0f,  1.0f},
		{	-1.0f,  1.0f,  1.0f},
		{	 1.0f,  1.0f,  1.0f},

	};

	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;

	GFX_THROW_INFO(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer)); 

	//Bind the vertex buffers
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

	//Create index buffer
	const unsigned short indices[] = {
		0, 2, 1,		2, 3, 1,
		1, 3, 5,		3, 7, 5,
		2, 6, 3,		3, 6, 7,
		4, 5, 7,		4, 7, 6,
		0, 4, 2,		2, 4, 6,
		0, 1, 4,		1, 5, 4
	};
	wrl::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	GFX_THROW_INFO(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	//Bind Index Buffer
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

	//Create constant buffer for transformation matrix
	struct ConstantBuffer {
		dx::XMMATRIX transform;									//Array of floats 4x4
	};

	const ConstantBuffer cb = {
		{
			dx::XMMatrixTranspose(
				dx::XMMatrixRotationZ(angle)*
				dx::XMMatrixRotationX(angle)*
				dx::XMMatrixTranslation(x, 0.0f, z + 4.0f) *
				dx::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f)
			)
		}
	};


	wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	//Bind constant buffer to vertex shader
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

	struct ConstantBuffer2 {
		struct {
			float r;
			float g;
			float b;
			float a;
		} face_colors[6];	
	};

	const ConstantBuffer2 cb2 = {
		{
			{ 1.0f, 0.0f, 1.0f},
			{ 1.0f, 0.0f, 0.0f},
			{ 0.0f, 1.0f, 0.0f},
			{ 0.0f, 0.0f, 1.0f},
			{ 1.0f, 1.0f, 0.0f},
			{ 0.0f, 1.0f, 1.0f},
		}
	};

	wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
	D3D11_BUFFER_DESC cbd2;
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.Usage = D3D11_USAGE_DEFAULT;
	cbd2.CPUAccessFlags = 0u;
	cbd2.MiscFlags = 0u;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd2 = {};
	csd2.pSysMem = &cb2;
	GFX_THROW_INFO(pDevice->CreateBuffer(&cbd2, &csd2, &pConstantBuffer2));

	//Bind Constant buffer to pixel shader
	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());
	
	//Create pixel shader
	wrl::ComPtr<ID3D11PixelShader> pPixelShader;
	wrl::ComPtr<ID3DBlob> pBlob;
	GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	//Bind pixel shader
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);

	//Create vertex shader
	wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	GFX_THROW_INFO(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	GFX_THROW_INFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

	//Bind vertex shader
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);

	//Input (vertex) layout(2D position only)
	wrl::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	GFX_THROW_INFO(pDevice->CreateInputLayout(
		ied, 
		(UINT)std::size(ied),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	));

	//Bind vertex layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//Set primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Configure Viewport
	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	GFX_THROW_INFO_ONLY(pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u)); 
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

Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string>& infoMsg) noexcept :
	Exception(line, file) {

	//Join all info messages with newlines into single string 
	for (const auto m : infoMsg) {
		info += m;
		info.push_back('\n');
	}

	//Remove final newline if exists
	if (!info.empty()) {
		info.pop_back();
	}
}

const char* Graphics::InfoException::what()const noexcept {
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept {
	return "Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept{
	return info;
}
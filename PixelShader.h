#pragma once
#include "Bindable.h"
#include "GraphicThrowMacros.h"



class PixelShader : public Bindable {
public:
	PixelShader(Graphics& gfx, const std::wstring& path);
	void Bind(Graphics& gfx) noexcept override;
protected:
	std::wstring path;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
};

 
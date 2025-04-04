#include "TransformCBuf.h"

TransformCBuf::TransformCBuf(Graphics& gfx, const Drawable& parent) : parent(parent) {
	if (!pVcbuf) {
		pVcbuf = std::make_unique<VertexConstantBuffer<DirectX::XMMATRIX>>(gfx);
	}
}

void TransformCBuf::Bind(Graphics& gfx) noexcept{
	pVcbuf->Update(gfx, DirectX::XMMatrixTranspose(
			parent.GetTransformXM() * gfx.GetProjection()					//Create final matrix used by Vertex Shader
		)
	);
	pVcbuf->Bind(gfx);
}

std::unique_ptr<VertexConstantBuffer<DirectX::XMMATRIX>>TransformCBuf::pVcbuf;


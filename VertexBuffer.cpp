#include "VertexBuffer.h"
#include <iostream>

void VertexBuffer::Bind(Graphics& gfx) noexcept{
	INFOMAN(gfx);

	UINT offset = 0;
	assert(pVertexBuffer && "Vertex buffer is NULL!");
	GFX_THROW_INFO_ONLY(GetContext(gfx)->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset));
	
}

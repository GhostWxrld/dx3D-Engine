#pragma once
#include "Drawable.h"
#include "IndexBuffer.h"
#include <cassert>
#include <typeinfo>

template<typename T>
class DrawableBase : public Drawable{
public:
	//Check to see if vec is empty
	bool IsStaticInitialized()  const noexcept {
		return !staticBinds.empty();
	}

	static void AddStaticBind(std::unique_ptr<Bindable> bind) noexcept(!IS_DEBUG) {
		assert("Must use Add IndexBuffer to bind index buffer" && typeid(*bind) != typeid(Bindable));
		staticBinds.push_back(std::move(bind));
	}

	void AddStaticIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept(!IS_DEBUG) {
		assert(pIndexBuffer == nullptr);
		pIndexBuffer = ibuf.get();
		staticBinds.push_back(std::move(ibuf));
	}

	void SetIndexFromStatic() noexcept(!IS_DEBUG) {
		assert("Trying to add index buffer twice!" && pIndexBuffer == nullptr);
		for (const auto& b : staticBinds) {
			if (const auto p = dynamic_cast<IndexBuffer*>(b.get())) {
				pIndexBuffer = p;
				return;
			}
		}
		assert("Failed to find index buffer in static binds" && pIndexBuffer != nullptr);
	}

protected:
	const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept {
		return staticBinds;
	}
private:
	static std::vector<std::unique_ptr<Bindable>> staticBinds;					//We assume at least 1 static bindable
};

template<class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticBinds;


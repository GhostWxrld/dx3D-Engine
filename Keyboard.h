#pragma once
#include <queue>
#include <bitset>

class Keyboard{
	friend class Window;
public:
	class Event {
	public:
		enum class Type {
			Press,
			Release,
			Invalid
		}; 
	private:
		Type type;
		unsigned char code;
	public:
		Event() :
			type(Type::Invalid),
			code(0u)
		{}

		Event(Type type, unsigned char code) noexcept :
			type(type),
			code(code)
		{}

		bool IsPress() const noexcept {
			return type == Type::Press;
		}

		bool IsRelease() const noexcept {
			return type == Type::Release;
		}

		bool IsValid() const noexcept {
			return type == Type::Invalid;
		}

		unsigned char GetCode() const noexcept {
			return code;
		}
	};

public:
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator = (const Keyboard&) = delete;

	//Key event stuff
	bool KeyIsPressed(unsigned char Keycode) const noexcept;
	Event ReadKey() noexcept;
	bool KeyIsEmpty() const noexcept;
	void FlushKey() noexcept;

	//Char event stuff
	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void FlushChar() noexcept;
	void Flush() noexcept;

	//Autorepeat Control
	void EnableAutoRepeat() noexcept;
	void DisableAutoRepeat() noexcept;
	bool AutoRepeatIsEnabled() const noexcept;

private:
	void OnKeyPressed(unsigned char Keycode) noexcept;
	void OnKeyRelease(unsigned char Keycode) noexcept;
	void OnChar(char Character) noexcept;
	void ClearState() noexcept;
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;

private:
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int bufferSize = 16u;
	bool autorepeatEnabled = false;
	std::bitset<nKeys> Keystates;
	std::queue<Event> Keybuffer;
	std::queue<char> Charbuffer;
};


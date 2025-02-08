#include "Keyboard.h"


bool Keyboard::KeyIsPressed(unsigned char Keycode) const noexcept{
	return Keystates[Keycode];
}

Keyboard::Event	Keyboard::ReadKey() noexcept {
	if (Keybuffer.size() > 0u) {
		Keyboard::Event e = Keybuffer.front();
		Keybuffer.pop();												//Dequeue 
		return e;
	}
	else {
		return Keyboard::Event();
	}
}

bool Keyboard::KeyIsEmpty() const noexcept{
	return Keybuffer.empty();
}

char Keyboard::ReadChar() noexcept {
	if (Charbuffer.size() > 0u) {
		unsigned char charcode = Charbuffer.front();
		Charbuffer.pop();												//Pull char off of queue
		return charcode;
	}
	else {
		return 0;
	}
}

bool Keyboard::CharIsEmpty() const noexcept {
	return Charbuffer.empty();
}

void Keyboard::FlushKey() noexcept {
	Keybuffer = std::queue<Event>();									//Default constructs a new Queue and replaces current one with new one
}

void Keyboard::FlushChar() noexcept {
	Charbuffer = std::queue<char>();									//Default constructs a new Char and replaces current one with new one
}

void Keyboard::Flush() noexcept {
	autorepeatEnabled = true;
}

void Keyboard::EnableAutoRepeat() noexcept {
	autorepeatEnabled = true;
}

void Keyboard::DisableAutoRepeat() noexcept {
	autorepeatEnabled = false;
}

bool Keyboard::AutoRepeatIsEnabled() const noexcept {
	return autorepeatEnabled;
}


//Private side of Class	- Triggers events when Key is press and gets the code
//Update both Key state and the Keybuffer
void Keyboard::OnKeyPressed(unsigned char Keycode) noexcept {
	Keystates[Keycode] = true;
	Keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, Keycode));
	TrimBuffer(Keybuffer);
}

void Keyboard::OnKeyRelease(unsigned char Keycode) noexcept{
	Keystates[Keycode] = false;
	Keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, Keycode));
	TrimBuffer(Keybuffer);
}

void Keyboard::OnChar(char character) noexcept {
	Charbuffer.push(character);
	TrimBuffer(Charbuffer);
}

void Keyboard::ClearState() noexcept {
	Keystates.reset();
}

//If size if greater than maximum allowed size then it pops out of the buffer
template<typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept {
	while (buffer.size() > bufferSize) {
		buffer.pop();
	}
}





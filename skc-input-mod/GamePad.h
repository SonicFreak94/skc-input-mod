#pragma once

#include <cstdint>

enum Buttons : uint8_t
{
	Button_None  = 0,
	Button_Up    = 0x1,
	Button_Down  = 0x2,
	Button_Left  = 0x4,
	Button_Right = 0x8,
	Button_B     = 0x10,
	Button_C     = 0x20,
	Button_A     = 0x40,
	Button_Start = 0x80,
};

class GamePad
{
	SDL_GameController* gamepad = nullptr;

	bool    connected_     = false;
	int     controller_id_ = -1;
	Buttons held_          = Button_None;
	Buttons pressed_       = Button_None;

public:
	GamePad();
	~GamePad();

	bool open(int id);
	void close();
	void poll();

	bool connected() const;
	int controller_id() const;
	Buttons held() const;
	Buttons pressed() const;
};

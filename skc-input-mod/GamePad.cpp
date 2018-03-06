#include "stdafx.h"
#include "GamePad.h"
#include "minmax.h"

static constexpr auto AXIS_THRESHOLD = 0.25f;

Buttons operator|(const Buttons& l, const Buttons& r)
{
	return static_cast<Buttons>(static_cast<uint8_t>(l) | static_cast<uint8_t>(r));
}

Buttons& operator|=(Buttons& l, const Buttons& r)
{
	l = l | r;
	return l;
}

GamePad::GamePad(GamePad&& other) noexcept
{
	move_from(std::move(other));
}

GamePad::~GamePad()
{
	close();
}

GamePad& GamePad::operator=(GamePad&& other) noexcept
{
	move_from(std::move(other));
	return *this;
}

bool GamePad::open(int id)
{
	if (connected_)
	{
		close();
	}

	gamepad = SDL_GameControllerOpen(id);

	if (gamepad == nullptr)
	{
		connected_ = false;
		return false;
	}

	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(gamepad);

	if (joystick == nullptr)
	{
		connected_ = false;
		return false;
	}

	controller_id_ = id;
	connected_     = true;
	return true;
}

void GamePad::close()
{
	if (!connected_)
	{
		return;
	}

	if (gamepad != nullptr)
	{
		SDL_GameControllerClose(gamepad);
		gamepad = nullptr;
	}

	controller_id_ = -1;
	connected_     = false;
	held_          = Button_None;
	pressed_       = Button_None;
}

void GamePad::poll()
{
	auto last_held = held_;

	held_    = Button_None;
	pressed_ = Button_None;

	if (!connected_)
	{
		return;
	}

	const auto left_x_i = clamp(SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX),
								static_cast<short>(-std::numeric_limits<short>::max()), std::numeric_limits<short>::max());
	const auto left_y_i = clamp(SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY),
								static_cast<short>(-std::numeric_limits<short>::max()), std::numeric_limits<short>::max());

	const auto left_x = left_x_i / static_cast<float>(std::numeric_limits<short>::max());
	const auto left_y = left_y_i / static_cast<float>(std::numeric_limits<short>::max());

	auto up    = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP) | (left_y < -AXIS_THRESHOLD);
	auto down  = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN) | (left_y > AXIS_THRESHOLD);
	auto left  = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT) | (left_x < -AXIS_THRESHOLD);
	auto right = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) | (left_x > AXIS_THRESHOLD);
	auto a     = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A) |
				 SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_Y);
	auto b     = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_B);
	auto x     = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_X);
	auto start = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_START);

	if (up)
	{
		held_ |= Button_Up;
	}

	if (down)
	{
		held_ |= Button_Down;
	}

	if (left)
	{
		held_ |= Button_Left;
	}

	if (right)
	{
		held_ |= Button_Right;
	}

	if (a)
	{
		held_ |= Button_C;
	}

	if (b)
	{
		held_ |= Button_B;
	}

	if (x)
	{
		held_ |= Button_A;
	}

	if (start)
	{
		held_ |= Button_Start;
	}

	pressed_ = static_cast<Buttons>(held_ & ~last_held);
}

bool GamePad::connected() const
{
	return connected_;
}

int GamePad::controller_id() const
{
	return controller_id_;
}

Buttons GamePad::held() const
{
	return held_;
}

Buttons GamePad::pressed() const
{
	return pressed_;
}

void GamePad::move_from(GamePad&& other)
{
	gamepad        = other.gamepad;
	controller_id_ = other.controller_id_;
	connected_     = other.connected_;
	held_          = other.held_;
	pressed_       = other.pressed_;

	other.gamepad        = nullptr;
	other.controller_id_ = -1;
	other.connected_     = false;
	other.held_          = Button_None;
	other.pressed_       = Button_None;
}

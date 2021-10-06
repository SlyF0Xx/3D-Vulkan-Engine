#pragma once
#include "System.h"

namespace diffusion {

class InputSystem :
    public System
{
public:
	InputSystem();

	void move_forward();
	void move_backward();
	void move_left();
	void move_right();
	void move_up();
	void move_down();
};

} // namespace diffusion {
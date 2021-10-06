#pragma once

#include "eventpp/callbacklist.h"

namespace diffusion {

eventpp::CallbackList<void()> move_forward;
eventpp::CallbackList<void()> move_backward;
eventpp::CallbackList<void()> move_left;
eventpp::CallbackList<void()> move_right;
eventpp::CallbackList<void()> move_up;
eventpp::CallbackList<void()> move_down;

}
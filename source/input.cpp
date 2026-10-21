#include "input.hpp"

#include <SDL3/SDL_keyboard.h>

namespace pong
{
    bool Input::down(Action action) const
    {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        
        switch (action)
        {
            case Action::Left: return keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
            case Action::Right: return keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
            case Action::Sprint: return keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
        }
        
        return false;
    }
}

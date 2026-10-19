/*
The zlib/libpng License

Copyright (c) 2018 Arthur Brainville
Copyright (c) 2015 Andrew Fenn
Copyright (c) 2005-2010 Phillip Castaneda (pjcast -- www.wreckedgames.com)

This software is provided 'as-is', without any express or implied warranty. In no
event will the authors be held liable for any damages arising from the use of this
software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to the
following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that
        you wrote the original software. If you use this software in a product,
        an acknowledgment in the product documentation would be appreciated
        but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/
#include "OISMouse.h"
#include <cstdlib>
#include <iostream>

namespace
{
int checks = 0;
int failures = 0;

void check(bool condition, const char* message)
{
    ++checks;
    if (!condition)
    {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}
}

int main()
{
    OIS::MouseState state;
    check(state.width == 50 && state.height == 50, "default dimensions");
    check(!state.buttonDown(OIS::MB_Left), "buttons start released");

    state.width = 1280;
    state.height = 720;
    state.X.abs = 211;
    state.X.rel = -7;
    state.Y.abs = 90;
    state.Y.rel = 4;
    state.Z.abs = 3;
    state.Z.rel = 1;
    state.buttons = (1 << OIS::MB_Left) | (1 << OIS::MB_Button3);
    check(state.buttonDown(OIS::MB_Left), "left button bit mapping");
    check(!state.buttonDown(OIS::MB_Right), "right button bit mapping");
    check(state.buttonDown(OIS::MB_Button3), "extended button bit mapping");

    const OIS::MouseState copy(state);
    check(copy.X.rel == -7 && copy.Y.rel == 4, "MouseState value copy preserves axis values");
    state.clear();
    check(copy.X.rel == -7 && copy.Y.rel == 4, "owned MouseState copy is unchanged when original clears");
    check(state.X.abs == 0 && state.X.rel == 0 && state.Y.abs == 0 && state.Y.rel == 0,
          "clear resets position and relative axes");
    check(state.Z.abs == 0 && state.Z.rel == 0 && state.buttons == 0,
          "clear resets wheel and buttons");
    check(state.width == 1280 && state.height == 720, "clear retains configured dimensions");

    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}

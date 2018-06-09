/*
The zlib/libpng License

Copyright (c) 2018 Osamu Takasugi

This software is provided 'as-is', without any express or implied warranty. In no event will
the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following
restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that
		you wrote the original software. If you use this software in a product,
		an acknowledgment in the product documentation would be appreciated but is
		not required.

    2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/
//////////////////////////////// OS Nuetral Headers ////////////////
#include "OISInputManager.h"
#include "OISException.h"
#include "OISKeyboard.h"
#include "OISMouse.h"
#include "OISJoyStick.h"
#include "OISEvents.h"

//Advanced Usage
#include "OISForceFeedback.h"

#include "OISFFI.h"

#include <sstream>
#include <algorithm>

using namespace OIS;

intptr_t OIS_CreateInputManager(intptr_t wndptr)
{
	ParamList pl;

	std::ostringstream wnd;
	wnd << wndptr;

	pl.insert(std::make_pair(std::string("WINDOW"), wnd.str()));

	return (intptr_t)InputManager::createInputSystem(pl);
}

void OIS_DestroyInputManager(intptr_t inputManager)
{
	if(inputManager) InputManager::destroyInputSystem((InputManager *)inputManager);
}

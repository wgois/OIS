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
	if(inputManager) InputManager::destroyInputSystem((InputManager*)inputManager);
}

int OIS_InputManager_GetNumberOfDevices(intptr_t inputManager, int iType)
{
	return ((InputManager*)inputManager)->getNumberOfDevices((Type)iType);
}

void OIS_InputManager_EnableAddOnFactory(intptr_t inputManager)
{
	((InputManager*)inputManager)->enableAddOnFactory(InputManager::AddOn_All);
}

intptr_t OIS_InputManager_CreateInputObject(intptr_t inputManager, int iType, bool buffered)
{
	return (intptr_t)((InputManager*)inputManager)->createInputObject((Type)iType, buffered);
}

void OIS_InputManager_DestroyInputObject(intptr_t inputManager, intptr_t inputObject)
{
	((InputManager*)inputManager)->destroyInputObject((Object*)inputObject);
}

bool OIS_InputObject_GetVenderString(intptr_t inputObject, char* buf, size_t bufsize)
{
	auto obj = (Object*)inputObject;
	if(obj->vendor().size() > bufsize) return false;
	for(int i = 0; i < obj->vendor().size(); ++i) {
		buf[i] = obj->vendor()[i];
	}
	return true;
}

bool OIS_InputObject_Buffered(intptr_t inputObject)
{
	return ((Object*)inputObject)->buffered();
}

void OIS_InputObject_SetBuffered(intptr_t inputObject, bool bufferd)
{
	((Object*)inputObject)->setBuffered(bufferd);
}

intptr_t OIS_InputObject_GetCreator(intptr_t inputObject)
{
	return (intptr_t)((Object*)inputObject)->getCreator();
}

void OIS_InputObject_Capture(intptr_t inputObject)
{
	((Object*)inputObject)->capture();
}

int OIS_InputObject_GetID(intptr_t inputObject)
{
	return ((Object*)inputObject)->getID();
}

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
#include "linux/LinuxInputManager.h"
#include "linux/LinuxKeyboard.h"
#include "OISException.h"
#include "OISEvents.h"

#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <cstring>

using namespace OIS;
#include <iostream>

//-------------------------------------------------------------------//
LinuxKeyboard::LinuxKeyboard(InputManager* creator, bool buffered, bool grab) :
 Keyboard(creator->inputSystemName(), buffered, 0, creator), xim(0), ximStyle(0), xic(0)
{
	display = 0;
	window  = 0;

	grabKeyboard = grab;
	keyFocusLost = false;

	static_cast<LinuxInputManager*>(mCreator)->_setKeyboardUsed(true);

	
	addKeyConversion(XK_Up, KC_UP);
	addKeyConversion(XK_Down, KC_DOWN);
	addKeyConversion(XK_Left, KC_LEFT);
	addKeyConversion(XK_Right, KC_RIGHT);

	addKeyConversion(XK_KP_Divide, KC_DIVIDE);

	addKeyConversion(XK_KP_Home, KC_NUMPAD7);
	addKeyConversion(XK_KP_Up, KC_NUMPAD8);
	addKeyConversion(XK_KP_Page_Up, KC_NUMPAD9);
	addKeyConversion(XK_KP_Left, KC_NUMPAD4);
	addKeyConversion(XK_KP_Begin, KC_NUMPAD5);
	addKeyConversion(XK_KP_Right, KC_NUMPAD6);
	addKeyConversion(XK_KP_End, KC_NUMPAD1);
	addKeyConversion(XK_KP_Down, KC_NUMPAD2);
	addKeyConversion(XK_KP_Page_Down, KC_NUMPAD3);
	addKeyConversion(XK_KP_Insert, KC_NUMPAD0);
	addKeyConversion(XK_KP_Delete, KC_DECIMAL);

	addKeyConversion(XK_Page_Up, KC_PGUP);
	addKeyConversion(XK_Page_Down, KC_PGDOWN);
	addKeyConversion(XK_Home, KC_HOME);
	addKeyConversion(XK_End, KC_END);

	addKeyConversion(XK_Print, KC_SYSRQ);
	addKeyConversion(XK_Scroll_Lock, KC_SCROLL);
	addKeyConversion(XK_Pause, KC_PAUSE);

	addKeyConversion(XK_Shift_R, KC_RSHIFT);
	addKeyConversion(XK_Shift_L, KC_LSHIFT);
	addKeyConversion(XK_Alt_R, KC_RMENU);
	addKeyConversion(XK_Alt_L, KC_LMENU);

	addKeyConversion(XK_Insert, KC_INSERT);
	addKeyConversion(XK_Delete, KC_DELETE);

	addKeyConversion(XK_Super_L, KC_LWIN);
	addKeyConversion(XK_Super_R, KC_RWIN);
	addKeyConversion(XK_Menu, KC_APPS);
}

void LinuxKeyboard::addKeyConversion(KeySym x_key, KeyCode ois_key)
{
	keyConversionToOIS.insert(XtoOIS_KeyMap::value_type(x_key, ois_key));
	keyConversionFromOIS.insert(OIStoX_KeyMap::value_type(ois_key, x_key));
}
//-------------------------------------------------------------------//
void LinuxKeyboard::_initialize()
{
	//Set the locale to (hopefully) the users LANG UTF-8 Env var
	if(setlocale(LC_ALL, "") == NULL)
		OIS_WARN(E_General, "LinuxKeyboard::_initialize: Failed to set default locale.");

	//Clear our keyboard state buffer
	memset(&KeyBuffer, 0, 256);
	mModifiers = 0;

	if(display) XCloseDisplay(display);
	display = 0;
	window  = static_cast<LinuxInputManager*>(mCreator)->_getWindow();

	//Create our local X mListener connection
	if(!(display = XOpenDisplay(0)))
		OIS_EXCEPT(E_General, "LinuxKeyboard::_initialize >> Error opening X!");

	// Get modifiers masks
	capsLockMask = XkbKeysymToModifiers(display, XK_Caps_Lock);
	numLockMask  = XkbKeysymToModifiers(display, XK_Num_Lock);

	//Configure locale modifiers
	if(XSetLocaleModifiers("@im=none") == NULL)
		OIS_WARN(E_General, "LinuxKeyboard::_initialize: Failed to configure locale modifiers.");

	//Open input method
	xim = XOpenIM(display, NULL, NULL, NULL);
	if(!xim)
	{
		OIS_WARN(E_General, "LinuxKeyboard::_initialize: Failed to open input method.");
	}
	else
	{
		XIMStyles* ximStyles;
		char* ret = XGetIMValues(xim, XNQueryInputStyle, &ximStyles, NULL);
		if(ret != NULL || ximStyles == NULL)
		{
			OIS_WARN(E_General, "LinuxKeyboard::_initialize: Input method does not support any styles.");
		}
		else
		{
			ximStyle = 0;
			for(int i = 0; i < ximStyles->count_styles; i++)
			{
				if(ximStyles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
				{
					ximStyle = ximStyles->supported_styles[i];
					break;
				}
			}

			if(ximStyle == 0)
				OIS_WARN(E_General, "LinuxKeyboard::_initialize: Input method does not support the requested style.");

			XFree(ximStyles);
		}
	}

	//Set it to receive Input events
	if(XSelectInput(display, window, KeyPressMask | KeyReleaseMask) == BadWindow)
		OIS_EXCEPT(E_General, "LinuxKeyboard::_initialize: X error!");

	if(xim && ximStyle)
	{
		xic = XCreateIC(xim,
						XNInputStyle,
						ximStyle,
						XNClientWindow,
						window,
						XNFocusWindow,
						window,
						NULL);
		if(!xic)
			OIS_WARN(E_General, "LinuxKeyboard::_initialize: Failed to create input context.");
	}

	if(grabKeyboard)
		XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);

	keyFocusLost = false;
}

//-------------------------------------------------------------------//
LinuxKeyboard::~LinuxKeyboard()
{
	if(display)
	{
		if(grabKeyboard)
			XUngrabKeyboard(display, CurrentTime);

		if(xic)
			XDestroyIC(xic);

		if(xim)
			XCloseIM(xim);
		XCloseDisplay(display);
	}

	static_cast<LinuxInputManager*>(mCreator)->_setKeyboardUsed(true);
}

//-------------------------------------------------------------------//
unsigned int UTF8ToUTF32(unsigned char* buf)
{
	unsigned char& FirstChar = buf[0];

	if(FirstChar < 128)
		return FirstChar;

	unsigned int val = 0;
	unsigned int len = 0;

	if((FirstChar & 0xE0) == 0xC0) //2 Chars
	{
		len = 2;
		val = FirstChar & 0x1F;
	}
	else if((FirstChar & 0xF0) == 0xE0) //3 Chars
	{
		len = 3;
		val = FirstChar & 0x0F;
	}
	else if((FirstChar & 0xF8) == 0xF0) //4 Chars
	{
		len = 4;
		val = FirstChar & 0x07;
	}
	else if((FirstChar & 0xFC) == 0xF8) //5 Chars
	{
		len = 5;
		val = FirstChar & 0x03;
	}
	else // if((FirstChar & 0xFE) == 0xFC) //6 Chars
	{
		len = 6;
		val = FirstChar & 0x01;
	}

	for(int i = 1; i < len; i++)
		val = (val << 6) | (buf[i] & 0x3F);

	return val;
}

//-------------------------------------------------------------------//
bool LinuxKeyboard::isKeyDown(KeyCode key) const
{
	return (KeyBuffer[key]);
}

//-------------------------------------------------------------------//
void LinuxKeyboard::capture()
{
	XEvent event;

	while(XPending(display) > 0)
	{
		XNextEvent(display, &event);

		if(KeyPress == event.type)
		{
			_handleKeyPress(event);
		}
		else if(KeyRelease == event.type)
		{
			_handleKeyRelease(event);
		}
	}

	//If grabbing mode is on.. Handle focus lost/gained via Alt-Tab and mouse clicks
	if(grabKeyboard)
	{
		LinuxInputManager* linMan = static_cast<LinuxInputManager*>(mCreator);

		if(linMan->_getGrabState() == false)
		{
			// are no longer grabbing
			if(keyFocusLost == false)
			{
				//UnGrab KeyBoard
				XUngrabKeyboard(display, CurrentTime);
				keyFocusLost = true;
			}
		}
		else
		{
			//We are grabbing - and regained focus
			if(keyFocusLost == true)
			{
				//ReGrab KeyBoard
				XGrabKeyboard(display, window, True, GrabModeAsync, GrabModeAsync, CurrentTime);
				keyFocusLost = false;
			}
		}
	}
}

//-------------------------------------------------------------------//
void LinuxKeyboard::_handleKeyPress(XEvent& event)
{
	XKeyEvent& e = (XKeyEvent&)event;
	static std::vector<char> buf(32);
	KeySym keySym;
	unsigned int character = 0;
	int bytes			   = 0;
	bool haveChar		   = true;

	haveChar = !XFilterEvent(&event, None);

	if(xic)
	{
		Status status;
		do
		{
			bytes	  = Xutf8LookupString(xic, &e, &buf[0], buf.size() - 1, &keySym, &status);
			buf[bytes] = '\0';

			if(status == XBufferOverflow)
				buf.resize(buf.size() * 2);
		} while(status == XBufferOverflow);
	}
	else
	{
		bytes	  = XLookupString(&e, &buf[0], buf.size() - 1, &keySym, NULL);
		buf[bytes] = '\0';
	}

	if(haveChar && bytes > 0)
	{
		if(mTextMode == Unicode)
			character = UTF8ToUTF32(reinterpret_cast<unsigned char*>(&buf[0]));
		else if(mTextMode == Ascii)
			character = buf[0];
	}

	if(e.state & capsLockMask)
		mModifiers |= CapsLock;
	else
		mModifiers &= ~CapsLock;

	if(e.state & numLockMask)
		mModifiers |= NumLock;
	else
		mModifiers &= ~NumLock;

	KeyCode kc = KeySymToOISKeyCode(keySym);
	_injectKeyDown(kc, character);

	//Check for Alt-Tab
	LinuxInputManager* linMan = static_cast<LinuxInputManager*>(mCreator);
	if((e.state & Mod1Mask) && (keySym == XK_Tab))
		linMan->_setGrabState(false);
}

void LinuxKeyboard::_handleKeyRelease(XEvent& event)
{
	XKeyEvent& e = (XKeyEvent&)event;
	KeySym keySym;

	XFilterEvent(&event, None);
	if(!_isKeyRepeat(event))
	{
		XLookupString(&e, NULL, 0, &keySym, NULL);

		KeyCode kc = KeySymToOISKeyCode(keySym);
		_injectKeyUp(kc);
	}
}

//-------------------------------------------------------------------//
void LinuxKeyboard::setBuffered(bool buffered)
{
	mBuffered = buffered;
}

OIS::KeyCode LinuxKeyboard::convert(KeySym kc)
{
	const auto result = keyConversionToOIS.find(kc);
	if(result == keyConversionToOIS.end())
		return KC_UNASSIGNED; //No explicit conversion for the symbol;
	return result->second;
}
KeySym LinuxKeyboard::convert(OIS::KeyCode kc)
{
	const auto result = keyConversionFromOIS.find(kc);
	if(result == keyConversionFromOIS.end())
		return NoSymbol; //No explicit conversion for the symbol;
	return result->second;
}
//-------------------------------------------------------------------//
bool LinuxKeyboard::_injectKeyDown(KeyCode kc, int text)
{
	if(kc > 255) kc = KC_UNASSIGNED;
	KeyBuffer[kc] = 1;

	//Turn on modifier flags
	if(kc == KC_LCONTROL || kc == KC_RCONTROL)
		mModifiers |= Ctrl;
	else if(kc == KC_LSHIFT || kc == KC_RSHIFT)
		mModifiers |= Shift;
	else if(kc == KC_LMENU || kc == KC_RMENU)
		mModifiers |= Alt;

	if(mBuffered && mListener)
		return mListener->keyPressed(KeyEvent(this, kc, text));

	return true;
}

//-------------------------------------------------------------------//
bool LinuxKeyboard::_injectKeyUp(KeyCode kc)
{
	if(kc > 255) kc = KC_UNASSIGNED;
	KeyBuffer[kc] = 0;

	//Turn off modifier flags
	if(kc == KC_LCONTROL || kc == KC_RCONTROL)
		mModifiers &= ~Ctrl;
	else if(kc == KC_LSHIFT || kc == KC_RSHIFT)
		mModifiers &= ~Shift;
	else if(kc == KC_LMENU || kc == KC_RMENU)
		mModifiers &= ~Alt;

	if(mBuffered && mListener)
		return mListener->keyReleased(KeyEvent(this, kc, 0));

	return true;
}

//-------------------------------------------------------------------//
const std::string& LinuxKeyboard::getAsString(KeyCode kc)
{
	mGetString = "Unknown";
	char* temp = 0;
	KeySym sym = OISKeyCodeToKeySym(kc);
	if(sym != NoSymbol)
	{
		temp = XKeysymToString(sym);
		if(temp)
			mGetString = temp;
	}

	return mGetString;
}

//-------------------------------------------------------------------//
OIS::KeyCode LinuxKeyboard::getAsKeyCode(std::string str)
{
	OIS::KeyCode mGetKeyCode;
	/*
 * TODO fixme!
    KeySym X11Key = XStringToKeysym(str.c_str());
    mGetKeyCode = keyConversionToOIS.at(X11Key);
*/
	return mGetKeyCode;
}

//-------------------------------------------------------------------//
void LinuxKeyboard::copyKeyStates(char keys[256]) const
{
	memcpy(keys, KeyBuffer, 256);
}

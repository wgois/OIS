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
#ifndef _LINUX_KEYBOARD_H_EADER_
#define _LINUX_KEYBOARD_H_EADER_

#include "linux/LinuxPrereqs.h"
#include "OISKeyboard.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <unordered_map>

namespace OIS
{
	/** Linux implementation of Keyboard object - uses x11 */
	class LinuxKeyboard : public Keyboard
	{
	public:
		LinuxKeyboard(InputManager* creator, bool buffered, bool grab);
		virtual ~LinuxKeyboard();

		/** @copydoc Keyboard::isKeyDown */
		virtual bool isKeyDown(KeyCode key) const;

		/** @copydoc Keyboard::getAsString */
		virtual const std::string& getAsString(KeyCode kc);

		/** @copydoc Keyboard::getAsKeyCode */
		virtual OIS::KeyCode getAsKeyCode(std::string str);

		/** @copydoc Keyboard::copyKeyStates */
		virtual void copyKeyStates(char keys[256]) const;

		/** @copydoc Object::setBuffered */
		virtual void setBuffered(bool buffered);

		/** @copydoc Object::capture */
		virtual void capture();

		/** @copydoc Object::queryInterface */
		virtual Interface* queryInterface(Interface::IType) { return 0; }

		/** @copydoc Object::_initialize */
		virtual void _initialize();

	protected:
		inline bool _isKeyRepeat(XEvent& event)
		{
			//When a key is repeated, there will be two events: released, followed by another immediate pressed. So check to see if another pressed is present
			if(!XPending(display))
				return false;

			XEvent e;
			XPeekEvent(display, &e);
			if(e.type == KeyPress && e.xkey.keycode == event.xkey.keycode && (e.xkey.time - event.xkey.time) < 2)
			{
				XNextEvent(display, &e);
				XFilterEvent(&event, None);
				return true;
			}

			return false;
		}

		bool _injectKeyDown(KeyCode kc, int text);
		bool _injectKeyUp(KeyCode kc);
		void _handleKeyPress(XEvent& event);
		void _handleKeyRelease(XEvent& event);

		inline KeyCode KeySymToOISKeyCode(KeySym keySym)
		{
			if(keySym != NoSymbol)
			{
				//Check for explicit convert
				const auto result = keyConversionToOIS.find(keySym);
				if(result != keyConversionToOIS.end())
					return result->second;
				
				::KeyCode xkc = XKeysymToKeycode(display, keySym);
				if(xkc > 8)
					return static_cast<KeyCode>(xkc - 8);
			}
			return KC_UNASSIGNED;
		}

		inline KeySym OISKeyCodeToKeySym(KeyCode kc)
		{
			if(kc == KC_UNASSIGNED)
				return NoSymbol;
			
			//Check for explicit convert
			const auto result = keyConversionFromOIS.find(kc);
			if(result != keyConversionFromOIS.end())
				return result->second;
			
			::KeyCode xkc = kc + 8;
			return XkbKeycodeToKeysym(display, xkc, 0, 0);
		}

		//! Explict convertion for non-text symbols
		typedef std::unordered_map<KeySym, KeyCode> XtoOIS_KeyMap;
		XtoOIS_KeyMap keyConversionToOIS;
		
		typedef std::unordered_map<KeyCode, KeySym, std::hash<int>> OIStoX_KeyMap;
		OIStoX_KeyMap keyConversionFromOIS;
		
		void addKeyConversion(KeySym x_key, KeyCode ois_key);

		//! Depressed Key List
		char KeyBuffer[256];

		//! X11 Stuff
		Window window;
		Display* display;
		XIM xim;
		XIMStyle ximStyle;
		XIC xic;
		bool grabKeyboard;
		bool keyFocusLost;

		int capsLockMask;
		int numLockMask;

		std::string mGetString;
	};
}

#endif //_LINUX_KEYBOARD_H_EADER_

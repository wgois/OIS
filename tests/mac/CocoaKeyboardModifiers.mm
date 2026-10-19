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

// Headless regression for the production Cocoa flagsChanged: callback.
#include "mac/CocoaInputManager.h"
#include "mac/CocoaKeyboard.h"

#include <iostream>
#include <vector>

using namespace OIS;

namespace
{
	struct Event
	{
		KeyCode key;
		bool down;
	};

	class Listener : public KeyListener
	{
	public:
		std::vector<Event> events;

		bool keyPressed(const KeyEvent& event)
		{
			events.push_back({event.key, true});
			return true;
		}

		bool keyReleased(const KeyEvent& event)
		{
			events.push_back({event.key, false});
			return true;
		}
	};

	class TestKeyboard : public CocoaKeyboard
	{
	public:
		TestKeyboard(CocoaInputManager* manager, bool buffered) :
		 CocoaKeyboard(manager, buffered, false) { }

		void flags(NSUInteger flags, unsigned short keyCode = 0x38)
		{
			NSEvent* event = [NSEvent keyEventWithType:NSEventTypeFlagsChanged
										location:NSZeroPoint
								   modifierFlags:flags
									   timestamp:1
									windowNumber:0
										 context:nil
									  characters:@""
							 charactersIgnoringModifiers:@""
									   isARepeat:NO
										 keyCode:keyCode];
			[mResponder flagsChanged:event];
		}
	};

	struct Fixture
	{
		// Do not call manager._initialize(): it enumerates HID devices and finds a window.
		// Its default nil window makes CocoaKeyboard's makeFirstResponder: a no-op.
		CocoaInputManager manager;
		Listener listener;
		TestKeyboard keyboard;

		Fixture(bool buffered = true) : keyboard(&manager, buffered)
		{
			keyboard._initialize();
			keyboard.setEventCallback(&listener);
		}
	};

	int failures = 0;
	int checks = 0;

	void check(bool condition, const char* description)
	{
		++checks;
		if(!condition)
		{
			++failures;
			std::cerr << "FAIL: " << description << '\n';
		}
	}

	void expectEvents(Fixture& fixture, std::initializer_list<Event> expected, const char* description)
	{
		fixture.keyboard.capture();
		const std::vector<Event>& actual = fixture.listener.events;
		bool equal = actual.size() == expected.size();
		unsigned int i = 0;
		for(const Event& event : expected)
		{
			if(i < actual.size())
				equal = equal && actual[i].key == event.key && actual[i].down == event.down;
			++i;
		}
		check(equal, description);
		fixture.listener.events.clear();
	}

	void individualModifiers()
	{
		struct Modifier
		{
			NSUInteger native;
			Keyboard::Modifier ois;
			KeyCode key;
		};
		const Modifier modifiers[] = {
			{NSEventModifierFlagShift, Keyboard::Shift, KC_LSHIFT},
			{NSEventModifierFlagOption, Keyboard::Alt, KC_LMENU},
			{NSEventModifierFlagControl, Keyboard::Ctrl, KC_LCONTROL}
		};
		for(const Modifier& modifier : modifiers)
		{
			Fixture fixture;
			fixture.keyboard.flags(modifier.native);
			check(fixture.keyboard.getModifiers() == modifier.ois, "press from zero sets the modifier");
			check(fixture.keyboard.isKeyDown(modifier.key), "modifier press updates key polling");
			expectEvents(fixture, {{modifier.key, true}}, "modifier press delivers one buffered callback");
			fixture.keyboard.flags(0);
			check(fixture.keyboard.getModifiers() == 0, "release clears the modifier");
			check(!fixture.keyboard.isKeyDown(modifier.key), "modifier release updates key polling");
			expectEvents(fixture, {{modifier.key, false}}, "modifier release delivers one buffered callback");
		}
	}

	void preserveOtherModifiers()
	{
		const NSUInteger all = NSEventModifierFlagShift | NSEventModifierFlagOption | NSEventModifierFlagControl;
		const unsigned int held = Keyboard::Shift | Keyboard::Alt | Keyboard::Ctrl;
		const NSUInteger native[] = {NSEventModifierFlagShift, NSEventModifierFlagOption, NSEventModifierFlagControl};
		const unsigned int ois[] = {Keyboard::Shift, Keyboard::Alt, Keyboard::Ctrl};
		for(unsigned int i = 0; i < 3; ++i)
		{
			Fixture fixture;
			fixture.keyboard.flags(all);
			// Isolate release masking from the separate press-from-zero defect.
			fixture.keyboard._getModifiers() = held;
			fixture.keyboard.flags(all & ~native[i]);
			check(fixture.keyboard.getModifiers() == (held & ~ois[i]), "release preserves every other held modifier");
		}
	}

	void deviceFlags()
	{
		Fixture fixture;
		fixture.keyboard.flags(NSEventModifierFlagShift | 0x2);
		check(fixture.keyboard.getModifiers() == Keyboard::Shift, "device-dependent bits do not hide a press");
		expectEvents(fixture, {{KC_LSHIFT, true}}, "device-dependent press delivers Shift");
		fixture.keyboard.flags(NSEventModifierFlagShift | 0x4);
		expectEvents(fixture, {}, "only device-dependent bits changing produces no callback");
		fixture.keyboard.flags(0x4);
		check(fixture.keyboard.getModifiers() == 0, "device-dependent release clears Shift");
		expectEvents(fixture, {{KC_LSHIFT, false}}, "device-dependent release delivers Shift");
		expectEvents(fixture, {}, "capture drains the modifier callback queue");
	}

	void multipleChanges()
	{
		Fixture fixture;
		fixture.keyboard.flags(NSEventModifierFlagShift | NSEventModifierFlagOption);
		check(fixture.keyboard.getModifiers() == (Keyboard::Shift | Keyboard::Alt), "one event can press multiple modifiers");
		expectEvents(fixture, {{KC_LSHIFT, true}, {KC_LMENU, true}}, "multiple presses each deliver a callback");
		fixture.keyboard.flags(NSEventModifierFlagControl);
		check(fixture.keyboard.getModifiers() == Keyboard::Ctrl, "one event can both press and release modifiers");
		check(!fixture.keyboard.isKeyDown(KC_LSHIFT) && !fixture.keyboard.isKeyDown(KC_LMENU) && fixture.keyboard.isKeyDown(KC_LCONTROL), "mixed transitions update all polled keys");
		expectEvents(fixture, {{KC_LSHIFT, false}, {KC_LMENU, false}, {KC_LCONTROL, true}}, "mixed changes use each flag's own direction");
		fixture.keyboard.flags(0);
		expectEvents(fixture, {{KC_LCONTROL, false}}, "last mixed modifier releases");
	}

	void capsLockAndNumericPad()
	{
		struct LockFlag
		{
			NSUInteger native;
			unsigned int ois;
			KeyCode key;
		};
		const LockFlag flags[] = {
			{NSEventModifierFlagCapsLock, Keyboard::CapsLock, KC_CAPITAL},
			{NSEventModifierFlagNumericPad, Keyboard::NumLock, KC_NUMLOCK}
		};
		for(const LockFlag& flag : flags)
		{
			Fixture fixture;
			for(unsigned int cycle = 0; cycle < 2; ++cycle)
			{
				fixture.keyboard.flags(flag.native, 0x47);
				check(fixture.keyboard.getModifiers() == flag.ois, "lock alias activation sets its modifier");
				check(fixture.keyboard.isKeyDown(flag.key), "lock alias activation sets its polled key");
				expectEvents(fixture, {{flag.key, true}}, "lock alias activation delivers one callback");
				fixture.keyboard.flags(0, 0x47);
				check(fixture.keyboard.getModifiers() == 0, "lock alias deactivation clears its modifier");
				check(!fixture.keyboard.isKeyDown(flag.key), "lock alias deactivation clears its polled key");
				expectEvents(fixture, {{flag.key, false}}, "lock alias deactivation delivers one callback");
			}
		}
	}

	void commandAndFunction()
	{
		Fixture fixture;
		fixture.keyboard.flags(NSEventModifierFlagCommand | NSEventModifierFlagFunction);
		check(fixture.keyboard.getModifiers() == 0, "Command and Function preserve existing OIS modifier mapping");
		check(fixture.keyboard.isKeyDown(KC_LWIN) && fixture.keyboard.isKeyDown(KC_APPS), "Command and Function update key polling");
		expectEvents(fixture, {{KC_LWIN, true}, {KC_APPS, true}}, "Command and Function both deliver press callbacks");
		fixture.keyboard.flags(0);
		expectEvents(fixture, {{KC_LWIN, false}, {KC_APPS, false}}, "Command and Function both deliver release callbacks");
	}

	void unbuffered()
	{
		Fixture fixture(false);
		fixture.keyboard.flags(NSEventModifierFlagOption | NSEventModifierFlagControl);
		check(fixture.keyboard.getModifiers() == (Keyboard::Alt | Keyboard::Ctrl), "unbuffered mode updates modifiers immediately");
		char keys[256];
		fixture.keyboard.copyKeyStates(keys);
		check(keys[KC_LMENU] && keys[KC_LCONTROL], "unbuffered mode copies the updated key states");
		expectEvents(fixture, {}, "unbuffered mode does not enqueue callbacks");
		fixture.keyboard.flags(0);
		check(fixture.keyboard.getModifiers() == 0, "unbuffered releases clear modifiers immediately");
	}
}

int main()
{
	@autoreleasepool
	{
		individualModifiers();
		preserveOtherModifiers();
		deviceFlags();
		multipleChanges();
		capsLockAndNumericPad();
		commandAndFunction();
		unbuffered();
	}
	std::cout << checks << " checks, " << failures << " failures\n";
	return failures ? 1 : 0;
}

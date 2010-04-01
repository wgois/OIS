======================================================================================
================ LICENSE =============================================================
======================================================================================

The zlib/libpng License

Copyright (c) 2005-2007 Phillip Castaneda (pjcast -- www.wreckedgames.com)

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


======================================================================================
================ LICENSE EXPLAINED ===================================================
======================================================================================
In case the license was not clear enough... Basically, you can link with this lib 
which puts no restrictions on what you have to license your code as. You can modify 
this lib, and not have release your changes. Though, as explained above, <bold> 
Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software </bold>

This is to allow users the greatest flexibility in what you can use this lib for.

======================================================================================
================ INFO ================================================================
======================================================================================

Cross Platform Object Oriented Input Lib System. Meant to be very robust and 
compatiable with many systems and operating systems

Win32/
	Contains Visual Studio .Net Solution Files
	Contains CodeBlocks + MinGW + StlPort project files for OIS
	
	---- Dependencies ------------------------------------------------------
	DirectInput 8
	Ogre & CEGUI 0.4.0 If building CEGUIOgre OIS Demo

	SDL/
		A test bed for an OIS InputManager with SDL as the backend. Not recommended;
		however, useful for platforms with non-native OIS ports for temporary use.

Linux/
	---- Dependencies ------------------------------------------------------
	X11
	Ogre (GLX Platform) & CEGUI 0.4.0 If building CEGUIOgre OIS Demo
        Newer Linux Kernel (2.6+ ?) for Event API - else, use --disable-joyevents

	Steps to build on Linux:
	./bootstrap
	./configure
	./make && make install

	---- Configure build options --------------------------------------------
	./configure --help              --- List all configure options
	./configure --disable-ogre      --- Disables CEGUIOgre ActionMapping Demo
	./configure --disable-joyevents --- Uses /dev/input/jsX instead of 
	                                    /dev/input/eventX

LinuxCB/
        Contains CodeBlock files for building OIS and Demos with codeblocks
        This project file looks for Ogre and other dependencies in /usr/local/lib
        and /usr/local/include. If installed elsewhere, modify the project
        settings. It also installs libOIS to ~/libs

Mac/
	
	XCode-1.5/
		Non-complete native OIS port.

	XCode-2.2/
		Working, complete, OIS port to OSX using SDL as a backend.
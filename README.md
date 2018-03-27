# OIS: Object oriented Input System

[![Software License](https://img.shields.io/badge/license-zlib%2Flibpng-green.svg)](LICENSE.md)
[![Build status](https://ci.appveyor.com/api/projects/status/8yenriuecb4pa4xj?svg=true)](https://ci.appveyor.com/project/Ybalrid/ois-0qdtw)
[![Build Status](https://travis-ci.org/wgois/OIS.svg?branch=master)](https://travis-ci.org/wgois/OIS)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://GitHub.com/wgois/OIS/graphs/commit-activity)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

Cross Platform Object Oriented Input Lib System. Meant to be very robust and
compatiable with many systems and operating systems.

### Linux

Dependencies: X11

To build and install:
```bash
    cmake -H. -B./build
    cd ./build
    make
    make install
```
### Windows

Dependencies: DirectInput 8

For Visual Studio:

```batch
    cmake -H. -B./build
    msbuild "build\OISFFConsoleApp.sln"
```

Or, simply open CMake-gui, and generate a visual studio solution in a few clicks.

### OSX

For XCode:

```bash
    cmake -H. -B./build
    cd ./build
    make

```
## License

```
The zlib/libpng License

Copyright © 2015 Andrew Fenn

Copyright © 2005-2010 Phillip Castaneda (pjcast -- www.wreckedgames.com)

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
```

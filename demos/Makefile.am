if PLATFORM_LINUX
AM_CPPFLAGS = $(STLPORT_CFLAGS) -I$(top_srcdir)/includes $(CFLAGS) -I/usr/X11R6/include
endif

if PLATFORM_WIN32
AM_CPPFLAGS = $(STLPORT_CFLAGS) -I$(top_srcdir)/includes -I$(top_srcdir)/Win32/CommandLine $(CFLAGS)
endif

noinst_PROGRAMS = ConsoleApp FFConsoleTest

ConsoleApp_SOURCES = OISConsole.cpp $(top_srcdir)/Win32/CommandLine/CommandLine.rc
ConsoleApp_LDFLAGS = -L$(top_builddir)/src
if PLATFORM_WIN32
ConsoleApp_LDADD = -lOIS -ldinput8 -ldxguid -lole32 -loleaut32 -luser32
else
ConsoleApp_LDADD = -lOIS
endif

FFConsoleTest_SOURCES = FFConsoleDemo.cpp $(top_srcdir)/Win32/DemoFFTest/FF.rc
FFConsoleTest_LDFLAGS = -L$(top_builddir)/src
if PLATFORM_WIN32
FFConsoleTest_LDADD = -lOIS -ldinput8 -ldxguid -lole32 -loleaut32 -luser32
else
FFConsoleTest_LDADD = -lOIS
endif

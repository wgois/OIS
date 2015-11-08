#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.50)
AC_INIT( [OIS], 1.4.0 )

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([includes/config.h])
AC_CONFIG_MACRO_DIR([m4])

case "$host" in
        *-*-mingw*|*-*-cygwin*)
                lt_cv_deplibs_check_method='pass_all'
        ;;
esac

LT_INIT

dnl Check for programs
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_INSTALL
AM_PROG_LIBTOOL

dnl Checking for STLPort
OIS_USE_STLPORT

CFLAGS="$CFLAGS"
CXXFLAGS="$CXXFLAGS"
LIBS="$LIBS"

case "$host" in
        *-*-mingw*|*-*-cygwin*)
                AC_DEFINE(PLATFORM_WIN32, 1, [Platform is Win32])
                AM_CONDITIONAL([PLATFORM_WIN32], [test xtest = xtest])
                AM_CONDITIONAL([PLATFORM_LINUX], [test xno = xtest])
                AM_CONDITIONAL([PLATFORM_APPLE], [test xno = xtest])
                PLATFORM_STRING="Win32"
                ;;
        *-*-linux*)
                AC_DEFINE(PLATFORM_LINUX, 1, [Platform is Linux])
                PLATFORM_STRING="Linux"
                AM_CONDITIONAL([PLATFORM_WIN32], [test xno = xtest])
                AM_CONDITIONAL([PLATFORM_LINUX], [test xtest = xtest])
                AM_CONDITIONAL([PLATFORM_APPLE], [test xno = xtest])
                dnl Detect X11
                AC_CHECK_HEADERS([X11/Xlib.h],, [AC_MSG_ERROR("Xlib.h not found - libx11-dev")])
                ;;
        *-*-darwin*)
                AC_MSG_WARN([Hello])
                AC_DEFINE(PLATFORM_APPLE, 1, [Platform is Apple])
                PLATFORM_STRING="Apple"
                AM_CONDITIONAL([PLATFORM_WIN32], [test xno = xtest])
                AM_CONDITIONAL([PLATFORM_LINUX], [test xno = xtest])
                AM_CONDITIONAL([PLATFORM_APPLE], [test xtest = xtest])
                ;;
        *)
                AC_MSG_WARN([*** Please add $host to configure.ac checks!])
                ;;
esac

dnl Added for BSD's
AC_PROG_LIBTOOL

AC_CONFIG_FILES([Makefile
                 includes/Makefile
                 src/Makefile
                 demos/Makefile
                 OIS.pc])
AC_OUTPUT

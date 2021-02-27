cmake_minimum_required(VERSION 2.8)
project(OIS)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CmakeModules)

set(CMAKE_CXX_STANDARD 11)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

set(OIS_MAJOR_VERSION 1)
set(OIS_MINOR_VERSION 5)
set(OIS_PATCH_VERSION 0)

set(OIS_VERSION ${OIS_MAJOR_VERSION}.${OIS_MINOR_VERSION}.${OIS_PATCH_VERSION})
set(OIS_COMPATIBILITY_VERSION ${OIS_MAJOR_VERSION}.${OIS_MINOR_VERSION}.0)
set(OIS_SOVERSION ${OIS_MAJOR_VERSION}.${OIS_MINOR_VERSION}.0)

set(OIS_LIBRARY_NAME OIS)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/includes/OISPrereqs.h.in ${CMAKE_CURRENT_SOURCE_DIR}/includes/OISPrereqs.h @ONLY)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

option(OIS_BUILD_SHARED_LIBS "Build shared libraries" ON)
option(OIS_BUILD_DEMOS "Build demo applications" ON)
set(CMAKE_MACOSX_RPATH 0)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/linux
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/SDL
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/iphone
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/mac
    ${CMAKE_CURRENT_SOURCE_DIR}/includes/win32
)

set(ois_source
    ${ois_source}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISInputManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISObject.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISEffect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISJoyStick.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISKeyboard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISForceFeedback.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/OISException.cpp"
)

set(BUILD_SHARED_LIBS ${OIS_BUILD_SHARED_LIBS})

if (UNIX AND NOT APPLE)

    find_package(X11 REQUIRED)
    include_directories(SYSTEM ${X11_INCLUDE_DIR})

    set(ois_source
        ${ois_source}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/EventHelpers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/LinuxForceFeedback.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/LinuxInputManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/LinuxJoyStickEvents.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/LinuxKeyboard.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/linux/LinuxMouse.cpp"
    )
endif()

if (WIN32)
    option(OIS_WIN32_XINPUT_SUPPORT "Add support XInput." ON)

    if(CMAKE_CROSSCOMPILING)
        MESSAGE(STATUS "Cross-compiling ... search libraries instead of WindowsSDK")
        find_library(DINPUT8_LIBRARY NAMES libdinput8.a dinput8.lib)
        find_library(XINPUT8_LIBRARY NAMES libxinput.a)
        find_library(DXGUID_LIBRARY  NAMES libdxguid.a)
        if (NOT DINPUT8_LIBRARY)
            MESSAGE(FATAL_ERROR " could not locate dinput8 DirectX library")
        endif()
        if (NOT XINPUT8_LIBRARY)
            MESSAGE(FATAL_ERROR " could not locate xinput DirectX library")
        endif()
        if (NOT DXGUID_LIBRARY)
            MESSAGE(FATAL_ERROR " could not locate dxguid DirectX library")
        endif()
        MESSAGE(STATUS " using " ${DINPUT8_LIBRARY} ", " ${XINPUT8_LIBRARY} " and " ${DXGUID_LIBRARY})
    else()
        find_package(WindowsSDK)
        # I'm assuming that this will be set under all normal circumstances
        # if not, the user can set it themselves
        if(WINDOWSSDK_FOUND)
            set(OIS_WIN32_DXSDK_DIR ${WINDOWSSDK_PREFERRED_DIR} CACHE PATH "Location of the DirectX SDK on your system.")
        else()
            set(OIS_WIN32_DXSDK_DIR "$ENV{DXSDK_DIR}" CACHE PATH "Location of the DirectX SDK on your system.")
        endif()

        if("${OIS_WIN32_DXSDK_DIR}" STREQUAL "")
            MESSAGE(FATAL_ERROR "Could not locate DirectX SDK on this system")
        else()
            MESSAGE(STATUS "Found DirectX SDK at ${OIS_WIN32_DXSDK_DIR}")
        endif()

        include_directories("${OIS_WIN32_DXSDK_DIR}/Include")
    endif()

    if(BUILD_SHARED_LIBS)
        add_definitions(-DOIS_DYNAMIC_LIB)
    endif()

    if(OIS_WIN32_XINPUT_SUPPORT)
        add_definitions(-DOIS_WIN32_XINPUT_SUPPORT)
    endif()

    set(ois_source
        ${ois_source}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Win32ForceFeedback.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Win32InputManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Win32JoyStick.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Win32KeyBoard.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Win32Mouse.cpp"
    )
endif()

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(FOUNDATION_LIBRARY Foundation)
    find_library(IOKIT_LIBRARY IOKit)
    find_path(COCOA_INCLUDE_DIR Cocoa/Cocoa.h)
    find_library(CARBON_LIBRARY Carbon)

    add_definitions(-x objective-c++)

    include_directories(SYSTEM ${COCOA_INCLUDE_DIR})
    set(ois_source
        ${ois_source}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/MacHelpers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/MacHIDManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/MacInputManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/MacJoyStick.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/MacKeyboard.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/MacMouse.cpp"

        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/CocoaInputManager.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/CocoaJoyStick.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/CocoaKeyboard.mm"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/mac/CocoaMouse.mm"
    )
endif()

source_group("OIS" FILES ${ois_source})

add_library(OIS ${ois_source})

# This is neccessary to get the dll import/export to work without modifying the
# source to use cmake's method of setting the dll import/export definitions
if (WIN32)
    SET_TARGET_PROPERTIES(OIS PROPERTIES COMPILE_FLAGS "-DOIS_NONCLIENT_BUILD")
endif()

set_target_properties(OIS PROPERTIES SOLUTION_FOLDER "libOIS")

if (APPLE)
    target_link_libraries(OIS ${COCOA_LIBRARY})
    target_link_libraries(OIS ${IOKIT_LIBRARY})
    target_link_libraries(OIS ${FOUNDATION_LIBRARY})
endif()

if (WIN32)
    if (CMAKE_CL_64)
        set(DIRECTX_ARCH x64)
    else()
        set(DIRECTX_ARCH x86)
    endif()

    if (WINDOWSSDK_FOUND)
        target_link_libraries(OIS "dinput8.lib" "dxguid.lib")
    elseif(CMAKE_CROSSCOMPILING)
        target_link_libraries(OIS "${DINPUT8_LIBRARY}" "${XINPUT8_LIBRARY}" "${DXGUID_LIBRARY}")
    else()
        target_link_libraries(OIS "${DXSDK_DIR}/Lib/${DIRECTX_ARCH}/dinput8.lib" "${DXSDK_DIR}/Lib/${DIRECTX_ARCH}/dxguid.lib")
    endif()
endif()

if(UNIX)

    if (NOT APPLE)
        add_dependencies(OIS X11)
        target_link_libraries(OIS X11)
    endif()

    set_target_properties(OIS PROPERTIES
        VERSION ${OIS_VERSION}
        SOVERSION ${OIS_SOVERSION}
    )
endif()

if (OIS_BUILD_DEMOS)
    source_group("ConsoleApp" FILES "${CMAKE_CURRENT_SOURCE_DIR}/demos/OISConsoleDemo/OISConsole.cpp")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)
    if (WIN32)
        add_executable(ConsoleApp "${CMAKE_CURRENT_SOURCE_DIR}/demos/OISConsoleDemo/OISConsole.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/demos/OISConsoleDemo/CommandLine.rc")
    else()
        add_executable(ConsoleApp "${CMAKE_CURRENT_SOURCE_DIR}/demos/OISConsoleDemo/OISConsole.cpp")
    endif()
    set_target_properties(ConsoleApp PROPERTIES SOLUTION_FOLDER "OIS Console Demo")
    add_dependencies(ConsoleApp OIS)
    target_link_libraries(ConsoleApp "OIS")
    if (APPLE)
        target_link_libraries(ConsoleApp ${CARBON_LIBRARY})
    endif()

    source_group("FFConsoleApp" FILES "${CMAKE_CURRENT_SOURCE_DIR}/demos/FFConsoleDemo/FFConsoleDemo.cpp")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)
    if (WIN32)
        add_executable(FFConsoleApp "${CMAKE_CURRENT_SOURCE_DIR}/demos/FFConsoleDemo/FFConsoleDemo.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/demos/FFConsoleDemo/FF.rc")
    else()
        add_executable(FFConsoleApp "${CMAKE_CURRENT_SOURCE_DIR}/demos/FFConsoleDemo/FFConsoleDemo.cpp")
    endif()
    set_target_properties(FFConsoleApp PROPERTIES SOLUTION_FOLDER "OIS FF Console Demo")
    add_dependencies(FFConsoleApp OIS)
    target_link_libraries(FFConsoleApp "OIS")
    if (APPLE)
        target_link_libraries(FFConsoleApp ${CARBON_LIBRARY})
    endif()

endif()

if(UNIX)
	include(GNUInstallDirs)
	set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
else()
	set(LIB_INSTALL_DIR "lib")
endif()

set(OIS_LIB_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR})

file(GLOB PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h")

install(FILES ${PUBLIC_HEADERS}
  DESTINATION include/ois
)

install(TARGETS OIS
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
  FRAMEWORK DESTINATION Library/Frameworks
)

if(MSVC AND BUILD_SHARED_LIBS)
  install(FILES $<TARGET_PDB_FILE:OIS> DESTINATION bin OPTIONAL)
endif(MSVC AND BUILD_SHARED_LIBS)

if(UNIX)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OIS.pc.in ${CMAKE_BINARY_DIR}/OIS.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/OIS.pc DESTINATION ${OIS_LIB_DIRECTORY}/pkgconfig)
endif()

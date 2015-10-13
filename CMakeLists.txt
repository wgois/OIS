cmake_minimum_required(VERSION 2.8)
project(OIS)

set(OIS_MAJOR_VERSION 1)
set(OIS_MINOR_VERSION 4)
set(OIS_PATCH_VERSION 0)

set(OIS_VERSION ${OIS_MAJOR_VERSION}.${OIS_MINOR_VERSION}.${OIS_PATCH_VERSION})
set(OIS_COMPATIBILITY_VERSION ${OIS_MAJOR_VERSION}.${OIS_MINOR_VERSION}.0)
set(OIS_SOVERSION ${OIS_MAJOR_VERSION}.${OIS_MINOR_VERSION}.0)

set(OIS_LIBRARY_NAME OIS)

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
    "${CMAKE_SOURCE_DIR}/src/OISInputManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/OISObject.cpp"
    "${CMAKE_SOURCE_DIR}/src/OISEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/OISJoyStick.cpp"
    "${CMAKE_SOURCE_DIR}/src/OISKeyboard.cpp"
    "${CMAKE_SOURCE_DIR}/src/OISForceFeedback.cpp"
    "${CMAKE_SOURCE_DIR}/src/OISException.cpp"
)

if (UNIX AND NOT APPLE)

    find_package(X11 REQUIRED)
    include_directories(SYSTEM ${X11_INCLUDE_DIR})

    set(ois_source
        ${ois_source}
        "${CMAKE_SOURCE_DIR}/src/linux/EventHelpers.cpp"
        "${CMAKE_SOURCE_DIR}/src/linux/LinuxForceFeedback.cpp"
        "${CMAKE_SOURCE_DIR}/src/linux/LinuxInputManager.cpp"
        "${CMAKE_SOURCE_DIR}/src/linux/LinuxJoyStickEvents.cpp"
        "${CMAKE_SOURCE_DIR}/src/linux/LinuxKeyboard.cpp"
        "${CMAKE_SOURCE_DIR}/src/linux/LinuxMouse.cpp"
    )
endif()

if (WIN32)

    set(ois_source
        ${ois_source}
        "${CMAKE_SOURCE_DIR}/src/win32/Win32ForceFeedback.cpp"
        "${CMAKE_SOURCE_DIR}/src/win32/Win32InputManager.cpp"
        "${CMAKE_SOURCE_DIR}/src/win32/Win32JoyStick.cpp"
        "${CMAKE_SOURCE_DIR}/src/win32/Win32KeyBoard.cpp"
        "${CMAKE_SOURCE_DIR}/src/win32/Win32Mouse.cpp"
    )
endif()

if (APPLE)

    set(ois_source
        ${ois_source}
        "${CMAKE_SOURCE_DIR}/src/mac/MacHelpers.cpp"
        "${CMAKE_SOURCE_DIR}/src/mac/MacHIDManager.cpp"
        "${CMAKE_SOURCE_DIR}/src/mac/MacInputManager.cpp"
        "${CMAKE_SOURCE_DIR}/src/mac/MacJoyStick.cpp"
        "${CMAKE_SOURCE_DIR}/src/mac/MacKeyboard.cpp"
        "${CMAKE_SOURCE_DIR}/src/mac/MacMouse.cpp"
    )
endif()

source_group("OIS" FILES ${ois_source})
add_library(OIS ${ois_source})

add_definitions(-DOIS_VERSION_MAJOR=${OIS_MAJOR_VERSION})
add_definitions(-DOIS_VERSION_MINOR=${OIS_MINOR_VERSION})
add_definitions(-DOIS_VERSION_PATCH=${OIS_PATCH_VERSION})
add_definitions(-DOIS_VERSION_NAME="\\"${OIS_VERSION}\\"")

if(UNIX)

    if (NOT APPLE)
        add_dependencies(OIS X11)
    endif()

    set_target_properties(OIS PROPERTIES
        VERSION ${OIS_VERSION}
        SOVERSION ${OIS_SOVERSION}
    )
endif()

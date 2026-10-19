add_executable(ois_portable_contracts portable/MouseStateContracts.cpp)
target_include_directories(ois_portable_contracts PRIVATE "${PROJECT_SOURCE_DIR}/includes")
target_link_libraries(ois_portable_contracts OIS)
if(WIN32 AND OIS_BUILD_SHARED_LIBS)
    add_custom_command(TARGET ois_portable_contracts POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:OIS>" "$<TARGET_FILE_DIR:ois_portable_contracts>"
        VERBATIM)
endif()
add_test(NAME ois_portable_contracts COMMAND ois_portable_contracts)

if(APPLE)
    add_executable(ois_cocoa_keyboard_modifiers mac/CocoaKeyboardModifiers.mm)
    target_include_directories(ois_cocoa_keyboard_modifiers PRIVATE "${PROJECT_SOURCE_DIR}/includes")
    target_link_libraries(ois_cocoa_keyboard_modifiers OIS ${COCOA_LIBRARY})
    add_test(NAME ois_cocoa_keyboard_modifiers COMMAND ois_cocoa_keyboard_modifiers)
endif()

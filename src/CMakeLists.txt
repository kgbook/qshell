
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC_FILES main.cpp
        resources/resources.qrc
        ui/MainWindow.cpp
        core/CryptoHelper.cpp
        core/ConfigManager.cpp
        ui/session/SessionTabWidget.cpp
        ui/session/SessionTreeWidget.cpp
        ui/session/SessionEditDialog.cpp
        ui/session/GroupEditDialog.cpp
        ui/terminal/BaseTerminal.cpp
        ui/terminal/SerialTerminal.cpp
        ui/terminal/LocalTerminal.cpp
        ui/terminal/SSHTerminal.h
        ui/button_toolbar/CommandButtonBar.cpp )

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(SRC_FILES ${SRC_FILES} ui/terminal/SSHTerminalLinux.cpp)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(SRC_FILES ${SRC_FILES} ui/terminal/SSHTerminalWindows.cpp)
else ()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} not supported")
endif ()

add_executable(qshell main.cpp
        resources/resources.qrc
        ${SRC_FILES}
)

target_link_libraries(qshell
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::SerialPort
        qtermwidget
        ptyqt
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(qshell libssh2_static)
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "$ENV{Qt6_DIR}/bin")
    # 编译后执行 windeployqt
    add_custom_command(TARGET qshell POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            --dir "$<TARGET_FILE_DIR:qshell>"
            "$<TARGET_FILE:qshell>"
            COMMENT "Running windeployqt to copy DLLs..."
    )
endif ()
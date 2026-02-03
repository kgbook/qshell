
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC_FILES main.cpp
        resources/resources.qrc
        ui/MainWindow.cpp
        ui/SettingDialog.cpp
        core/CryptoHelper.cpp
        core/ConfigManager.cpp
        ui/session/CollapsibleDockWidget.cpp
        ui/session/SessionTabWidget.cpp
        ui/session/SessionTreeWidget.cpp
        ui/session/SessionEditDialog.cpp
        ui/session/GroupEditDialog.cpp
        ui/terminal/BaseTerminal.cpp
        ui/terminal/SerialTerminal.cpp
        ui/terminal/LocalTerminal.cpp
        ui/terminal/SSHTerminal.h
        ui/command/CommandHistoryDialog.cpp
        ui/command/CommandWindow.cpp
        ui/command/CommandButtonBar.cpp
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(SRC_FILES ${SRC_FILES} ui/terminal/SSHTerminalLinux.cpp)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(SRC_FILES ${SRC_FILES} ui/terminal/SSHTerminalWindows.cpp)
else ()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} not supported")
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(qshell WIN32
            ${SRC_FILES}
            resources/resources.qrc
    )
    target_link_libraries(qshell libssh2_static)
else ()
    add_executable(qshell
            ${SRC_FILES}
            resources/resources.qrc
    )
endif ()

target_link_libraries(qshell
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::SerialPort
        qtermwidget
        ptyqt
)

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

if (CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Darwin")
install(TARGETS qshell
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES resources/qshell.desktop
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/)

install(FILES resources/images/qshell.png
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps)
endif ()


# ==================== CPack DEB 打包配置 ====================
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    # 版本号 - 从命令行传入，默认 1.0.0
    if(NOT DEFINED APP_VERSION)
        set(APP_VERSION "1.0.0")
    endif()

    # 基本信息
    set(CPACK_PACKAGE_NAME "qshell")
    set(CPACK_PACKAGE_VERSION "${APP_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QShell Terminal Emulator")
    set(CPACK_PACKAGE_DESCRIPTION "A multi-protocol terminal emulator supporting SSH, Serial, and Local terminals.")
    set(CPACK_PACKAGE_VENDOR "qiushao")
    set(CPACK_PACKAGE_CONTACT "360325900@qq.com")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/qiushao/qshell")

    # 设置打包类型为 DEB
    set(CPACK_GENERATOR "DEB")

    # DEB 特定配置
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "qiushao <360325900@qq.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")

    # 自动检测依赖
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    include(CPack)
endif()

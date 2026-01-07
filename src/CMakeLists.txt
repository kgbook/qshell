add_executable(qshell main.cpp
        resources/resources.qrc
        ui/MainWindow.cpp
        ui/session/SessionTabWidget.cpp
        ui/session/SessionTreeWidget.cpp
        ui/button_toolbar/CommandButtonBar.cpp
)

target_link_libraries(qshell
        Qt::Core
        Qt::Gui
        Qt::Widgets
        qtermwidget
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
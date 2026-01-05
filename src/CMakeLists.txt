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
)
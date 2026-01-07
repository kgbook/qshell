
set(SRC_HEADERS
        util/CharWidth.h
        util/CharacterColor.h
        util/Character.h
        util/ColorScheme.h
        util/Filter.h
        util/History.h
        util/HistorySearch.h
        util/KeyboardTranslator.h
        util/SearchBar.h
        util/TerminalCharacterDecoder.h
        Emulation.h
        Vt102Emulation.h
        Screen.h
        ScreenWindow.h
        TerminalDisplay.h
        qtermwidget.h
        qtermwidget_version.h)

set(SRC_FILES
        util/CharWidth.cpp
        util/ColorScheme.cpp
        util/Filter.cpp
        util/History.cpp
        util/HistorySearch.cpp
        util/KeyboardTranslator.cpp
        util/SearchBar.cpp
        util/TerminalCharacterDecoder.cpp
        Emulation.cpp
        Vt102Emulation.cpp
        Screen.cpp
        ScreenWindow.cpp
        TerminalDisplay.cpp
        qtermwidget.cpp
)

set(QRC_FILES res.qrc)
set(QFORMS_FILES util/SearchBar.ui)

add_library(qtermwidget
        ${SRC_FILES}
        ${SRC_HEADERS}
        ${QRC_FILES}
        ${QFORMS_FILES}
)

target_link_libraries(qtermwidget PUBLIC
        Qt6::Core Qt6::Widgets Qt6::Network
)

find_package(utf8proc QUIET CONFIG)

if (utf8proc_FOUND)
    target_link_libraries(qtermwidget PRIVATE utf8proc::utf8proc)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(UTF8PROC REQUIRED IMPORTED_TARGET libutf8proc)
    target_link_libraries(qtermwidget PRIVATE PkgConfig::UTF8PROC)
endif()

set(EXPORT_HEADERS util ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(qtermwidget PUBLIC ${EXPORT_HEADERS})

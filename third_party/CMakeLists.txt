set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE)  # third party code, disable Clang-Tidy

add_subdirectory(utf8proc-2.11.2)
add_subdirectory(qtermwidget)
add_subdirectory(ptyqt)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_subdirectory(libssh2-1.11.1)
endif ()
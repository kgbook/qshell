set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE)  # third party code, disable Clang-Tidy

add_subdirectory(utf8proc-2.11.2)
add_subdirectory(qtermwidget)
add_subdirectory(ptyqt)
add_subdirectory(lua-5.4.8)
add_subdirectory(sol2-3.3.0)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CRYPTO_BACKEND "WinCNG" CACHE STRING "" FORCE)
    add_subdirectory(libssh2-1.11.1)
endif ()
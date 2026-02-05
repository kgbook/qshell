# Lua 源文件（排除 lua.c 和 luac.c，这两个是独立程序的入口）
set(LUA_SOURCES
        src/lapi.c
        src/lauxlib.c
        src/lbaselib.c
        src/lcode.c
        src/lcorolib.c
        src/lctype.c
        src/ldblib.c
        src/ldebug.c
        src/ldo.c
        src/ldump.c
        src/lfunc.c
        src/lgc.c
        src/linit.c
        src/liolib.c
        src/llex.c
        src/lmathlib.c
        src/lmem.c
        src/loadlib.c
        src/lobject.c
        src/lopcodes.c
        src/loslib.c
        src/lparser.c
        src/lstate.c
        src/lstring.c
        src/lstrlib.c
        src/ltable.c
        src/ltablib.c
        src/ltm.c
        src/lundump.c
        src/lutf8lib.c
        src/lvm.c
        src/lzio.c
)

# 创建静态库
add_library(lua_static STATIC ${LUA_SOURCES})

target_include_directories(lua_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

## 平台定义
if(UNIX AND NOT APPLE)
    target_compile_definitions(lua_static PRIVATE LUA_USE_LINUX)
    target_link_libraries(lua_static PRIVATE m dl)
elseif(APPLE)
    target_compile_definitions(lua_static PRIVATE LUA_USE_MACOSX)
    target_link_libraries(lua_static PRIVATE m)
elseif(WIN32)
    # Windows 不需要额外定义
endif()

# C++ 项目中使用需要 extern "C"
target_compile_definitions(lua_static PUBLIC LUA_COMPAT_5_4)

# 别名方便使用
add_library(Lua::Lua ALIAS lua_static)

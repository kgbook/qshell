add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Sol2 需要 C++17
target_compile_features(sol2 INTERFACE cxx_std_17)
# macOS 兼容性修复
if(APPLE)
    target_compile_definitions(sol2 INTERFACE
            SOL_ALL_SAFETIES_ON=1
            SOL_USING_CXX_LUA=0
    )
endif()

add_library(sol2::sol2 ALIAS sol2)

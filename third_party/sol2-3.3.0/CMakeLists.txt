add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Sol2 需要 C++17
target_compile_features(sol2 INTERFACE cxx_std_17)
# Apple Clang 兼容性修复 - 关键宏定义
if(APPLE)
    target_compile_definitions(sol2 INTERFACE
            SOL_ALL_SAFETIES_ON=1
            SOL_STD_VARIANT=1
            # 禁用有问题的 tl::optional 引用特化
            TL_OPTIONAL_NO_CONSTRR=1
    )
endif()

add_library(sol2::sol2 ALIAS sol2)

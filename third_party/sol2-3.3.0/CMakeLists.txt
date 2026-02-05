add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Sol2 需要 C++17
target_compile_features(sol2 INTERFACE cxx_std_17)

add_library(sol2::sol2 ALIAS sol2)

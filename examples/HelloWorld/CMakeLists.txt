file(GLOB_RECURSE HELLO_WORLD_HEADERS "*.hpp")
file(GLOB_RECURSE HELLO_WORLD_SOURCES "*.cpp")

add_executable(HelloWorld ${HELLO_WORLD_HEADERS} ${HELLO_WORLD_SOURCES})

target_link_libraries(HelloWorld PRIVATE ink::ink)
target_compile_definitions(HelloWorld PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX")

# Compile HLSL.
include("FindD3D12")
find_package(D3D12 REQUIRED)

set(HELLO_WORLD_GENERATED_INCLUDE_DIR "$<TARGET_FILE_DIR:HelloWorld>/_generated/include")

add_custom_command(
    TARGET HelloWorld PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HELLO_WORLD_GENERATED_INCLUDE_DIR}
    COMMAND ${D3D12_DXC} -E "vertex_main" -T "vs_6_0" -Ges -Fh "${HELLO_WORLD_GENERATED_INCLUDE_DIR}/triangle.vs.hlsl.hpp" "triangle.hlsl"
    COMMAND ${D3D12_DXC} -E "pixel_main" -T "ps_6_0" -Ges -Fh "${HELLO_WORLD_GENERATED_INCLUDE_DIR}/triangle.ps.hlsl.hpp" "triangle.hlsl"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Compiling HelloWorld shaders."
    VERBATIM
)

target_include_directories(HelloWorld PRIVATE "${HELLO_WORLD_GENERATED_INCLUDE_DIR}")

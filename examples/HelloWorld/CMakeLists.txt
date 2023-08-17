file(GLOB_RECURSE HELLO_WORLD_HEADERS "*.hpp")
file(GLOB_RECURSE HELLO_WORLD_SOURCES "*.cpp")

add_executable(HelloWorld ${HELLO_WORLD_HEADERS} ${HELLO_WORLD_SOURCES})

target_link_libraries(HelloWorld PRIVATE ink::static)
target_compile_definitions(HelloWorld PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX")

# Compile HLSL.
include("FindD3D12")
find_package(D3D12 REQUIRED)

get_filename_component(HELLO_WORLD_TARGET_DIR "$<TARGET_FILE:HelloWorld>" DIRECTORY)
set(HELLO_WORLD_SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/_generated/include")

add_custom_target(
    shaders ALL
    COMMENT "Compiling triangle.hlsl"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HELLO_WORLD_SHADER_DIR}
    COMMAND ${D3D12_DXC} -E "vertex_main" -T "vs_6_0" -Ges -Fh "${HELLO_WORLD_SHADER_DIR}/triangle.vs.hlsl.hpp" "triangle.hlsl"
    COMMAND ${D3D12_DXC} -E "pixel_main" -T "ps_6_0" -Ges -Fh "${HELLO_WORLD_SHADER_DIR}/triangle.ps.hlsl.hpp" "triangle.hlsl"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)

target_include_directories(HelloWorld PRIVATE "${HELLO_WORLD_SHADER_DIR}")
add_dependencies(HelloWorld shaders)

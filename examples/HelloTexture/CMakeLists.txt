file(GLOB_RECURSE HELLO_TEXTURE_HEADERS "*.hpp")
file(GLOB_RECURSE HELLO_TEXTURE_SOURCES "*.cpp")

add_executable(HelloTexture ${HELLO_TEXTURE_HEADERS} ${HELLO_TEXTURE_SOURCES})

target_link_libraries(HelloTexture PRIVATE ink::ink DirectXTex)
target_compile_definitions(HelloTexture PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX")

# Compile HLSL.
include("FindD3D12")
find_package(D3D12 REQUIRED)

set(HELLO_TEXTURE_GENERATED_INCLUDE_DIR "$<TARGET_FILE_DIR:HelloTexture>/_generated/include")

add_custom_command(
    TARGET HelloTexture PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HELLO_TEXTURE_GENERATED_INCLUDE_DIR}
    COMMAND ${D3D12_DXC} -E "vertex_main" -T "vs_6_0" -Ges -Fh "${HELLO_TEXTURE_GENERATED_INCLUDE_DIR}/texture.vs.hlsl.hpp" "texture.hlsl"
    COMMAND ${D3D12_DXC} -E "pixel_main" -T "ps_6_0" -Ges -Fh "${HELLO_TEXTURE_GENERATED_INCLUDE_DIR}/texture.ps.hlsl.hpp" "texture.hlsl"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Compiling HelloTexture shaders."
    VERBATIM
)

add_custom_command(
    TARGET HelloTexture POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "asset" "$<TARGET_FILE_DIR:HelloTexture>/asset"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Copying HelloTexture assets."
    VERBATIM
)

target_include_directories(HelloTexture PRIVATE "${HELLO_TEXTURE_GENERATED_INCLUDE_DIR}")

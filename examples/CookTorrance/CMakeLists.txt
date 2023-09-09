file(GLOB_RECURSE COOK_TORRANCE_HEADERS "*.hpp")
file(GLOB_RECURSE COOK_TORRANCE_SOURCES "*.cpp")

add_executable(CookTorrance ${COOK_TORRANCE_HEADERS} ${COOK_TORRANCE_SOURCES})

target_link_libraries(CookTorrance PRIVATE ink_example_common)
target_compile_definitions(CookTorrance PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX")

# Compile HLSL.
include("FindD3D12")
find_package(D3D12 REQUIRED)

set(COOK_TORRANCE_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_generated/include")

add_custom_target(
    CookTorranceShaders ALL
    DEPENDS "cook_torrance.hlsl"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${COOK_TORRANCE_GENERATED_INCLUDE_DIR}
    COMMAND ${D3D12_DXC} -E "rootsig" -T "rootsig_1_0" -rootsig-define "rootsig" -Fh "${COOK_TORRANCE_GENERATED_INCLUDE_DIR}/cook_torrance.rootsig.hlsl.hpp" "cook_torrance.hlsl"
    COMMAND ${D3D12_DXC} -E "vertex_main" -T "vs_6_0" -Ges -Fh "${COOK_TORRANCE_GENERATED_INCLUDE_DIR}/cook_torrance.vs.hlsl.hpp" "cook_torrance.hlsl"
    COMMAND ${D3D12_DXC} -E "pixel_main" -T "ps_6_0" -Ges -Fh "${COOK_TORRANCE_GENERATED_INCLUDE_DIR}/cook_torrance.ps.hlsl.hpp" "cook_torrance.hlsl"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Compiling CookTorrance shaders."
    VERBATIM
)

add_dependencies(CookTorrance CookTorranceShaders)
target_include_directories(CookTorrance PRIVATE "${COOK_TORRANCE_GENERATED_INCLUDE_DIR}")

add_custom_command(
    TARGET CookTorrance POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "asset" "${CMAKE_CURRENT_BINARY_DIR}/asset"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Copying CookTorrance assets."
    VERBATIM
)

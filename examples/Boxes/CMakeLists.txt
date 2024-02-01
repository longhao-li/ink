file(GLOB_RECURSE BOXES_HEADERS "*.hpp")
file(GLOB_RECURSE BOXES_SOURCES "*.cpp")

add_executable(Boxes ${BOXES_HEADERS} ${BOXES_SOURCES})

target_link_libraries(Boxes PRIVATE ink_example_common)
target_compile_definitions(Boxes PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX")

# Compile HLSL.
include("FindD3D12")
find_package(D3D12 REQUIRED)

set(BOXES_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_generated/include")

add_custom_target(
    BoxesShaders ALL
    DEPENDS "boxes.hlsl"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BOXES_GENERATED_INCLUDE_DIR}
    COMMAND ${D3D12_DXC} -E "rootsig" -T "rootsig_1_0" -rootsig-define "rootsig" -Fh "${BOXES_GENERATED_INCLUDE_DIR}/boxes.rootsig.hlsl.hpp" "boxes.hlsl"
    COMMAND ${D3D12_DXC} -E "vertex_main" -T "vs_6_0" -Ges -Fh "${BOXES_GENERATED_INCLUDE_DIR}/boxes.vs.hlsl.hpp" "boxes.hlsl"
    COMMAND ${D3D12_DXC} -E "point_light_pixel_main" -T "ps_6_0" -DPOINT_LIGHT=1 -Ges -Fh "${BOXES_GENERATED_INCLUDE_DIR}/boxes.point_light.ps.hlsl.hpp" "boxes.hlsl"
    COMMAND ${D3D12_DXC} -E "spot_light_pixel_main" -T "ps_6_0" -DSPOT_LIGHT=1 -Ges -Fh "${BOXES_GENERATED_INCLUDE_DIR}/boxes.spot_light.ps.hlsl.hpp" "boxes.hlsl"
    COMMAND ${D3D12_DXC} -E "directional_light_pixel_main" -T "ps_6_0" -DDIRECTIONAL_LIGHT=1 -Ges -Fh "${BOXES_GENERATED_INCLUDE_DIR}/boxes.directional_light.ps.hlsl.hpp" "boxes.hlsl"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Compiling Boxes shaders."
    VERBATIM
)

add_dependencies(Boxes BoxesShaders)
target_include_directories(Boxes PRIVATE "${BOXES_GENERATED_INCLUDE_DIR}")


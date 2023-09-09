file(GLOB_RECURSE INK_EXAMPLE_COMMON_HEADERS "*.hpp")
file(GLOB_RECURSE INK_EXAMPLE_COMMON_SOURCES "*.cpp")

add_library(ink_example_common STATIC ${INK_EXAMPLE_COMMON_HEADERS} ${INK_EXAMPLE_COMMON_SOURCES})

target_link_libraries(
    ink_example_common
    PUBLIC ink::ink
    PRIVATE tinygltf
)

target_include_directories(ink_example_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(ink_example_common PUBLIC "UNICODE" "_UNICODE" "NOMINMAX" "WIN32_LEAN_AND_MEAN")

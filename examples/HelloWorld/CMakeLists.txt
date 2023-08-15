file(GLOB_RECURSE HELLO_WORLD_HEADERS "*.hpp")
file(GLOB_RECURSE HELLO_WORLD_SOURCES "*.cpp")

add_executable(HelloWorld ${HELLO_WORLD_HEADERS} ${HELLO_WORLD_SOURCES})

target_link_libraries(HelloWorld PRIVATE ink::static)
target_compile_definitions(HelloWorld PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX")

file(GLOB_RECURSE INK_TEST_HEADER_FILES "*.hpp")
file(GLOB_RECURSE INK_TEST_SOURCE_FILES "*.cpp")

include("CPM")

# Add Catch2
CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    VERSION 3.4.0
    OPTIONS "BUILD_TESTING OFF"
            "CATCH_INSTALL_DOCS OFF"
            "CATCH_INSTALL_EXTRAS OFF"
)

add_executable(inkTest ${INK_TEST_HEADER_FILES} ${INK_TEST_SOURCE_FILES})

target_compile_definitions(
    inkTest
    PRIVATE "WIN32_LEAN_AND_MEAN" "NOMINMAX" "UNICODE" "_UNICODE"
)

# Compiler flags.
if(MSVC)
    target_compile_options(inkTest PRIVATE "/permissive-" "/W4")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        target_compile_options(inkTest PRIVATE "/volatile:iso" "/Zc:__cplusplus" "/Zc:preprocessor" "/utf-8")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(
        inkTest
        PRIVATE "-Wall" "-Wextra" "-Wmost" "-pedantic" "-Wconversion" "-Wcast-align" "-Wshadow"
                "-Wmissing-field-initializers" "-Wno-language-extension-token"
    )
endif()

# Link external library.
target_link_libraries(inkTest PRIVATE ink::ink Catch2::Catch2WithMain)

# Use pre-compiled headers to speed up compilation time.
target_precompile_headers(
    inkTest
    PRIVATE "<catch2/catch_all.hpp>"
)

# Setup test for CMake.
include("CTest")
include("Catch")
catch_discover_tests(inkTest)

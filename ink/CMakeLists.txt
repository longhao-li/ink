if(NOT INK_BUILD_STATIC_LIBS AND NOT INK_BUILD_STATIC_LIBS)
    message(FATAL_ERROR "You must build at least one of the static or shared libraries.")
endif()

file(GLOB_RECURSE INK_HEADER_FILES "*.hpp")
file(GLOB_RECURSE INK_SOURCE_FILES "*.cpp")

# Compiler flags.
if(MSVC)
    set(INK_COMPILER_OPTIONS "/permissive-" "/W4")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        set(INK_COMPILER_OPTIONS ${INK_COMPILER_OPTIONS} "/volatile:iso" "/Zc:__cplusplus" "/Zc:preprocessor" "/utf-8")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(INK_COMPILER_OPTIONS "-Wall" "-Wextra" "-Wmost" "-pedantic" "-Wconversion" "-Wcast-align" "-Wshadow"
                             "-Wmissing-field-initializers" "-Wno-language-extension-token")
else()
    set(INK_COMPILER_OPTIONS)
endif()


# Precompiled header.
set(
    INK_PRECOMPILED_HEADERS "<Windows.h>" "<concurrent_queue.h>" "<d3d12.h>" "<d3dcompiler.h>" 
                            "<dxgi1_6.h>" "<wrl/client.h>"
                            "<algorithm>" "<array>" "<atomic>" "<cassert>" "<cmath>" "<functional>"
                            "<limits>" "<mutex>" "<string>" "<vector>"
)

# Build static library.
if(INK_BUILD_STATIC_LIBS)
    add_library(ink-static STATIC ${INK_HEADER_FILES} ${INK_SOURCE_FILES})
    add_library(ink::static ALIAS ink-static)
    if(NOT INK_BUILD_SHARED_LIBS)
        add_library(ink::ink ALIAS ink-static)
    endif()

    # set_target_properties(ink-static PROPERTIES OUTPUT_NAME "ink")

    # Precompiled header.
    target_precompile_headers(ink-static PRIVATE ${INK_PRECOMPILED_HEADERS})

    # Include directories.
    target_include_directories(
        ink-static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # Compile definitions.
    target_compile_definitions(ink-static PRIVATE "UNICODE" "_UNICODE" "WIN32_LEAN_AND_MEAN" "NOMINMAX")

    # Link external libraries.
    target_link_libraries(ink-static PUBLIC "d3d12" "dxgi" "d3dcompiler")

    # Compiler options.
    target_compile_options(ink-static PRIVATE ${INK_COMPILER_OPTIONS})
endif()

# Build shared library.
if(INK_BUILD_SHARED_LIBS)
    add_library(ink-shared SHARED ${INK_HEADER_FILES} ${INK_SOURCE_FILES})
    add_library(ink::shared ALIAS ink-shared)
    add_library(ink::ink ALIAS ink-shared)

    set_target_properties(
        ink-shared PROPERTIES
        OUTPUT_NAME "ink"
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET "hidden"
        VISIBILITY_INLINES_HIDDEN ON
        INTERPROCEDURAL_OPTIMIZATION ON
    )

    # Precompiled header.
    target_precompile_headers(ink-shared PRIVATE ${INK_PRECOMPILED_HEADERS})

    # Include directories.
    target_include_directories(
        ink-shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    # Compile definitions.
    target_compile_definitions(
        ink-shared
        PRIVATE "UNICODE" "_UNICODE" "WIN32_LEAN_AND_MEAN" "NOMINMAX" "INK_BUILD_SHARED_LIBRARY"
        PUBLIC  "INK_SHARED_LIBRARY"
    )

    # Link external libraries.
    target_link_libraries(ink-shared PUBLIC "d3d12" "dxgi" "d3dcompiler")

    # Compiler options.
    target_compile_options(ink-shared PRIVATE ${INK_COMPILER_OPTIONS})
endif()

include("CPM")
# Use DirectXTex to load textures.
CPMAddPackage(
    NAME DirectXTex
    GITHUB_REPOSITORY microsoft/DirectXTex
    GIT_TAG jun2023
    OPTIONS "BUILD_TOOLS OFF"
            "BUILD_SAMPLE OFF"
            "BUILD_DX11 OFF"
            "BC_USE_OPENMP OFF"
)

add_subdirectory("HelloWorld")
add_subdirectory("HelloTexture")

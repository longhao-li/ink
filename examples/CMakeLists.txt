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

# Use tinygltf to load glTF models.
CPMAddPackage(
    NAME tinygltf
    GITHUB_REPOSITORY syoyo/tinygltf
    GIT_TAG v2.8.14
    OPTIONS "TINYGLTF_BUILD_LOADER_EXAMPLE OFF"
            "TINYGLTF_INSTALL OFF"
)

add_subdirectory("Common")
add_subdirectory("HelloWorld")
add_subdirectory("HelloTexture")
add_subdirectory("CookTorrance")

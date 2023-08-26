# ink

A rendering framework for DirectX 12.

## Introduction

ink is a C++17 rendering framework for DirectX 12. It is designed to be a thin layer on top of DirectX 12, providing a more convenient interface for rendering. Here is a brief list of features:

- Easy-to-use interface with slight performance overhead.
- Object-based design. This library does not rely on global state, and it is safe to mix the usage of static and shared library.
- Built-in resource management, including descriptors, command lists and temporary memory. You do not need to struggle with shader-visible descriptors and temporary upload buffers anymore.
- A simple but easy-to-use math library.
- Maybe more in the future.

### Motivation

This project came from my idea of writing a game engine, but later I gave up this idea when I found that game engine is such a huge project that I cannot finish it alone. So I decided to focus on the rendering part, and make it a library that can be used in other projects.

Another reason why I started this project is that I found that I've been spending a lot of time struggling with managing DirectX resources, like descriptors and temporary resources while learning D3D12. There are a lot of tutorials teaching you how to use those rendering APIs, but few of them talk about how to design the rendering system. I hope my project will give you some inspiration when you design your rendering system.

## Example Code

### Create Objects

Rendering objects could be created by `RenderDevice`:

```cpp
RenderDevice device;
Window window("HelloWorld", 800, 600, WindowStyle::Titled);
SwapChain swapChain(device.newSwapChain(window));
RootSignature rootSignature(device.newRootSignature(...));
GraphicsPipelineState pipelineState(device.newGraphicsPipeline(...));
CommandBuffer commandBuffer(device.newCommandBuffer());
StructuredBuffer vertexBuffer(device.newStructuredBuffer(...));
```

These resources are managed by RAII and you do not need to worry about resource lifetime.

Most rendering objects are movable for convenience except the `RenderDevice`. You can easily move them to `shared_ptr`, `unique_ptr` or other places.

Creating objects from `RenderDevice` is thread-safe, but for most other objects you should not share them between threads.

### Upload Resources

Here is an example of uploading vertex data to GPU buffer:

```cpp
commandBuffer.copyBuffer(VERTICES, vertexBuffer, 0, sizeof(VERTICES));
// Transition resource state if necessary.
commandBuffer.transition(vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
// Record other commands ...
```

Each command buffer has its own temporary upload buffer allocator. The `copyBuffer()` method will automatically allocate temporary upload buffer and cache the data in it. Therefore, the `VERTICES` data could be safely deleted once `copyBuffer()` finishes.

### Render Objects

Here is part of the HelloTriangle `update()` example:

```cpp
auto Application::update() -> void {
    auto &backBuffer = m_swapChain.backBuffer();

    RenderPass renderPass{};
    renderPass.renderTargetCount             = 1;
    renderPass.renderTargets[0].renderTarget = &backBuffer;
    renderPass.renderTargets[0].loadAction   = LoadAction::Clear;
    renderPass.renderTargets[0].storeAction  = StoreAction::Store;
    renderPass.renderTargets[0].stateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    renderPass.renderTargets[0].stateAfter   = D3D12_RESOURCE_STATE_PRESENT;

    m_commandBuffer.beginRenderPass(renderPass);
    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);
    m_commandBuffer.setPipelineState(m_pipelineState);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandBuffer.setVertexBuffer(0, m_vertexBuffer);

    auto [width, height] = m_window.frameSize();
    m_commandBuffer.setViewport(0, 0, width, height);
    m_commandBuffer.setScissorRect(0, 0, width, height);
    m_commandBuffer.draw(3);
    m_commandBuffer.endRenderPass();
    m_commandBuffer.submit();

    m_swapChain.present();
}
```

### Other Examples

More example projects are located in the `examples` directory. You can build them with CMake.

## Build

### Prerequisites

This library is developed on Windows 11 with Windows SDK 10.0.22000.0 and MSVC v143. It should works with Windows SDK 10.0.17763.0 and later versions and MSVC v142, but it is not tested.

To build with Visual Studio, you need:

- CMake
- Visual Studio 2019 or later
- Windows SDK 10.0.17763.0 or later. DirectX shader compiler(dxc) should be included.
- MSVC v142 or later

To build with clang-cl, you also need to install LLVM support and clang build tools for Visual Studio, and prepare with the following command:

```PowerShell
cmake -T ClangCl ...
```

#### LLVM

This project could also be built with LLVM and ninja. You still need Windows SDK and MSVC to build the project. You will also need the following tools:

- LLVM (Any version with full C++17 support)
- Ninja

#### MinGW

Both MinGW and LLVM-MinGW are not supported, because this project requires Concurrency Runtime.

### Build Options

This project could be built with CMake. Here are some options you may need:

- `INK_BUILD_STATIC_LIBS`: Specifies whether to build static libraries. Default is `ON`. The static library is the default target if both the static library and shared library are enabled.
- `INK_BUILD_SHARED_LIBS`: Specifies whether to build shared libraries. Default is `ON`.
- `INK_BUILD_EXAMPLES`: Specifies whether to build examples. Default is `OFF`.
- `INK_BUILD_TESTS`: Specifies whether to build unit tests. Default is `ON`.

### Integration

You can simply use this project as a CMake sub-project.

There are 3 CMake targets in this project:

- `ink::ink`: The default target. If both static and shared libraries are enabled, this target will be the static library.
- `ink::shared`: The shared library target. This target is not defined if shared library is disabled.
- `ink::static`: The static library target. This target is not defined if static library is disabled.

## License

This project is under MIT license. See [LICENSE](LICENSE) for more information.

## Third-Party Libraries

The ink library itself does not depend on any third-party libraries. The examples and tests depend on some third-party libraries:

- [Catch2](https://github.com/catchorg/Catch2): A modern, C++-native, test framework for unit-tests, TDD and BDD - using C++14, C++17 and later. BSL License.
- [DirectXTex](https://github.com/microsoft/DirectXTex): DirectXTex texture processing library. MIT License.
- [tinygltf](https://github.com/syoyo/tinygltf): Header only C++11 tiny glTF 2.0 library. MIT License.

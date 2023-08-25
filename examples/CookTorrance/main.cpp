#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/math/numbers.hpp"
#include "ink/math/quaternion.hpp"
#include "ink/render/device.hpp"

#include <tiny_gltf.h>

// Generated headers.
#include "cook_torrance.ps.hlsl.hpp"
#include "cook_torrance.rootsig.hlsl.hpp"
#include "cook_torrance.vs.hlsl.hpp"

using namespace ink;

namespace {

struct Transform {
    Matrix4 model;
    Matrix4 view;
    Matrix4 projection;
    Vector3 cameraPos;
};

struct Light {
    Vector3                position;
    [[maybe_unused]] float padding;
    Vector3                color;
    [[maybe_unused]] float padding1;
};

struct Material {
    float   roughness;
    Vector3 F0;
};

class Camera {
public:
    Camera(float fov, float aspectRatio, float zNear = 0.1f, float zFar = 1000.0f) noexcept
        : m_isViewDirty(true),
          m_isProjectionDirty(true),
          m_position(),
          m_rotation(),
          m_fovY(fov),
          m_aspectRatio(aspectRatio),
          m_zNear(zNear),
          m_zFar(zFar),
          m_view(),
          m_projection() {}

    [[nodiscard]] auto position() const noexcept -> Vector3 { return m_position; }

    auto setPosition(Vector3 position) noexcept -> void {
        m_position    = position;
        m_isViewDirty = true;
    }

    [[nodiscard]] auto rotation() const noexcept -> Quaternion {
        return {m_rotation.x, m_rotation.y, m_rotation.z};
    }

    auto rotate(float pitch, float yaw, float roll) noexcept -> void {
        m_rotation.x += pitch;
        m_rotation.y += yaw;
        m_rotation.z += roll;
        m_rotation.x  = std::fmodf(m_rotation.x, Pi<float> * 2.0f);
        m_rotation.y  = std::fmodf(m_rotation.y, Pi<float> * 2.0f);
        m_rotation.z  = std::fmodf(m_rotation.z, Pi<float> * 2.0f);
        m_isViewDirty = true;
    }

    [[nodiscard]] auto fieldOfView() const noexcept -> float { return m_fovY; }

    auto setFieldOfView(float fov) noexcept -> void {
        m_fovY              = fov;
        m_isProjectionDirty = true;
    }

    [[nodiscard]] auto aspectRatio() const noexcept -> float { return m_aspectRatio; }

    auto setAspectRatio(float aspectRatio) noexcept -> void {
        m_aspectRatio       = aspectRatio;
        m_isProjectionDirty = true;
    }

    [[nodiscard]] auto zNear() const noexcept -> float { return m_zNear; }

    auto setZNear(float zNear) noexcept -> void {
        m_zNear             = zNear;
        m_isProjectionDirty = true;
    }

    [[nodiscard]] auto zFar() const noexcept -> float { return m_zFar; }

    auto setZFar(float zFar) noexcept -> void {
        m_zFar              = zFar;
        m_isProjectionDirty = true;
    }

    [[nodiscard]] auto front() const noexcept -> Vector3 {
        Quaternion quat{m_rotation.x, m_rotation.y, m_rotation.z};
        Quaternion forward{0.0f, 0.0f, 0.0f, 1.0f};
        forward = quat * forward * quat.conjugated();
        return {forward.x, forward.y, forward.z};
    }

    auto move(float x, float y, float z) noexcept -> void {
        Vector3 front = this->front();
        Vector3 right = cross({0.0f, 1.0f, 0.0f}, front).normalized();
        Vector3 up    = cross(front, right).normalized();

        m_position += right * x;
        m_position += up * y;
        m_position += front * z;
        m_isViewDirty = true;
    }

    [[nodiscard]] auto view() const noexcept -> const Matrix4 & {
        if (m_isViewDirty) {
            m_view        = lookTo(m_position, front(), {0.0f, 1.0f, 0.0f});
            m_isViewDirty = false;
        }
        return m_view;
    }

    [[nodiscard]] auto projection() const noexcept -> const Matrix4 & {
        if (m_isProjectionDirty) {
            m_projection        = perspective(m_fovY, m_aspectRatio, m_zNear, m_zFar);
            m_isProjectionDirty = false;
        }
        return m_projection;
    }

private:
    mutable bool m_isViewDirty;
    mutable bool m_isProjectionDirty;

    Vector3 m_position;
    Vector3 m_rotation;

    float m_fovY;
    float m_aspectRatio;
    float m_zNear;
    float m_zFar;

    mutable Matrix4 m_view;
    mutable Matrix4 m_projection;
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
};

class Mesh;

struct SubMesh {
    std::size_t   m_positionBufferID;
    std::uint64_t m_positionBufferOffset;
    std::uint32_t m_positionCount;
    std::uint32_t m_positionStride;
    std::size_t   m_normalBufferID;
    std::uint64_t m_normalBufferOffset;
    std::uint32_t m_normalCount;
    std::uint32_t m_normalStride;
    std::size_t   m_indexBufferID;
    std::uint64_t m_indexBufferOffset;
    std::uint32_t m_indexCount;
    std::uint32_t m_indexStride;
    Matrix4       m_modelMatrix;
};

struct Mesh {
    Mesh(RenderDevice &renderDevice, std::string_view path);

    std::vector<GpuBuffer> m_gpuBuffers;
    std::vector<SubMesh>   m_nodes;
};

Mesh::Mesh(RenderDevice &renderDevice, std::string_view path) : m_gpuBuffers(), m_nodes() {
    tinygltf::Model    gltfModel;
    tinygltf::TinyGLTF gltfContext;
    gltfContext.SetImageLoader(
        +[](tinygltf::Image *, const int, std::string *, std::string *, int, int,
            const unsigned char *, int, void *) -> bool { return true; },
        nullptr);

    { // Load glTF file.
        std::string error;

        bool fileLoaded =
            gltfContext.LoadASCIIFromFile(&gltfModel, &error, nullptr, "asset/sphere.gltf");
        if (!fileLoaded)
            throw Exception("Failed to load glTF file: " + error);
    }

    // Upload buffers.
    CommandBuffer cmdBuffer = renderDevice.newCommandBuffer();
    for (const auto &buffer : gltfModel.buffers) {
        GpuBuffer newBuffer = renderDevice.newGpuBuffer(buffer.data.size());
        cmdBuffer.copyBuffer(buffer.data.data(), newBuffer, 0, buffer.data.size());
        cmdBuffer.transition(newBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_gpuBuffers.push_back(std::move(newBuffer));
    }
    cmdBuffer.submit();
    cmdBuffer.waitForComplete();

    // Load scene.
    const tinygltf::Scene &scene =
        gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

    // BFS.
    std::queue<std::pair<Matrix4, int>> nodes;
    for (auto i : scene.nodes)
        nodes.emplace(Matrix4{1.0f}, i);

    while (!nodes.empty()) {
        auto [transform, nodeIndex] = nodes.front();
        nodes.pop();

        const tinygltf::Node node = gltfModel.nodes[nodeIndex];

        Vector3    translation{};
        Vector3    scale{1.0f};
        Quaternion rotation{1.0f};

        if (node.translation.size() == 3)
            translation = Vector3{
                static_cast<float>(node.translation[0]),
                static_cast<float>(node.translation[1]),
                static_cast<float>(node.translation[2]),
            };

        if (node.rotation.size() == 4)
            rotation = Quaternion{
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2]),
                static_cast<float>(node.rotation[3]),
            };

        if (node.scale.size() == 3)
            scale = Vector3{
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2]),
            };

        transform =
            Matrix4(1.0f).scaled(scale).rotated(rotation).translated(translation) * transform;

        // Add children.
        for (auto i : node.children)
            nodes.emplace(transform, i);

        // This node does not have mesh.
        if (node.mesh < 0)
            continue;

        const tinygltf::Mesh mesh = gltfModel.meshes[node.mesh];
        for (const auto &primitive : mesh.primitives) {
            if (primitive.indices < 0)
                continue;

            SubMesh submesh{};
            submesh.m_modelMatrix = transform;

            auto &attributes = primitive.attributes;

            { // Get vertices.
                assert(attributes.find("POSITION") != attributes.end());

                auto        attr       = attributes.find("POSITION");
                const auto &accessor   = gltfModel.accessors[attr->second];
                const auto &bufferView = gltfModel.bufferViews[accessor.bufferView];

                submesh.m_positionBufferID     = static_cast<std::size_t>(bufferView.buffer);
                submesh.m_positionBufferOffset = accessor.byteOffset + bufferView.byteOffset;
                submesh.m_positionCount        = static_cast<std::uint32_t>(accessor.count);
                submesh.m_positionStride =
                    static_cast<std::uint32_t>(accessor.ByteStride(bufferView));
            }

            { // Get normals.
                assert(attributes.find("NORMAL") != attributes.end());

                auto        attr       = attributes.find("NORMAL");
                const auto &accessor   = gltfModel.accessors[attr->second];
                const auto &bufferView = gltfModel.bufferViews[accessor.bufferView];

                submesh.m_normalBufferID     = static_cast<std::size_t>(bufferView.buffer);
                submesh.m_normalBufferOffset = accessor.byteOffset + bufferView.byteOffset;
                submesh.m_normalCount        = static_cast<std::uint32_t>(accessor.count);
                submesh.m_normalStride =
                    static_cast<std::uint32_t>(accessor.ByteStride(bufferView));
            }

            { // Get indices.
                const auto &accessor   = gltfModel.accessors[primitive.indices];
                const auto &bufferView = gltfModel.bufferViews[accessor.bufferView];
                const auto &buffer     = gltfModel.buffers[bufferView.buffer];

                submesh.m_indexBufferID     = static_cast<std::size_t>(bufferView.buffer);
                submesh.m_indexCount        = static_cast<std::uint32_t>(accessor.count);
                submesh.m_indexBufferOffset = accessor.byteOffset + bufferView.byteOffset;

                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                    submesh.m_indexStride = sizeof(std::uint32_t);
                    break;

                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                    submesh.m_indexStride = sizeof(std::uint16_t);
                    break;

                default:
                    throw Exception("Unsupported index type.");
                }
            }

            m_nodes.push_back(submesh);
        }
    }
}

class Application {
public:
    Application();

    auto run() -> void;
    auto update(float deltaTime) -> void;

private:
    RenderDevice          m_renderDevice;
    Window                m_window;
    SwapChain             m_swapChain;
    DepthBuffer           m_depthBuffer;
    CommandBuffer         m_commandBuffer;
    RootSignature         m_rootSignature;
    GraphicsPipelineState m_pipelineState;

    Camera m_camera;

    std::vector<Material> m_materials;
    Mesh                  m_mesh;
};

constexpr const D3D12_INPUT_ELEMENT_DESC INPUT_ELEMENTS[]{
    D3D12_INPUT_ELEMENT_DESC{
        /* SemanticName         = */ "POSITION",
        /* SemanticIndex        = */ 0,
        /* Format               = */ DXGI_FORMAT_R32G32B32_FLOAT,
        /* InputSlot            = */ 0,
        /* AlignedByteOffset    = */ 0,
        /* InputSlotClass       = */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        /* InstanceDataStepRate = */ 0,
    },
    D3D12_INPUT_ELEMENT_DESC{
        /* SemanticName         = */ "NORMAL",
        /* SemanticIndex        = */ 0,
        /* Format               = */ DXGI_FORMAT_R32G32B32_FLOAT,
        /* InputSlot            = */ 1,
        /* AlignedByteOffset    = */ 0,
        /* InputSlotClass       = */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        /* InstanceDataStepRate = */ 0,
    },
};

Application::Application()
    : m_renderDevice(),
      m_window("Cook-Torrance", 1280, 720, WindowStyle::Titled | WindowStyle::Resizable),
      m_swapChain(m_renderDevice.newSwapChain(m_window)),
      m_depthBuffer(m_renderDevice.newDepthBuffer(1280, 720, DXGI_FORMAT_D32_FLOAT)),
      m_commandBuffer(m_renderDevice.newCommandBuffer()),
      m_rootSignature(m_renderDevice.newRootSignature(g_rootsig, sizeof(g_rootsig))),
      m_pipelineState(),
      m_camera(Pi<float> / 3.0f, 1280.0f / 720.0f),
      m_materials(),
      m_mesh(m_renderDevice, "asset/sphere.gltf") {
    m_window.setFrameResizeCallback([this](Window &, std::uint32_t width, std::uint32_t height) {
        if ((width * height) == 0)
            return;

        m_swapChain.resize({width, height});
        m_depthBuffer = m_renderDevice.newDepthBuffer(width, height, DXGI_FORMAT_D32_FLOAT);
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    });

    { // Create graphics pipeline state.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

        desc.pRootSignature                        = m_rootSignature.rootSignature();
        desc.VS                                    = {g_vertex_main, sizeof(g_vertex_main)};
        desc.PS                                    = {g_pixel_main, sizeof(g_pixel_main)};
        desc.GS                                    = {};
        desc.InputLayout.pInputElementDescs        = INPUT_ELEMENTS;
        desc.InputLayout.NumElements               = static_cast<UINT>(std::size(INPUT_ELEMENTS));
        desc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
        desc.RasterizerState.CullMode              = D3D12_CULL_MODE_NONE;
        desc.RasterizerState.FrontCounterClockwise = FALSE;
        desc.RasterizerState.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
        desc.RasterizerState.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        desc.RasterizerState.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        desc.RasterizerState.DepthClipEnable       = TRUE;
        desc.RasterizerState.MultisampleEnable     = FALSE;
        desc.RasterizerState.AntialiasedLineEnable = FALSE;
        desc.RasterizerState.ForcedSampleCount     = 0;
        desc.RasterizerState.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        desc.BlendState.AlphaToCoverageEnable      = FALSE;
        desc.BlendState.IndependentBlendEnable     = FALSE;

        for (auto &renderTarget : desc.BlendState.RenderTarget)
            renderTarget = D3D12_RENDER_TARGET_BLEND_DESC{
                /* BlendEnable           = */ FALSE,
                /* LogicOpEnable         = */ FALSE,
                /* SrcBlend              = */ D3D12_BLEND_ONE,
                /* DestBlend             = */ D3D12_BLEND_ZERO,
                /* BlendOp               = */ D3D12_BLEND_OP_ADD,
                /* SrcBlendAlpha         = */ D3D12_BLEND_ONE,
                /* DestBlendAlpha        = */ D3D12_BLEND_ZERO,
                /* BlendOpAlpha          = */ D3D12_BLEND_OP_ADD,
                /* LogicOp               = */ D3D12_LOGIC_OP_NOOP,
                /* RenderTargetWriteMask = */ D3D12_COLOR_WRITE_ENABLE_ALL,
            };

        desc.DepthStencilState.DepthEnable      = TRUE;
        desc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
        desc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        desc.DepthStencilState.StencilEnable    = FALSE;
        desc.DepthStencilState.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
        desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        desc.SampleMask                         = UINT_MAX;
        desc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets                   = 1U;
        desc.RTVFormats[0]                      = m_swapChain.pixelFormat();
        desc.DSVFormat                          = m_depthBuffer.pixelFormat();
        desc.SampleDesc.Count                   = 1;

        m_pipelineState = m_renderDevice.newGraphicsPipeline(desc);
    }

    m_materials.push_back({0.1f, {0.56f, 0.57f, 0.58f}}); // iron
    m_materials.push_back({0.1f, {0.95f, 0.64f, 0.54f}}); // copper
    m_materials.push_back({0.1f, {0.91f, 0.92f, 0.92f}}); // aluminum
    m_materials.push_back({0.1f, {0.95f, 0.93f, 0.88f}}); // silver
    m_materials.push_back({0.1f, {1.00f, 0.71f, 0.29f}}); // gold

    m_camera.setPosition({0.0f, 0.0f, -4.5f});
    m_swapChain.setClearColor(colors::Black);
}

auto Application::run() -> void {
    LARGE_INTEGER now, lastUpdate, countsPerSec, lastUpdateFps;
    QueryPerformanceFrequency(&countsPerSec);
    QueryPerformanceCounter(&now);
    const double timeCoef = 1.0 / static_cast<double>(countsPerSec.QuadPart);
    lastUpdate            = now;
    lastUpdateFps         = now;

    MSG msg{};
    while (!m_window.isClosed()) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        } else {
            QueryPerformanceCounter(&now);
            double deltaTime = static_cast<double>(now.QuadPart - lastUpdate.QuadPart) * timeCoef;
            this->update(static_cast<float>(deltaTime));

            if (now.QuadPart - lastUpdateFps.QuadPart > countsPerSec.QuadPart) {
                const float fps = static_cast<float>(countsPerSec.QuadPart) /
                                  static_cast<float>(now.QuadPart - lastUpdate.QuadPart);
                std::string newTitle("CookTorrance - ");
                newTitle.append(std::to_string(fps));
                newTitle.append(" FPS");
                m_window.title(newTitle);

                lastUpdateFps = now;
            }

            lastUpdate = now;
        }
    }
}

auto Application::update(float deltaTime) -> void {
    if (isKeyPressed(KeyCode::W))
        m_camera.move(0, 0, deltaTime);
    if (isKeyPressed(KeyCode::A))
        m_camera.move(-deltaTime, 0, 0);
    if (isKeyPressed(KeyCode::S))
        m_camera.move(0, 0, -deltaTime);
    if (isKeyPressed(KeyCode::D))
        m_camera.move(deltaTime, 0, 0);

    auto &backBuffer = m_swapChain.backBuffer();

    RenderPass renderPass{};
    renderPass.renderTargetCount              = 1;
    renderPass.renderTargets[0].renderTarget  = &backBuffer;
    renderPass.renderTargets[0].loadAction    = LoadAction::Clear;
    renderPass.renderTargets[0].storeAction   = StoreAction::Store;
    renderPass.renderTargets[0].stateBefore   = D3D12_RESOURCE_STATE_RENDER_TARGET;
    renderPass.renderTargets[0].stateAfter    = D3D12_RESOURCE_STATE_PRESENT;
    renderPass.depthTarget.depthTarget        = &m_depthBuffer;
    renderPass.depthTarget.depthLoadAction    = LoadAction::Clear;
    renderPass.depthTarget.depthStoreAction   = StoreAction::Store;
    renderPass.depthTarget.stencilLoadAction  = LoadAction::DontCare;
    renderPass.depthTarget.stencilStoreAction = StoreAction::DontCare;
    renderPass.depthTarget.stateBefore        = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    renderPass.depthTarget.stateAfter         = D3D12_RESOURCE_STATE_DEPTH_READ;

    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);
    m_commandBuffer.setPipelineState(m_pipelineState);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::array<Light, 4> lights{
        Light{
            {4.076245307922363f, 5.903861999511719f, -1.0054539442062378f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            0.0f,
        },
        Light{
            {4.076245307922363f, -5.903861999511719f, -1.0054539442062378f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            0.0f,
        },
        Light{
            {-4.076245307922363f, 5.903861999511719f, -1.0054539442062378f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            0.0f,
        },
        Light{
            {-4.076245307922363f, -5.903861999511719f, -1.0054539442062378f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            0.0f,
        },
    };

    for (auto &submesh : m_mesh.m_nodes) {
        m_commandBuffer.beginRenderPass(renderPass);
        m_commandBuffer.setVertexBuffer(
            0,
            m_mesh.m_gpuBuffers[submesh.m_positionBufferID].gpuAddress() +
                submesh.m_positionBufferOffset,
            submesh.m_positionCount, submesh.m_positionStride);
        m_commandBuffer.setVertexBuffer(1,
                                        m_mesh.m_gpuBuffers[submesh.m_normalBufferID].gpuAddress() +
                                            submesh.m_normalBufferOffset,
                                        submesh.m_normalCount, submesh.m_normalStride);
        m_commandBuffer.setIndexBuffer(m_mesh.m_gpuBuffers[submesh.m_indexBufferID].gpuAddress() +
                                           submesh.m_indexBufferOffset,
                                       submesh.m_indexCount, submesh.m_indexStride);

        // Set object position.
        m_commandBuffer.setGraphicsConstant(0, 0, 0.0f);
        m_commandBuffer.setGraphicsConstant(0, 1, 0.0f);
        m_commandBuffer.setGraphicsConstant(0, 2, 0.0f);

        Transform transform{
            /* model      = */ submesh.m_modelMatrix,
            /* view       = */ m_camera.view(),
            /* projection = */ m_camera.projection(),
            /* cameraPos  = */ m_camera.position(),
        };

        m_commandBuffer.setGraphicsConstantBuffer(1, 0, &transform, sizeof(transform));
        m_commandBuffer.setGraphicsConstantBuffer(1, 1, &lights, sizeof(lights));
        m_commandBuffer.setGraphicsConstantBuffer(1, 2, &m_materials[0], sizeof(Material));

        auto [width, height] = m_window.frameSize();
        m_commandBuffer.setViewport(0, 0, width, height);
        m_commandBuffer.setScissorRect(0, 0, width, height);

        m_commandBuffer.drawIndexed(submesh.m_indexCount, 0);
        m_commandBuffer.endRenderPass();

        renderPass.depthTarget.depthLoadAction = LoadAction::Load;
    }

    m_commandBuffer.submit();
    m_swapChain.present();
}

} // namespace

auto main() -> int {
    try {
        Application app;
        app.run();
    } catch (const Exception &e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
}
#include "ink/camera.hpp"
#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/math/numbers.hpp"
#include "ink/math/quaternion.hpp"
#include "ink/model.hpp"
#include "ink/render/device.hpp"

// Generated headers.
#include "cook_torrance.ps.hlsl.hpp"
#include "cook_torrance.rootsig.hlsl.hpp"
#include "cook_torrance.vs.hlsl.hpp"

using namespace ink;

namespace {

struct Transform {
    Matrix4 model;
    Matrix4 modelInvTranspose;
    Matrix4 view;
    Matrix4 projection;
    Vector3 cameraPos;
};

struct Lights {
    std::uint32_t numLights;
    Vector4       lightPositions[16];
    Vector4       lightColors[16];
};

struct Material {
    Vector3 emissive;
    Vector4 baseColor;
};

class Application {
public:
    Application();

    auto run() -> void;
    auto update(float deltaTime) -> void;

private:
    auto updateCamera(float deltaTime) -> void;

private:
    RenderDevice          m_renderDevice;
    Window                m_window;
    SwapChain             m_swapChain;
    DepthBuffer           m_depthBuffer;
    CommandBuffer         m_commandBuffer;
    RootSignature         m_rootSignature;
    GraphicsPipelineState m_pipelineState;

    Camera  m_camera;
    Model   m_model;
    Sampler m_linearSampler;

    std::int32_t m_cursorLastX;
    std::int32_t m_cursorLastY;
    std::int32_t m_cursorX;
    std::int32_t m_cursorY;
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
    D3D12_INPUT_ELEMENT_DESC{
        /* SemanticName         = */ "TEXCOORD",
        /* SemanticIndex        = */ 0,
        /* Format               = */ DXGI_FORMAT_R32G32_FLOAT,
        /* InputSlot            = */ 2,
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
      m_camera({0.0f, 0.0f, -4.0f}, Pi<float> / 3.0f, 1280.0f / 720.0f),
      m_model(m_renderDevice, "asset/DamagedHelmet.glb", true),
      m_linearSampler(m_renderDevice.newSampler(D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
                                                D3D12_TEXTURE_ADDRESS_MODE_WRAP)),
      m_cursorLastX(0),
      m_cursorLastY(0),
      m_cursorX(0),
      m_cursorY(0) {
    m_window.setFrameResizeCallback([this](Window &, std::uint32_t width, std::uint32_t height) {
        if ((width * height) == 0)
            return;

        m_swapChain.resize({width, height});
        m_depthBuffer = m_renderDevice.newDepthBuffer(width, height, DXGI_FORMAT_D32_FLOAT);
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    });

    m_window.setKeyCallback([this](Window &window, KeyCode key, KeyAction action, Modifier) {
        if (key == KeyCode::Escape && action == KeyAction::Press)
            window.close();
    });

    m_window.setMousePositionCallback([this](Window &, std::int32_t x, std::int32_t y) -> void {
        m_cursorLastX = m_cursorX;
        m_cursorLastY = m_cursorY;
        m_cursorX     = x;
        m_cursorY     = y;
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
        desc.RasterizerState.CullMode              = D3D12_CULL_MODE_BACK;
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

auto Application::updateCamera(float deltaTime) -> void {
    if (isKeyPressed(KeyCode::W))
        m_camera.translate({0, 0, deltaTime});
    if (isKeyPressed(KeyCode::A))
        m_camera.translate({-deltaTime, 0, 0});
    if (isKeyPressed(KeyCode::S))
        m_camera.translate({0, 0, -deltaTime});
    if (isKeyPressed(KeyCode::D))
        m_camera.translate({deltaTime, 0, 0});
    if (isKeyPressed(KeyCode::MouseLeft)) {
        m_camera.rotate(static_cast<float>(m_cursorY - m_cursorLastY) * deltaTime,
                        static_cast<float>(m_cursorX - m_cursorLastX) * deltaTime, 0.0f);
        m_cursorLastX = m_cursorX;
        m_cursorLastY = m_cursorY;
    }
}

auto Application::update(float deltaTime) -> void {
    this->updateCamera(deltaTime);
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

    Lights lights{};
    lights.numLights         = 4;
    lights.lightPositions[0] = {4.076245307922363f, 5.903861999511719f, -1.0054539442062378f, 1.0f};
    lights.lightPositions[1] = {4.076245307922363f, -5.903861999511719f, -1.0054539442062378f,
                                1.0f};
    lights.lightPositions[2] = {-4.076245307922363f, 5.903861999511719f, -1.0054539442062378f,
                                1.0f};
    lights.lightPositions[3] = {-4.076245307922363f, -5.903861999511719f, -1.0054539442062378f,
                                1.0f};
    lights.lightColors[0]    = {1.0f, 1.0f, 1.0f, 1.0f};
    lights.lightColors[1]    = {1.0f, 1.0f, 1.0f, 1.0f};
    lights.lightColors[2]    = {1.0f, 1.0f, 1.0f, 1.0f};
    lights.lightColors[3]    = {1.0f, 1.0f, 1.0f, 1.0f};

    m_model.render([this, &renderPass, &lights](const Mesh &mesh, const Matrix4 &modelTransform) {
        m_commandBuffer.beginRenderPass(renderPass);
        m_commandBuffer.setVertexBuffer(0, mesh.position.buffer, mesh.position.count,
                                        mesh.position.stride);
        m_commandBuffer.setVertexBuffer(1, mesh.normal.buffer, mesh.normal.count,
                                        mesh.normal.stride);
        m_commandBuffer.setIndexBuffer(mesh.index.buffer, mesh.index.count, mesh.index.stride);

        Transform transform{
            /* model             = */ modelTransform,
            /* modelInvTranspose = */ modelTransform.inversed().transposed(),
            /* view              = */ m_camera.viewMatrix(),
            /* projection        = */ m_camera.projectionMatrix(),
            /* cameraPos         = */ m_camera.position(),
        };

        m_commandBuffer.setGraphicsConstantBuffer(0, 0, &transform, sizeof(transform));

        Material material{
            /* emissive  = */ mesh.material->emissiveFactor,
            /* baseColor = */ mesh.material->baseColor,
        };

        m_commandBuffer.setGraphicsConstantBuffer(0, 1, &material, sizeof(material));
        m_commandBuffer.setGraphicsConstantBuffer(0, 2, &lights, sizeof(lights));

        m_commandBuffer.setGraphicsDescriptor(
            0, 3, mesh.material->baseColorTexture->shaderResourceView());
        m_commandBuffer.setGraphicsDescriptor(
            0, 4, mesh.material->metallicRoughnessTexture->shaderResourceView());
        m_commandBuffer.setGraphicsDescriptor(0, 5,
                                              mesh.material->normalTexture->shaderResourceView());
        m_commandBuffer.setGraphicsDescriptor(0, 6,
                                              mesh.material->emissiveTexture->shaderResourceView());

        m_commandBuffer.setGraphicsDescriptor(1, 0, m_linearSampler.descriptor());

        auto [width, height] = m_window.frameSize();
        m_commandBuffer.setViewport(0, 0, width, height);
        m_commandBuffer.setScissorRect(0, 0, width, height);

        m_commandBuffer.drawIndexed(mesh.index.count, 0);
        m_commandBuffer.endRenderPass();
    });

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
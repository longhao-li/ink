#include "ink/camera.hpp"
#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/math/numbers.hpp"
#include "ink/model.hpp"
#include "ink/render/device.hpp"
#include "shader.hpp"

using namespace ink;

namespace {

struct Transform {
    Matrix4 model;
    Matrix4 modelInvTranspose;
    Matrix4 view;
    Matrix4 projection;
    Vector3 cameraPos;
};

struct PointLight {
    Vector4 position;
    Vector4 color;
};

struct SpotLight {
    Vector4 position;
    Vector4 direction;
    Vector4 color;
    float   spot;
};

struct DirectionalLight {
    Vector4 direction;
    Vector4 color;
};

struct Material {
    Vector4 color;
    float   metallic;
    float   roughness;
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
    GraphicsPipelineState m_pointLightPipeline;
    GraphicsPipelineState m_spotLightPipeline;
    GraphicsPipelineState m_directionalLightPipeline;

    Camera m_camera;
    Model  m_model;

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
};

constexpr D3D12_RENDER_TARGET_BLEND_DESC DEFAULT_RENDER_TARGET_BLEND_DESC{
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

constexpr D3D12_RASTERIZER_DESC DEFAULT_D3D12_RASTERIZER_DESC{
    /* FillMode              = */ D3D12_FILL_MODE_SOLID,
    /* CullMode              = */ D3D12_CULL_MODE_BACK,
    /* FrontCounterClockwise = */ FALSE,
    /* DepthBias             = */ D3D12_DEFAULT_DEPTH_BIAS,
    /* DepthBiasClamp        = */ D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
    /* SlopeScaledDepthBias  = */ D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
    /* DepthClipEnable       = */ TRUE,
    /* MultisampleEnable     = */ FALSE,
    /* AntialiasedLineEnable = */ FALSE,
    /* ForcedSampleCount     = */ 0,
    /* ConservativeRaster    = */ D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
};

constexpr D3D12_DEPTH_STENCIL_DESC DEFAULT_D3D12_DEPTH_STENCIL_DESC{
    /* DepthEnable      = */ TRUE,
    /* DepthWriteMask   = */ D3D12_DEPTH_WRITE_MASK_ALL,
    /* DepthFunc        = */ D3D12_COMPARISON_FUNC_LESS,
    /* StencilEnable    = */ FALSE,
    /* StencilReadMask  = */ D3D12_DEFAULT_STENCIL_READ_MASK,
    /* StencilWriteMask = */ D3D12_DEFAULT_STENCIL_WRITE_MASK,
    /* FrontFace        = */
    {
        /* StencilFailOp      = */ D3D12_STENCIL_OP_KEEP,
        /* StencilDepthFailOp = */ D3D12_STENCIL_OP_KEEP,
        /* StencilPassOp      = */ D3D12_STENCIL_OP_KEEP,
        /* StencilFunc        = */ D3D12_COMPARISON_FUNC_ALWAYS,
    },
    /* BackFace         = */
    {
        /* StencilFailOp      = */ D3D12_STENCIL_OP_KEEP,
        /* StencilDepthFailOp = */ D3D12_STENCIL_OP_KEEP,
        /* StencilPassOp      = */ D3D12_STENCIL_OP_KEEP,
        /* StencilFunc        = */ D3D12_COMPARISON_FUNC_ALWAYS,
    },
};

constexpr D3D12_GRAPHICS_PIPELINE_STATE_DESC DEFAULT_GRAPHICS_PIPELINE_STATE_DESC{
    /* pRootSignature = */ nullptr,
    /* VS             = */ {nullptr, 0},
    /* PS             = */ {nullptr, 0},
    /* DS             = */ {nullptr, 0},
    /* HS             = */ {nullptr, 0},
    /* GS             = */ {nullptr, 0},
    /* StreamOutput   = */
    {
        /* pSODeclaration   = */ nullptr,
        /* NumEntries       = */ 0,
        /* pBufferStrides   = */ nullptr,
        /* NumStrides       = */ 0,
        /* RasterizedStream = */ 0,
    },
    /* BlendState = */
    {
        /* AlphaToCoverageEnable  = */ FALSE,
        /* IndependentBlendEnable = */ FALSE,
        /* RenderTarget           = */
        {
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
            DEFAULT_RENDER_TARGET_BLEND_DESC,
        },
    },
    /* SampleMask                 = */ UINT_MAX,
    /* RasterizerState            = */ DEFAULT_D3D12_RASTERIZER_DESC,
    /* DepthStencilState          = */ DEFAULT_D3D12_DEPTH_STENCIL_DESC,
    /* InputLayout                = */ {},
    /* IBStripCutValue            = */ D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
    /* PrimitiveTopologyType      = */ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
    /* NumRenderTargets           = */ 1,
    /* RTVFormats                 = */ {DXGI_FORMAT_R8G8B8A8_UNORM},
    /* DSVFormat                  = */ DXGI_FORMAT_D32_FLOAT,
    /* SampleDesc                 = */ {1, 0},
    /* NodeMask                   = */ 0,
    /* CachedPSO                  = */ {},
    /* Flags                      = */ D3D12_PIPELINE_STATE_FLAG_NONE,
};

Application::Application()
    : m_renderDevice(),
      m_window("Boxes", 1280, 720),
      m_swapChain(m_renderDevice.newSwapChain(m_window)),
      m_depthBuffer(m_renderDevice.newDepthBuffer(1280, 720, DXGI_FORMAT_D32_FLOAT)),
      m_commandBuffer(m_renderDevice.newCommandBuffer()),
      m_rootSignature(m_renderDevice.newRootSignature(RootSig.data, RootSig.size)),
      m_pointLightPipeline(),
      m_spotLightPipeline(),
      m_directionalLightPipeline(),
      m_camera({0.0f, 0.0f, -4.0f}, Pi<float> / 3.0f, 1280.0f / 720.0f),
      m_model(m_renderDevice, 1.0f, 1.0f, 1.0f, {}),
      m_cursorLastX(0),
      m_cursorLastY(0),
      m_cursorX(0),
      m_cursorY(0) {
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

    { // Create point light pipeline.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{DEFAULT_GRAPHICS_PIPELINE_STATE_DESC};
        D3D12_RENDER_TARGET_BLEND_DESC     blend = DEFAULT_RENDER_TARGET_BLEND_DESC;
        blend.BlendEnable                        = TRUE;
        blend.SrcBlend                           = D3D12_BLEND_ONE;
        blend.DestBlend                          = D3D12_BLEND_ONE;
        blend.BlendOp                            = D3D12_BLEND_OP_ADD;

        desc.pRootSignature = m_rootSignature.rootSignature();
        desc.VS             = {VertexShader.data, VertexShader.size};
        desc.PS             = {PointLightPixelShader.data, PointLightPixelShader.size};
        desc.InputLayout    = {INPUT_ELEMENTS, std::size(INPUT_ELEMENTS)};
        desc.BlendState = {FALSE, FALSE, {blend, blend, blend, blend, blend, blend, blend, blend}};
        m_pointLightPipeline = m_renderDevice.newGraphicsPipeline(desc);
    }

    { // Create spot light pipeline.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{DEFAULT_GRAPHICS_PIPELINE_STATE_DESC};
        D3D12_RENDER_TARGET_BLEND_DESC     blend = DEFAULT_RENDER_TARGET_BLEND_DESC;
        blend.BlendEnable                        = TRUE;
        blend.SrcBlend                           = D3D12_BLEND_ONE;
        blend.DestBlend                          = D3D12_BLEND_ONE;
        blend.BlendOp                            = D3D12_BLEND_OP_ADD;

        desc.pRootSignature = m_rootSignature.rootSignature();
        desc.VS             = {VertexShader.data, VertexShader.size};
        desc.PS             = {SpotLightPixelShader.data, SpotLightPixelShader.size};
        desc.InputLayout    = {INPUT_ELEMENTS, std::size(INPUT_ELEMENTS)};
        desc.BlendState = {FALSE, FALSE, {blend, blend, blend, blend, blend, blend, blend, blend}};
        m_spotLightPipeline = m_renderDevice.newGraphicsPipeline(desc);
    }

    { // Create directional light pipeline.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{DEFAULT_GRAPHICS_PIPELINE_STATE_DESC};
        D3D12_RENDER_TARGET_BLEND_DESC     blend = DEFAULT_RENDER_TARGET_BLEND_DESC;
        blend.BlendEnable                        = TRUE;
        blend.SrcBlend                           = D3D12_BLEND_ONE;
        blend.DestBlend                          = D3D12_BLEND_ONE;
        blend.BlendOp                            = D3D12_BLEND_OP_ADD;

        desc.pRootSignature = m_rootSignature.rootSignature();
        desc.VS             = {VertexShader.data, VertexShader.size};
        desc.PS             = {DirectionalLightPixelShader.data, DirectionalLightPixelShader.size};
        desc.InputLayout    = {INPUT_ELEMENTS, std::size(INPUT_ELEMENTS)};
        desc.BlendState = {FALSE, FALSE, {blend, blend, blend, blend, blend, blend, blend, blend}};
        m_directionalLightPipeline = m_renderDevice.newGraphicsPipeline(desc);
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
                std::string newTitle("Boxes - ");
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

    // Calculate light positions.
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    const float time    = static_cast<float>(now.QuadPart) / 1000000.0f;
    const float radius  = time / 4.0f;
    const float sinTime = std::sin(radius);
    const float cosTime = std::cos(radius);

    PointLight pointLight{
        Vector4{1.0f, -1.0f, 1.0f, 1.0f},
        Vector4{std::fabs(sinTime), std::fabs(cosTime), 1.0f, 1.0f},
    };

    Matrix4 pointLightModel = Matrix4(1.0f).rotate({1.0f, 1.0f, 1.0f}, radius);
    pointLight.position     = pointLight.position * pointLightModel;

    // SpotLight spotLight{
    //     Vector4{2.0f, 0.0f, 0.0f, 1.0f},
    //     Vector4{0.0f, 0.0f, 1.0f, 0.0f},
    //     Vector4{1.0f, std::fabs(sinTime), std::fabs(cosTime), 1.0f},
    //     0.8f,
    // };

    // Matrix4 spotLightModel = Matrix4(1.0f).rotate({0.0f, 1.0f, 0.0f}, radius);
    // spotLight.position     = spotLight.position * spotLightModel;
    // spotLight.direction    = (Vector4{0.0f, 0.0f, 0.0f, 1.0f} - spotLight.position).normalized();

    DirectionalLight directionalLight{
        Vector4{1.0f, 1.0f, 1.0f, 0.0f}.normalized(),
        Vector4{1.0f, 1.0f, 1.0f, 1.0f},
    };

    const Material material{
        /* Color     = */ Vector4{1.0f, 1.0f, 1.0f, 1.0f},
        /* Metallic  = */ 0.2f,
        /* Roughness = */ 0.8f,
    };

    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_model.render([this, renderPass, &material, &pointLight](const Mesh    &mesh,
                                                              const Matrix4 &modelTransform) {
        m_commandBuffer.setPipelineState(m_pointLightPipeline);
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
        m_commandBuffer.setGraphicsConstantBuffer(0, 1, &material, sizeof(material));
        m_commandBuffer.setGraphicsConstantBuffer(0, 2, &pointLight, sizeof(pointLight));

        auto [width, height] = m_window.frameSize();
        m_commandBuffer.setViewport(0, 0, width, height);
        m_commandBuffer.setScissorRect(0, 0, width, height);

        m_commandBuffer.drawIndexed(mesh.index.count, 0);
        m_commandBuffer.endRenderPass();
    });

    m_model.render([this, &renderPass, &material, &directionalLight](const Mesh    &mesh,
                                                                    const Matrix4 &modelTransform) {
        renderPass.renderTargets[0].loadAction = LoadAction::Load;
        m_commandBuffer.setPipelineState(m_directionalLightPipeline);
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
        m_commandBuffer.setGraphicsConstantBuffer(0, 1, &material, sizeof(material));
        m_commandBuffer.setGraphicsConstantBuffer(0, 2, &directionalLight,
                                                  sizeof(directionalLight));

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

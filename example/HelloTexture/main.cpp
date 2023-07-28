#include <ink/asset/image.h>
#include <ink/core/log.h>
#include <ink/core/window.h>
#include <ink/math/number.h>
#include <ink/math/quaternion.h>
#include <ink/render/command_buffer.h>
#include <ink/render/swap_chain.h>

// Generated header files.
#include "pixel.hlsl.h"
#include "vertex.hlsl.h"

using namespace ink;

struct Vertex {
    Vector3 position;
    Vector2 texcoord;
};

struct Camera {
    Vector4 position    = {0.0f, 0.0f, 0.0f, 1.0f};
    float   pitch       = 0;
    float   roll        = 0;
    float   fovY        = Pi<float> / 2.0f;
    float   aspectRatio = 4.0f / 3.0f;
    float   zNear       = 0.1f;
    float   zFar        = 1000.0f;

    auto applyPitch(float offset) noexcept -> void {
        pitch += offset;
        pitch = std::clamp(pitch, -Pi<float> * 0.4f, Pi<float> * 0.4f);
    }

    auto applyRoll(float offset) noexcept -> void {
        roll += offset;
        roll = std::fmod(roll, Pi<float> * 2.0f);
    }

    [[nodiscard]]
    auto direction() const noexcept -> Vector4 {
        Quaternion quat(pitch, roll, 0.0f);
        Quaternion dir(0.0f, 0.0f, 0.0f, 1.0f);
        dir = quat * dir * quat.conjugated();
        return Vector4{dir.x, dir.y, dir.z, 0.0f}.normalized();
    }

    [[nodiscard]]
    auto lookAt() const noexcept -> Matrix4 {
        auto dir = direction();
        return ink::lookTo(position, dir, {0.0f, 1.0f, 0.0f, 0.0f});
    }

    [[nodiscard]]
    auto perspective() const noexcept -> Matrix4 {
        return ink::perspective(fovY, aspectRatio, zNear, zFar);
    }
};

class Application {
public:
    Application() noexcept;

    auto run() -> void;
    auto update(float deltaTime) -> void;

private:
    Window      m_mainWindow;
    SwapChain   m_swapChain;
    DepthBuffer m_depthBuffer;

    RootSignature         m_rootSignature;
    GraphicsPipelineState m_pipelineState;

    CommandBuffer    m_commandBuffer;
    StructuredBuffer m_vertexBuffer;
    Texture2D        m_boxTexture;
    Texture2D        m_faceTexture;
    SamplerView      m_sampler;

    Camera m_mainCamera;
};

inline constexpr Vertex VERTICES[] = {
    {Vector3{-0.5f, -0.5f, -0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{+0.5f, -0.5f, -0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{+0.5f, +0.5f, -0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{+0.5f, +0.5f, -0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{-0.5f, +0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{-0.5f, -0.5f, -0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{-0.5f, -0.5f, +0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{+0.5f, -0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{+0.5f, +0.5f, +0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{+0.5f, +0.5f, +0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{-0.5f, +0.5f, +0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{-0.5f, -0.5f, +0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{-0.5f, +0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{-0.5f, +0.5f, -0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{-0.5f, -0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{-0.5f, -0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{-0.5f, -0.5f, +0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{-0.5f, +0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{+0.5f, +0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{+0.5f, +0.5f, -0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{+0.5f, -0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{+0.5f, -0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{+0.5f, -0.5f, +0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{+0.5f, +0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{-0.5f, -0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{+0.5f, -0.5f, -0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{+0.5f, -0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{+0.5f, -0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{-0.5f, -0.5f, +0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{-0.5f, -0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{-0.5f, +0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
    {Vector3{+0.5f, +0.5f, -0.5f}, Vector2{1.0f, 1.0f}},
    {Vector3{+0.5f, +0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{+0.5f, +0.5f, +0.5f}, Vector2{1.0f, 0.0f}},
    {Vector3{-0.5f, +0.5f, +0.5f}, Vector2{0.0f, 0.0f}},
    {Vector3{-0.5f, +0.5f, -0.5f}, Vector2{0.0f, 1.0f}},
};

inline constexpr D3D12_INPUT_ELEMENT_DESC INPUT_ELEMENTS[]{
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
        /* SemanticName         = */ "TEXCOORD",
        /* SemanticIndex        = */ 0,
        /* Format               = */ DXGI_FORMAT_R32G32_FLOAT,
        /* InputSlot            = */ 0,
        /* AlignedByteOffset    = */ 12,
        /* InputSlotClass       = */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        /* InstanceDataStepRate = */ 0,
    },
};

Application::Application() noexcept
    : m_mainWindow(u"HelloTexture", 800, 600),
      m_swapChain(m_mainWindow),
      m_depthBuffer(m_mainWindow.width(), m_mainWindow.height(), DXGI_FORMAT_D32_FLOAT),
      m_rootSignature(),
      m_pipelineState(),
      m_commandBuffer(),
      m_vertexBuffer(std::uint32_t(std::size(VERTICES)), sizeof(Vertex)),
      m_boxTexture() {
    { // Upload data to GPU buffer.
        Image boxImage, faceImage;

        if (!boxImage.load(u"asset/container.jpg")) {
            logFatal(u"Failed to load asset/container.jpg.");
            std::terminate();
        }

        if (!faceImage.load(u"asset/awesomeface.png")) {
            logFatal(u"Failed to load asset/awesomeface.png.");
            std::terminate();
        }

        m_boxTexture = Texture2D(boxImage.width(), boxImage.height(), boxImage.pixelFormat(), 1U);
        m_faceTexture =
            Texture2D(faceImage.width(), faceImage.height(), faceImage.pixelFormat(), 1U);

        m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandBuffer.transition(m_boxTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandBuffer.transition(m_faceTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandBuffer.copyBuffer(VERTICES, m_vertexBuffer, 0, sizeof(VERTICES));
        m_commandBuffer.copyTexture(boxImage.data(), boxImage.pixelFormat(), boxImage.rowPitch(),
                                    boxImage.width(), boxImage.height(), m_boxTexture, 0);
        m_commandBuffer.copyTexture(faceImage.data(), faceImage.pixelFormat(), faceImage.rowPitch(),
                                    faceImage.width(), faceImage.height(), m_faceTexture, 0);
        m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_commandBuffer.transition(m_boxTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_commandBuffer.transition(m_faceTexture, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_commandBuffer.submit();
    }

    { // Create root signature.
        D3D12_ROOT_PARAMETER parameters[2]{};

        D3D12_DESCRIPTOR_RANGE viewRanges[2]{
            D3D12_DESCRIPTOR_RANGE{
                /* RangeType                         = */ D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                /* NumDescriptors                    = */ 1,
                /* BaseShaderRegister                = */ 0,
                /* RegisterSpace                     = */ 0,
                /* OffsetInDescriptorsFromTableStart = */ 0,
            },
            D3D12_DESCRIPTOR_RANGE{
                /* RangeType                         = */ D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                /* NumDescriptors                    = */ 2,
                /* BaseShaderRegister                = */ 0,
                /* RegisterSpace                     = */ 0,
                /* OffsetInDescriptorsFromTableStart = */ 1,
            },
        };
        parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[0].DescriptorTable.NumDescriptorRanges = UINT(std::size(viewRanges));
        parameters[0].DescriptorTable.pDescriptorRanges   = viewRanges;
        parameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_DESCRIPTOR_RANGE samplerRange{
            /* RangeType                         = */ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
            /* NumDescriptors                    = */ 1,
            /* BaseShaderRegister                = */ 0,
            /* RegisterSpace                     = */ 0,
            /* OffsetInDescriptorsFromTableStart = */ 0,
        };
        parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[1].DescriptorTable.NumDescriptorRanges = 1;
        parameters[1].DescriptorTable.pDescriptorRanges   = &samplerRange;
        parameters[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc{
            /* NumParameters     = */ 2,
            /* pParameters       = */ parameters,
            /* NumStaticSamplers = */ 0,
            /* pStaticSamplers   = */ nullptr,
            /* Flags             = */ D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
        };

        m_rootSignature = RootSignature(desc);
    }

    { // Create graphics pipeline state.
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

        desc.pRootSignature                        = m_rootSignature.rootSignature();
        desc.VS.pShaderBytecode                    = VERTEX_SHADER;
        desc.VS.BytecodeLength                     = sizeof(VERTEX_SHADER);
        desc.PS.pShaderBytecode                    = PIXEL_SHADER;
        desc.PS.BytecodeLength                     = sizeof(PIXEL_SHADER);
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
        desc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS;
        desc.DepthStencilState.StencilEnable    = FALSE;
        desc.DepthStencilState.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
        desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        desc.SampleMask                         = UINT_MAX;
        desc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets                   = 1;
        desc.RTVFormats[0]                      = m_swapChain.pixelFormat();
        desc.DSVFormat                          = m_depthBuffer.pixelFormat();
        desc.SampleDesc.Count                   = 1;

        m_pipelineState = GraphicsPipelineState(desc);
    }

    { // Create sampler.
        D3D12_SAMPLER_DESC desc{};
        desc.Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressV       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.AddressW       = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc.MipLODBias     = 0;
        desc.MaxAnisotropy  = 16;
        desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        desc.MinLOD         = 0.0f;
        desc.MaxLOD         = D3D12_FLOAT32_MAX;

        m_sampler.initSampler(desc);
    }

    // Set swap chain back buffer clear color.
    m_swapChain.setClearColor(colors::Black);
}

auto Application::run() -> void {
    LARGE_INTEGER now, lastUpdate, countsPerSec;
    QueryPerformanceFrequency(&countsPerSec);
    QueryPerformanceCounter(&now);
    const double timeCoef = 1.0 / static_cast<double>(countsPerSec.QuadPart);
    lastUpdate            = now;

    MSG msg{};
    while (!m_mainWindow.isClosed()) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        } else {
            QueryPerformanceCounter(&now);
            double deltaTime = static_cast<double>(now.QuadPart - lastUpdate.QuadPart) * timeCoef;
            this->update(static_cast<float>(deltaTime));
            lastUpdate = now;
        }
    }
}

struct Transform {
    Matrix4 model;
    Matrix4 view;
    Matrix4 projection;
};

auto Application::update(float deltaTime) -> void {
    { // Update camera.
        Vector4 dir   = m_mainCamera.direction();
        Vector4 right = cross(Vector4{0.0f, 1.0f, 0.0f, 0.0f}, dir).normalized();
        if (isKeyPressed(KeyCode::W))
            m_mainCamera.position += deltaTime * dir;
        if (isKeyPressed(KeyCode::A))
            m_mainCamera.position -= deltaTime * right;
        if (isKeyPressed(KeyCode::S))
            m_mainCamera.position -= deltaTime * dir;
        if (isKeyPressed(KeyCode::D))
            m_mainCamera.position += deltaTime * right;
        if (isKeyPressed(KeyCode::Up))
            m_mainCamera.applyPitch(-deltaTime);
        if (isKeyPressed(KeyCode::Down))
            m_mainCamera.applyPitch(deltaTime);
        if (isKeyPressed(KeyCode::Left))
            m_mainCamera.applyRoll(-deltaTime);
        if (isKeyPressed(KeyCode::Right))
            m_mainCamera.applyRoll(deltaTime);
    }

    LARGE_INTEGER now, countsPerSec;
    QueryPerformanceFrequency(&countsPerSec);
    QueryPerformanceCounter(&now);

    const float timeCoef = static_cast<float>(static_cast<double>(now.QuadPart) /
                                              static_cast<double>(countsPerSec.QuadPart));

    // Set transform.
    Transform transform;
    transform.model = Matrix4(1.0f);
    transform.model.rotate({0.5f, 1.0f, 0.0f}, Pi<float> * timeCoef * 0.25f);
    transform.view       = m_mainCamera.lookAt();
    transform.projection = m_mainCamera.perspective();

    auto &backBuffer = m_swapChain.backBuffer();

    m_commandBuffer.transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandBuffer.transition(m_depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_commandBuffer.setRenderTarget(backBuffer, m_depthBuffer);
    m_commandBuffer.clearColor(backBuffer);
    m_commandBuffer.clearDepth(m_depthBuffer);

    m_commandBuffer.setPipelineState(m_pipelineState);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandBuffer.setVertexBuffer(0, m_vertexBuffer);

    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);
    m_commandBuffer.setGraphicsConstantBuffer(0, 0, &transform, sizeof(transform));
    m_commandBuffer.setGraphicsView(0, 1, m_boxTexture.shaderResourceView());
    m_commandBuffer.setGraphicsView(0, 2, m_faceTexture.shaderResourceView());
    m_commandBuffer.setGraphicsSampler(1, 0, m_sampler);

    m_commandBuffer.setViewport(0, 0, m_mainWindow.width(), m_mainWindow.height());
    m_commandBuffer.setScissorRect(0, 0, m_mainWindow.width(), m_mainWindow.height());
    m_commandBuffer.draw(static_cast<std::uint32_t>(std::size(VERTICES)));

    m_commandBuffer.transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
    m_commandBuffer.submit();

    m_swapChain.present();
}

int WINAPI WinMain(_In_ HINSTANCE     hInstance,
                   _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR         lpCmdLine,
                   _In_ int           nShowCmd) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    addLogMessageHandler<VisualStudioDebugLogMessageHandler>();

    Application app;
    app.run();

    return 0;
}

#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/math/numbers.hpp"
#include "ink/math/quaternion.hpp"
#include "ink/render/device.hpp"

#include <DirectXTex.h>

// Generated headers.
#include "texture.ps.hlsl.hpp"
#include "texture.vs.hlsl.hpp"

using namespace ink;

struct Vertex {
    Vector3 position;
    Vector2 texcoord;
};

struct Camera {
    Vector4    position    = {0.0f, 0.0f, -1.5f, 1.0f};
    Quaternion rotation    = {1.0f};
    float      fovY        = Pi<float> / 2.0f;
    float      aspectRatio = 4.0f / 3.0f;
    float      zNear       = 0.1f;
    float      zFar        = 1000.0f;

    [[nodiscard]] auto direction() const noexcept -> Vector4 {
        Quaternion dir{0.0f, 0.0f, 0.0f, 1.0f};
        dir = rotation * dir * rotation.conjugated();
        return Vector4{dir.x, dir.y, dir.z, 0.0f}.normalized();
    }

    [[nodiscard]] auto view() const noexcept -> Matrix4 {
        return ink::lookTo(position, direction(), {0.0f, 1.0f, 0.0f, 0.0f});
    }

    [[nodiscard]] auto projection() const noexcept -> Matrix4 {
        return ink::perspective(fovY, aspectRatio, zNear, zFar);
    }
};

class Application {
public:
    Application();

    auto run() -> void;
    auto update(float deltaTime) -> void;

private:
    Window                m_window;
    Camera                m_camera;
    RenderDevice          m_renderDevice;
    SwapChain             m_swapChain;
    DepthBuffer           m_depthBuffer;
    RootSignature         m_rootSignature;
    GraphicsPipelineState m_pipelineState;
    CommandBuffer         m_commandBuffer;
    StructuredBuffer      m_vertexBuffer;
    Texture2D             m_boxTexture;
    Texture2D             m_faceTexture;
    Sampler               m_sampler;
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

Application::Application()
    : m_window("HelloTexture", 800, 600, WindowStyle::Titled | WindowStyle::Resizable),
      m_camera(),
      m_renderDevice(),
      m_swapChain(m_renderDevice.newSwapChain(m_window)),
      m_depthBuffer(m_renderDevice.newDepthBuffer(
          m_window.frameSize().width, m_window.frameSize().height, DXGI_FORMAT_D32_FLOAT)),
      m_rootSignature(),
      m_pipelineState(),
      m_commandBuffer(m_renderDevice.newCommandBuffer()),
      m_vertexBuffer(m_renderDevice.newStructuredBuffer(std::size(VERTICES), sizeof(Vertex))),
      m_boxTexture(),
      m_faceTexture(),
      m_sampler(m_renderDevice.newSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                          D3D12_TEXTURE_ADDRESS_MODE_WRAP)) {
    m_window.setFrameResizeCallback([this](Window &, std::uint32_t width, std::uint32_t height) {
        if ((width | height) == 0)
            return;

        m_swapChain.resize({width, height});
        m_depthBuffer        = m_renderDevice.newDepthBuffer(width, height, DXGI_FORMAT_D32_FLOAT);
        m_camera.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    });

    { // Upload data to GPU buffer.
        DirectX::TexMetadata  metadata;
        DirectX::ScratchImage image;

        HRESULT hr = DirectX::LoadFromWICFile(L"asset/container.jpg", DirectX::WIC_FLAGS_NONE,
                                              &metadata, image);
        if (FAILED(hr))
            throw SystemErrorException(hr, "Failed to load asset.container.jpg.");

        m_boxTexture =
            m_renderDevice.new2DTexture(metadata.width, metadata.height, metadata.format, 1);
        m_commandBuffer.copyTexture(image.GetPixels(), metadata.format, image.GetImages()->rowPitch,
                                    metadata.width, metadata.height, m_boxTexture, 0);
        m_commandBuffer.transition(m_boxTexture, D3D12_RESOURCE_STATE_GENERIC_READ);

        image.Release();
        hr = DirectX::LoadFromWICFile(L"asset/awesomeface.png", DirectX::WIC_FLAGS_NONE, &metadata,
                                      image);
        if (FAILED(hr))
            throw SystemErrorException(hr, "Failed to load asset/awesomeface.png.");

        m_faceTexture =
            m_renderDevice.new2DTexture(metadata.width, metadata.height, metadata.format, 1);
        m_commandBuffer.copyTexture(image.GetPixels(), metadata.format, image.GetImages()->rowPitch,
                                    metadata.width, metadata.height, m_faceTexture, 0);
        m_commandBuffer.transition(m_faceTexture, D3D12_RESOURCE_STATE_GENERIC_READ);

        m_commandBuffer.copyBuffer(VERTICES, m_vertexBuffer, 0, sizeof(VERTICES));
        m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

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

        m_rootSignature = m_renderDevice.newRootSignature(desc);
    }

    // Create graphics pipeline state.
    DXGI_FORMAT rtvFormat = m_swapChain.pixelFormat();
    m_pipelineState       = m_renderDevice.newGraphicsPipeline(
        m_rootSignature, {g_vertex_main, sizeof(g_vertex_main)},
        {g_pixel_main, sizeof(g_pixel_main)}, {}, static_cast<UINT>(std::size(INPUT_ELEMENTS)),
        INPUT_ELEMENTS, 1, &rtvFormat, m_depthBuffer.pixelFormat(), D3D12_FILL_MODE_SOLID,
        D3D12_CULL_MODE_NONE);

    m_swapChain.setClearColor(colors::White);
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
                std::string newTitle("HelloWorld - ");
                newTitle.append(std::to_string(fps));
                newTitle.append(" FPS");
                m_window.title(newTitle);

                lastUpdateFps = now;
            }

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
    // Set transform.
    LARGE_INTEGER now, countsPerSec;
    QueryPerformanceFrequency(&countsPerSec);
    QueryPerformanceCounter(&now);

    const auto timeCoef = static_cast<float>(static_cast<double>(now.QuadPart) /
                                             static_cast<double>(countsPerSec.QuadPart));

    Transform transform{
        /* model      = */ Matrix4(1.0f).rotated({0.5f, 1.0f, 0.0f}, Pi<float> * timeCoef * 0.5f),
        /* view       = */ m_camera.view(),
        /* projection = */ m_camera.projection(),
    };

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

    m_commandBuffer.beginRenderPass(renderPass);
    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);
    m_commandBuffer.setPipelineState(m_pipelineState);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandBuffer.setVertexBuffer(0, m_vertexBuffer);

    m_commandBuffer.setGraphicsConstantBuffer(0, 0, &transform, sizeof(transform));
    m_commandBuffer.setGraphicsDescriptor(0, 1, m_boxTexture.shaderResourceView());
    m_commandBuffer.setGraphicsDescriptor(0, 2, m_faceTexture.shaderResourceView());
    m_commandBuffer.setGraphicsDescriptor(1, 0, m_sampler.descriptor());

    auto [width, height] = m_window.frameSize();
    m_commandBuffer.setViewport(0, 0, width, height);
    m_commandBuffer.setScissorRect(0, 0, width, height);
    m_commandBuffer.draw(static_cast<std::uint32_t>(std::size(VERTICES)));
    m_commandBuffer.endRenderPass();

    m_commandBuffer.submit();
    m_swapChain.present();
}

auto main() -> int {
    CoInitialize(nullptr);
    try {
        auto app = std::make_unique<Application>();
        app->run();
    } catch (Exception &e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    }
    CoUninitialize();
}

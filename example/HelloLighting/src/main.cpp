#include <ink/core/log.h>
#include <ink/core/window.h>
#include <ink/math/number.h>
#include <ink/math/quaternion.h>
#include <ink/render/command_buffer.h>
#include <ink/render/swap_chain.h>

// Generator header files.
#include "light_pixel.hlsl.h"
#include "light_vertex.hlsl.h"
#include "object_pixel.hlsl.h"
#include "object_vertex.hlsl.h"

using namespace ink;

class Camera {
public:
    Camera() noexcept = default;

    auto rotate(float pitch, float roll) noexcept -> void {
        m_pitch += pitch;
        m_roll += roll;
        m_pitch = std::clamp(m_pitch, -Pi<float> * 0.45f, Pi<float> * 0.45f);
        m_roll  = std::fmod(m_roll, Pi<float> * 2.0f);

        m_isLookAtDirty = true;
    }

    [[nodiscard]]
    auto forward() const noexcept -> Vector4 {
        Quaternion rotation(m_pitch, m_roll, 0.0f);
        Quaternion direction(0.0f, 0.0f, 0.0f, 1.0f);
        direction = rotation * direction * rotation.conjugated();
        return Vector4(direction.x, direction.y, direction.z, 0.0f).normalized();
    }

    [[nodiscard]]
    auto right() const noexcept -> Vector4 {
        return cross(Vector4(0.0f, 1.0f, 0.0f, 0.0f), forward());
    }

    auto move(Vector4 offset) noexcept -> void {
        m_position += offset;
        m_isLookAtDirty = true;
    }

    [[nodiscard]]
    auto view() const noexcept -> Matrix4 {
        if (!m_isLookAtDirty)
            return m_lookAt;

        Quaternion rotation(m_pitch, m_roll, 0.0f);
        Quaternion direction(0.0f, 0.0f, 0.0f, 1.0f);
        direction = rotation * direction * rotation.conjugated();
        Vector4 dir(direction.x, direction.y, direction.z, 0.0f);
        dir.normalize();
        m_lookAt        = ink::lookTo(m_position, dir, {0.0f, 1.0f, 0.0f, 0.0f});
        m_isLookAtDirty = false;

        return m_lookAt;
    }

    [[nodiscard]]
    auto fieldOfView() const noexcept -> float {
        return m_fovY;
    }

    auto fieldOfView(float fov) noexcept -> void {
        m_fovY              = fov;
        m_isProjectionDirty = true;
    }

    [[nodiscard]]
    auto aspectRatio() const noexcept -> float {
        return m_aspectRatio;
    }

    auto aspectRatio(float aspect) noexcept -> void {
        m_aspectRatio       = aspect;
        m_isProjectionDirty = true;
    }

    auto aspectRatio(std::uint32_t width, std::uint32_t height) noexcept -> void {
        m_aspectRatio       = static_cast<float>(width) / static_cast<float>(height);
        m_isProjectionDirty = true;
    }

    [[nodiscard]]
    auto zNear() const noexcept -> float {
        return m_zNear;
    }

    auto zNear(float dist) noexcept -> void {
        m_zNear             = dist;
        m_isProjectionDirty = true;
    }

    [[nodiscard]]
    auto zFar() const noexcept -> float {
        return m_zFar;
    }

    auto zFar(float dist) noexcept -> void {
        m_zFar              = dist;
        m_isProjectionDirty = true;
    }

    [[nodiscard]]
    auto position() const noexcept -> Vector4 {
        return m_position;
    }

    [[nodiscard]]
    auto projection() const noexcept -> Matrix4 {
        if (!m_isProjectionDirty)
            return m_projection;

        m_projection        = ink::perspective(m_fovY, m_aspectRatio, m_zNear, m_zFar);
        m_isProjectionDirty = false;
        return m_projection;
    }

private:
    mutable bool m_isLookAtDirty     = true;
    mutable bool m_isProjectionDirty = true;
    float        m_pitch             = 0.0f;
    float        m_roll              = 0.0f;
    float        m_fovY              = Pi<float> * 0.25f;
    float        m_aspectRatio       = 4.0f / 3.0f;
    float        m_zNear             = 0.1f;
    float        m_zFar              = 1000.0f;
    Vector4      m_position          = {0.0f, 1.0f, -3.0f, 1.0f};

    mutable Matrix4 m_lookAt{};
    mutable Matrix4 m_projection{};
};

struct Vertex {
    Vector3 position;
    Vector3 normal;
};

constexpr Vertex VERTICES[] = {
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}}, {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},   {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},   {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},     {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},  {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},     {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},   {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},   {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},   {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},     {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
};

class Application {
public:
    Application() noexcept;

    auto run() -> void;
    auto update(float deltaTime) -> void;

private:
    Window                m_mainWindow;
    SwapChain             m_swapChain;
    DepthBuffer           m_depthBuffer;
    RootSignature         m_rootSignature;
    GraphicsPipelineState m_objectPipelineState;
    GraphicsPipelineState m_lightPipelineState;
    CommandBuffer         m_commandBuffer;
    StructuredBuffer      m_vertexBuffer;
    Camera                m_mainCamera;
};

constexpr D3D12_INPUT_ELEMENT_DESC INPUT_ELEMENTS[]{
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
        /* InputSlot            = */ 0,
        /* AlignedByteOffset    = */ 12,
        /* InputSlotClass       = */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        /* InstanceDataStepRate = */ 0,
    },
};

Application::Application() noexcept
    : m_mainWindow(u"HelloLighting", 800, 600),
      m_swapChain(m_mainWindow),
      m_depthBuffer(m_mainWindow.width(), m_mainWindow.height(), DXGI_FORMAT_D32_FLOAT),
      m_rootSignature(),
      m_objectPipelineState(),
      m_lightPipelineState(),
      m_commandBuffer(),
      m_vertexBuffer(std::uint32_t(std::size(VERTICES)), sizeof(Vertex)),
      m_mainCamera() {
    { // Upload to GPU buffer.
        m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
        m_commandBuffer.copyBuffer(VERTICES, m_vertexBuffer, 0, sizeof(VERTICES));
        m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_commandBuffer.submit();
    }

    { // Create root signature.
        D3D12_ROOT_PARAMETER parameters[1]{};

        D3D12_DESCRIPTOR_RANGE viewRanges[]{
            D3D12_DESCRIPTOR_RANGE{
                /* RangeType                         = */ D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                /* NumDescriptors                    = */ 2,
                /* BaseShaderRegister                = */ 0,
                /* RegisterSpace                     = */ 0,
                /* OffsetInDescriptorsFromTableStart = */ 0,
            },
        };
        parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[0].DescriptorTable.NumDescriptorRanges = UINT(std::size(viewRanges));
        parameters[0].DescriptorTable.pDescriptorRanges   = viewRanges;
        parameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC desc{
            /* NumParameters     = */ 1,
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
        desc.VS.pShaderBytecode                    = OBJECT_VERTEX_SHADER;
        desc.VS.BytecodeLength                     = sizeof(OBJECT_VERTEX_SHADER);
        desc.PS.pShaderBytecode                    = OBJECT_PIXEL_SHADER;
        desc.PS.BytecodeLength                     = sizeof(OBJECT_PIXEL_SHADER);
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

        m_objectPipelineState = GraphicsPipelineState(desc);

        desc.VS.pShaderBytecode = LIGHT_VERTEX_SHADER;
        desc.VS.BytecodeLength  = sizeof(LIGHT_VERTEX_SHADER);
        desc.PS.pShaderBytecode = LIGHT_PIXEL_SHADER;
        desc.PS.BytecodeLength  = sizeof(LIGHT_PIXEL_SHADER);

        m_lightPipelineState = GraphicsPipelineState(desc);
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

struct TransformUniform {
    Matrix4 model;
    Matrix4 view;
    Matrix4 projection;
};

struct LightUniform {
    Vector4 cameraPos;
    Vector4 lightPos;
    Color   objectColor;
    Color   lightColor;
};

auto Application::update(float deltaTime) -> void {
    // Update camera.
    if (isKeyPressed(KeyCode::W))
        m_mainCamera.move(m_mainCamera.forward() * deltaTime);
    if (isKeyPressed(KeyCode::A))
        m_mainCamera.move(-m_mainCamera.right() * deltaTime);
    if (isKeyPressed(KeyCode::S))
        m_mainCamera.move(-m_mainCamera.forward() * deltaTime);
    if (isKeyPressed(KeyCode::D))
        m_mainCamera.move(m_mainCamera.right() * deltaTime);
    if (isKeyPressed(KeyCode::Up))
        m_mainCamera.rotate(-deltaTime, 0);
    if (isKeyPressed(KeyCode::Down))
        m_mainCamera.rotate(deltaTime, 0);
    if (isKeyPressed(KeyCode::Left))
        m_mainCamera.rotate(0, -deltaTime);
    if (isKeyPressed(KeyCode::Right))
        m_mainCamera.rotate(0, deltaTime);

    TransformUniform transform;
    transform.model      = Matrix4(1.0f);
    transform.view       = m_mainCamera.view();
    transform.projection = m_mainCamera.projection();

    LightUniform light;
    light.cameraPos   = m_mainCamera.position();
    light.lightPos    = {1.2f, 1.0f, 2.0f, 1.0f};
    light.lightColor  = {1.0f, 1.0f, 1.0f, 1.0f};
    light.objectColor = {1.0f, 0.5f, 0.31f, 1.0f};

    auto &backBuffer = m_swapChain.backBuffer();
    m_commandBuffer.transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandBuffer.transition(m_depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_commandBuffer.setRenderTarget(backBuffer, m_depthBuffer);
    m_commandBuffer.clearColor(backBuffer);
    m_commandBuffer.clearDepth(m_depthBuffer);

    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);

    m_commandBuffer.setPipelineState(m_objectPipelineState);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandBuffer.setVertexBuffer(0, m_vertexBuffer);

    m_commandBuffer.setGraphicsConstantBuffer(0, 0, &transform, sizeof(transform));
    m_commandBuffer.setGraphicsConstantBuffer(0, 1, &light, sizeof(light));

    m_commandBuffer.setViewport(0, 0, m_mainWindow.width(), m_mainWindow.height());
    m_commandBuffer.setScissorRect(0, 0, m_mainWindow.width(), m_mainWindow.height());
    m_commandBuffer.draw(static_cast<std::uint32_t>(std::size(VERTICES)));

    transform.model = Matrix4(1.0f).translated(light.lightPos);
    m_commandBuffer.setPipelineState(m_lightPipelineState);

    m_commandBuffer.setGraphicsConstantBuffer(0, 0, &transform, sizeof(transform));
    m_commandBuffer.setGraphicsConstantBuffer(0, 1, &light, sizeof(light));
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

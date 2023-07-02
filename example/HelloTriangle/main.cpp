#include <ink/core/log.h>
#include <ink/core/window.h>
#include <ink/math/vector.h>
#include <ink/render/command_buffer.h>
#include <ink/render/swap_chain.h>

// Generated header files.
#include "pixel.hlsl.h"
#include "vertex.hlsl.h"

using namespace ink;

struct Vertex {
    Vector4 position;
    Color   color;
};

class Application {
public:
    Application() noexcept;

    auto run() -> void;
    auto update() -> void;

private:
    Window    m_mainWindow;
    SwapChain m_swapChain;

    RootSignature         m_rootSignature;
    GraphicsPipelineState m_pipelineState;

    CommandBuffer    m_commandBuffer;
    StructuredBuffer m_vertexBuffer;
};

inline constexpr Vertex VERTICES[]{
    {{0.0f, 0.5f, 0.0f, 1.0f}, colors::Red},
    {{0.5f, -0.5f, 0.0f, 1.0f}, colors::Green},
    {{-0.5f, -0.5f, 0.0f, 1.0f}, colors::Blue},
};

inline constexpr D3D12_INPUT_ELEMENT_DESC INPUT_ELEMENTS[]{
    D3D12_INPUT_ELEMENT_DESC{
        /* SemanticName         = */ "POSITION",
        /* SemanticIndex        = */ 0,
        /* Format               = */ DXGI_FORMAT_R32G32B32A32_FLOAT,
        /* InputSlot            = */ 0,
        /* AlignedByteOffset    = */ 0,
        /* InputSlotClass       = */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        /* InstanceDataStepRate = */ 0,
    },
    D3D12_INPUT_ELEMENT_DESC{
        /* SemanticName         = */ "COLOR",
        /* SemanticIndex        = */ 0,
        /* Format               = */ DXGI_FORMAT_R32G32B32A32_FLOAT,
        /* InputSlot            = */ 0,
        /* AlignedByteOffset    = */ 16,
        /* InputSlotClass       = */ D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        /* InstanceDataStepRate = */ 0,
    },
};

Application::Application() noexcept
    : m_mainWindow(u"HelloTriangle", 800, 600),
      m_swapChain(m_mainWindow),
      m_rootSignature(),
      m_pipelineState(),
      m_commandBuffer(),
      m_vertexBuffer(3, sizeof(Vertex)) {
    // Upload vertex data to vertex buffer.
    m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandBuffer.copyBuffer(VERTICES, m_vertexBuffer, 0, sizeof(VERTICES));
    m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    m_commandBuffer.submit();

    // Create root signature.
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    m_rootSignature   = RootSignature(rootSigDesc);

    // Create graphics pipeline state.
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

    desc.DepthStencilState.DepthEnable   = FALSE;
    desc.DepthStencilState.StencilEnable = FALSE;
    desc.SampleMask                      = UINT_MAX;
    desc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.NumRenderTargets                = 1;
    desc.RTVFormats[0]                   = m_swapChain.pixelFormat();
    desc.SampleDesc.Count                = 1;

    m_pipelineState = GraphicsPipelineState(desc);

    // Set swap chain back buffer clear color.
    m_swapChain.setClearColor(colors::Black);
}

auto Application::run() -> void {
    MSG msg{};
    while (!m_mainWindow.isClosed()) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        } else {
            this->update();
        }
    }
}

auto Application::update() -> void {
    auto &backBuffer = m_swapChain.backBuffer();

    m_commandBuffer.transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandBuffer.setRenderTarget(backBuffer);
    m_commandBuffer.clearColor(backBuffer);

    m_commandBuffer.setGraphicsRootSignature(m_rootSignature);
    m_commandBuffer.setPipelineState(m_pipelineState);
    m_commandBuffer.setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandBuffer.setVertexBuffer(0, m_vertexBuffer);

    m_commandBuffer.setViewport(0, 0, m_mainWindow.width(), m_mainWindow.height());
    m_commandBuffer.setScissorRect(0, 0, m_mainWindow.width(), m_mainWindow.height());
    m_commandBuffer.draw(3);

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

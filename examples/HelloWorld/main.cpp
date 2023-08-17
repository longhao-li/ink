#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/render/command_buffer.hpp"
#include "ink/render/device.hpp"

// Generated headers.
#include "triangle.ps.hlsl.hpp"
#include "triangle.vs.hlsl.hpp"

using namespace ink;

struct Vertex {
    float position[4];
    Color color;
};

class Application {
public:
    Application();
    ~Application() noexcept;

    auto run() -> void;
    auto update() -> void;

private:
    LARGE_INTEGER m_lastUpdate;
    LARGE_INTEGER m_lastFpsUpdate;
    LARGE_INTEGER m_countsPerSecond;

    bool m_isPaused;

    RenderDevice          m_renderDevice;
    Window                m_window;
    SwapChain             m_swapChain;
    RootSignature         m_rootSignature;
    GraphicsPipelineState m_pipelineState;
    CommandBuffer         m_commandBuffer;
    StructuredBuffer      m_vertexBuffer;
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

Application::Application()
    : m_lastUpdate(),
      m_lastFpsUpdate(),
      m_countsPerSecond(),
      m_isPaused(),
      m_renderDevice(),
      m_window("HelloWorld", 800, 600, WindowStyle::Titled | WindowStyle::Resizable),
      m_swapChain(m_renderDevice.newSwapChain(m_window)),
      m_rootSignature(m_renderDevice.newRootSignature(0, nullptr)),
      m_pipelineState(),
      m_commandBuffer(m_renderDevice.newCommandBuffer()),
      m_vertexBuffer(m_renderDevice.newStructuredBuffer(3, sizeof(Vertex))) {
    m_window.setFrameResizeCallback([this](Window &, std::uint32_t width, std::uint32_t height) {
        if (width | height)
            this->m_swapChain.resize({0, 0});
    });

    m_window.setMinimizeCallback([this](Window &window) {
        this->m_isPaused = true;
        window.title("HelloWorld - Paused");
    });

    m_window.setRestoreCallback([this](Window &) { this->m_isPaused = false; });

    // Upload vertex data to vertex buffer.
    m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    m_commandBuffer.copyBuffer(VERTICES, m_vertexBuffer, 0, sizeof(VERTICES));
    m_commandBuffer.transition(m_vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    m_commandBuffer.submit();

    // Create graphics pipeline.
    DXGI_FORMAT renderTargetFormat = m_swapChain.pixelFormat();
    m_pipelineState                = m_renderDevice.newGraphicsPipeline(
        m_rootSignature, {g_vertex_main, sizeof(g_vertex_main)},
        {g_pixel_main, sizeof(g_pixel_main)}, {}, std::size(INPUT_ELEMENTS), INPUT_ELEMENTS, 1,
        &renderTargetFormat, DXGI_FORMAT_UNKNOWN, D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);

    // Set swap chain back buffer clear color.
    m_swapChain.setClearColor(colors::White);
}

Application::~Application() noexcept = default;

auto Application::run() -> void {
    QueryPerformanceFrequency(&m_countsPerSecond);
    QueryPerformanceCounter(&m_lastUpdate);
    m_lastFpsUpdate = m_lastUpdate;
    MSG msg{};
    while (!m_window.isClosed()) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        } else if (!m_isPaused) {
            update();
        }
    }
}

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

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (now.QuadPart - m_lastFpsUpdate.QuadPart > m_countsPerSecond.QuadPart) {
        const float fps = static_cast<float>(m_countsPerSecond.QuadPart) /
                          static_cast<float>(now.QuadPart - m_lastUpdate.QuadPart);
        std::string newTitle("HelloWorld - ");
        newTitle.append(std::to_string(fps));
        newTitle.append(" FPS");
        m_window.title(newTitle);

        m_lastFpsUpdate = now;
    }

    m_lastUpdate = now;
}

auto main() -> int {
    try {
        auto app = std::make_unique<Application>();
        app->run();
    } catch (Exception &e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    }
}

#include <ink/core/log.h>
#include <ink/core/window.h>
#include <ink/render/command_buffer.h>
#include <ink/render/swap_chain.h>

using namespace ink;

class Application {
public:
    Application() noexcept
        : m_mainWindow(u"ink", 800, 600), m_swapChain(m_mainWindow), m_commandBuffer() {}

    ~Application() noexcept = default;

    auto run() -> void;
    auto update() -> void;

private:
    Window        m_mainWindow;
    SwapChain     m_swapChain;
    CommandBuffer m_commandBuffer;
};

auto Application::run() -> void {
    MSG msg{};
    while (!m_mainWindow.isClosed()) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        } else {
            update();
        }
    }
}

auto Application::update() -> void {
    auto &backBuffer = m_swapChain.backBuffer();

    m_commandBuffer.transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandBuffer.setRenderTarget(backBuffer);
    m_commandBuffer.clearColor(backBuffer, colors::Red);
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

    auto app = std::make_unique<Application>();
    app->run();

    return 0;
}

#include "ink/core/exception.hpp"
#include "ink/core/window.hpp"
#include "ink/render/command_buffer.hpp"
#include "ink/render/device.hpp"

using namespace ink;

class Application {
public:
    Application();
    ~Application() noexcept;

    auto run() -> void;
    auto update() -> void;

private:
    RenderDevice  m_renderDevice;
    Window        m_window;
    SwapChain     m_swapChain;
    CommandBuffer m_commandBuffer;
};

Application::Application()
    : m_renderDevice(),
      m_window("HelloWorld", 800, 600, WindowStyle::Titled | WindowStyle::Resizable),
      m_swapChain(m_renderDevice.newSwapChain(m_window)),
      m_commandBuffer(m_renderDevice.newCommandBuffer()) {
    m_window.setFrameResizeCallback(
        [this](Window &, std::uint32_t, std::uint32_t) {
            this->m_swapChain.resize({0, 0});
        });
}

Application::~Application() noexcept = default;

auto Application::run() -> void {
    MSG msg{};
    while (!m_window.isClosed()) {
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
    backBuffer.clearColor(colors::White);

    RenderPass renderPass{};
    renderPass.renderTargetCount             = 1;
    renderPass.renderTargets[0].renderTarget = &backBuffer;
    renderPass.renderTargets[0].loadAction   = LoadAction::Clear;
    renderPass.renderTargets[0].storeAction  = StoreAction::Store;
    renderPass.renderTargets[0].stateBefore  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    renderPass.renderTargets[0].stateAfter   = D3D12_RESOURCE_STATE_PRESENT;

    m_commandBuffer.beginRenderPass(renderPass);
    m_commandBuffer.endRenderPass();
    m_commandBuffer.submit();
    m_swapChain.present();
}

auto main() -> int {
    try {
        auto app = std::make_unique<Application>();
        app->run();
    } catch (Exception &e) {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
    }
}

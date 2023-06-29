#include "ink/core/assert.h"
#include "ink/core/exception.h"
#include "ink/render/device.h"
#include "ink/core/window.h"

using namespace ink;

class TestWindow final : public Window {
public:
    using Window::Window;

    auto onKey(KeyCode key, KeyAction action, Modifier mods) -> void override {
        if (key == KeyCode::Escape)
            this->close();
    }
};

auto main() -> int {
    Logger::singleton().logLevel(LogLevel::Trace);
    addLogMessageHandler<VisualStudioDebugLogMessageHandler>();
    addLogMessageHandler<ConsoleLogMessageHandler>();

    RenderDevice& device = RenderDevice::singleton();
    logInfo(u"DXR support: {}.", device.supportRayTracing());

    TestWindow testWindow(u"ink", 800, 600, WindowStyle::Default | WindowStyle::Resizable);

    MSG msg{};
    while (!testWindow.isClosed()) {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return 0;
}

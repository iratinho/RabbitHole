#include "Renderer/Ultralight/UltralightRenderer.hpp"

#include <window.hpp>

#include "Ultralight/Ultralight.h"
#include "AppCore/AppCore.h"
#include "Renderer/Device.hpp"
#include "Renderer/Texture2D.hpp"

class PageLoadListener : public ultralight::LoadListener {
public:
    void OnDOMReady(ultralight::View *caller, uint64_t frame_id, bool is_main_frame,
                    const ultralight::String &url) override {
        std::cout << "[Info]: Main html page loaded. (" << url.utf8().data() << ")" << std::endl;
    }

    void OnFailLoading(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url,
                       const ultralight::String &description, const ultralight::String &error_domain,
                       int error_code) override {
        std::cerr << "[Error]: " << description.utf8().data() << "\n" << "error_domain: " << error_domain.utf8().data()
                << "\n" << "error_code: " << error_code << std::endl;
    }

    void OnFinishLoading(ultralight::View *caller, uint64_t frame_id, bool is_main_frame,
                         const ultralight::String &url) override {
        ultralight::Surface *surface = caller->surface();
        caller->set_needs_paint(true);
    }
};

class ViewListener : public ultralight::ViewListener {
public:
    void OnChangeTitle(ultralight::View *caller, const ultralight::String &title) override {
        std::cout << "[Info]: Page Title as changed to " << title.utf8().data() << std::endl;
    }

    void OnChangeURL(ultralight::View *caller, const ultralight::String &url) override {
        std::cout << "[Info]: Page URL as changed to " << url.utf8().data() << std::endl;
    }

    void OnChangeTooltip(ultralight::View *caller, const ultralight::String &tooltip) override {
    }

    void OnChangeCursor(ultralight::View *caller, ultralight::Cursor cursor) override {
    }

    void OnAddConsoleMessage(ultralight::View *caller,
                             const ultralight::ConsoleMessage &message) override {
        std::cout << "[Info]: " << message.message().utf8().data() << std::endl;
    }
};

bool UltralightRenderer::Initialize(Device *device) {
    _device = device;
    _window = _device ? device->GetWindow() : nullptr;

    if (!_window) {
        return false;
    }


    ultralight::Config config;
    //config.resource_path_prefix = "./resources/";
    //config.use_gpu_renderer = false;
    //config.device_scale = 1.0;


    // Get the absolute path to the current working directory
    std::filesystem::path current_path = std::filesystem::current_path();

    // Construct absolute paths for assets and log file
    std::string assets_path = (current_path / "assets").string();
    std::string log_path = (current_path / "ultralight.log").string();

    std::cout << assets_path << std::endl;
    std::cout << log_path << std::endl;

    ultralight::Platform::instance().set_config(config);
    ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());
    ultralight::Platform::instance().set_file_system(ultralight::GetPlatformFileSystem(assets_path.c_str()));
    ultralight::Platform::instance().set_logger(ultralight::GetDefaultLogger(log_path.c_str()));

    // Initialize the renderer
    _renderer = ultralight::Renderer::Create();

    ultralight::ViewConfig viewConfig;
    viewConfig.initial_device_scale = 1.0;
    viewConfig.is_accelerated = false;
    viewConfig.is_transparent = true;

    _view = _renderer->CreateView(_window->GetWindowSurfaceSize().x, _window->GetWindowSurfaceSize().y, viewConfig,
                                  nullptr);

    auto *listener = new PageLoadListener;
    auto *viewListener = new ViewListener;
    _view->set_load_listener(listener);
    _view->set_view_listener(viewListener);
    _view->LoadURL(R"(file:///index.html)");
    _view->Focus();

    // For some reason we are only able to correctly load the page if we call this multiple times during initialization
    for (int i = 0; i < 50; ++i) {
        PumpMessages();
    }

    // Force first time render
    _renderer->Render();

    _window->GetWindowResizeDelegate().AddLambda([this](glm::i32vec2 newSize) {
        _view->Resize(newSize.x, newSize.y);
        _bIsDirty = true;
    });

    _window->GetMousePosDelegate().AddLambda([this](glm::vec2 pos) {
        // TODO create a way to detect if the mouse is over the UI area so that we dont trigger updates when moving
        // in the viewport
        ultralight::MouseEvent evt {};
        evt.type = ultralight::MouseEvent::kType_MouseMoved;
        evt.x = (int)pos.x;
        evt.y = (int)pos.y;
        evt.button = ultralight::MouseEvent::kButton_None;
        _view->FireMouseEvent(evt);

        _bIsDirty = true;
    });


    return true;
}

bool UltralightRenderer::Render() {
    /*
    const auto view = registry.view<UserInterfaceComponent>();

    for (auto entity : view) {
        auto& userInterfaceComponent = view.get<UserInterfaceComponent>(entity);
        userInterfaceComponent._uiRenderTarget = _uiRenderTarget;
    }
    */

    PumpMessages();

    ultralight::Surface *surface = _view->surface();

    if (_bIsDirty || (!surface->dirty_bounds().IsEmpty() && !_view->is_loading())) {
        _renderer->Render();
        surface->ClearDirtyBounds();

        /*
        auto* bitmap_surface = dynamic_cast<ultralight::BitmapSurface*>(surface);
        const auto bitmap = bitmap_surface->bitmap();
        const auto* pixels = static_cast<const uint32_t*>(bitmap->LockPixels());
        _uiRenderTarget->GetTexture()->WritePixelData((void*)pixels, bitmap->size());
        */
        // int width = bitmap->width();
        // int height = bitmap->height();
        // uint32_t stride = bitmap_surface->row_bytes();

        // for (uint32_t y = 0; y < height; ++y) {
        //     for (uint32_t x = 0; x < width; ++x) {
        //         const uint32_t* pixel = pixels + y * (stride / sizeof(uint32_t)) + x;
        //         uint8_t b = static_cast<uint8_t>(*pixel);
        //         uint8_t g = static_cast<uint8_t>(*pixel >> 8);
        //         uint8_t r = static_cast<uint8_t>(*pixel >> 16);
        //         uint8_t a = static_cast<uint8_t>(*pixel >> 24);
        //         printf("(%u,%u): %02X %02X %02X %02X\n", x, y, b, g, r, a);
        //     }
        // }

        //bitmap->UnlockPixels();

        _bIsDirty = false;
        return true;
    }

    return false;
}

const void *UltralightRenderer::LockPixels() const {
    ultralight::Surface *surface = _view->surface();
    auto *bitmap_surface = dynamic_cast<ultralight::BitmapSurface *>(surface);
    const auto bitmap = bitmap_surface->bitmap();
    const auto *pixels = static_cast<const uint32_t *>(bitmap->LockPixels());
    return pixels;

    return {};
}

void UltralightRenderer::UnlockPixels() const {
    ultralight::Surface *surface = _view->surface();
    auto *bitmap_surface = dynamic_cast<ultralight::BitmapSurface *>(surface);
    const auto bitmap = bitmap_surface->bitmap();
    bitmap->UnlockPixels();
}

std::size_t UltralightRenderer::PixelBufferSize() const {
    ultralight::Surface *surface = _view->surface();
    auto *bitmap_surface = dynamic_cast<ultralight::BitmapSurface *>(surface);
    const auto bitmap = bitmap_surface->bitmap();
    return bitmap->size();

    return {};
}

void UltralightRenderer::PumpMessages() const {
    if (_renderer) {
        _renderer->Update();
    }
}

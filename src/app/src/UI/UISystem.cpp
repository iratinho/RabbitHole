#include "UI/UISystem.h"
#include "window.h"
#include "Renderer/render_context.h"
#include "Ultralight/Ultralight.h"
#include "AppCore/AppCore.h"
#include "Core/Components/UserInterfaceComponent.h"
#include "Renderer/RenderTarget.h"
#include "Renderer/Texture.h"

class PageLoadListener : public ultralight::LoadListener
{
public:
    void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override
    {
        std::cout << "[Info]: Main html page loaded. (" << url.utf8().data() << ")" << std::endl;
    }
    void OnFailLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url, const ultralight::String& description, const ultralight::String& error_domain, int error_code) override
    {
        std::cerr << "[Error]: " << description.utf8().data() << "\n" << "error_domain: " << error_domain.utf8().data() << "\n" << "error_code: " << error_code << std::endl;   
    }
    void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override
    {
        ultralight::Surface* surface = caller->surface();
        caller->set_needs_paint(true);
    }
};

class ViewListener : public ultralight::ViewListener
{
public:
    void OnChangeTitle(ultralight::View* caller, const ultralight::String& title) override
    {
        std::cout << "[Info]: Page Title as changed to " << title.utf8().data() << std::endl;   
    }
    
    void OnChangeURL(ultralight::View* caller, const ultralight::String& url) override
    {
        std::cout << "[Info]: Page URL as changed to " << url.utf8().data() << std::endl;
    }
    void OnChangeTooltip(ultralight::View* caller, const ultralight::String& tooltip) override {}
    void OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) override {}
    void OnAddConsoleMessage(ultralight::View* caller, ultralight::MessageSource source, ultralight::MessageLevel level,
        const ultralight::String& message, uint32_t line_number, uint32_t column_number,
        const ultralight::String& source_id) override
    {
        std::cout << "[Info]: " << message.utf8().data() << std::endl;
    }
};

bool UISystem::Initialize(RenderContext* renderContext, InitializationParams initialization_params)
{
    _renderContext = renderContext;
    _window = initialization_params.window_;

    if(!_window || !_renderContext) {
        return false;
    }
    
    ultralight::Config config;
    config.resource_path = "./resources";
    config.use_gpu_renderer = false;
    config.device_scale = 1.0;

    ultralight::Platform::instance().set_config(config);
    ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());
    ultralight::Platform::instance().set_file_system(ultralight::GetPlatformFileSystem("./ultralight/assets/"));
    ultralight::Platform::instance().set_logger(ultralight::GetDefaultLogger("./ultralight.log"));
    
    // Initialize the renderer
    _renderer = ultralight::Renderer::Create();
    
    _view = _renderer->CreateView(_window->GetFramebufferSize().width, _window->GetFramebufferSize().height, true, nullptr);
    auto* listener = new PageLoadListener;
    auto* viewListener = new ViewListener;
    _view->set_load_listener(listener);
    _view->set_view_listener(viewListener);
    _view->LoadURL(R"(file:///index.html)");
    _view->Focus();

    // Create the UI render target to where we are going to draw the html page into
    TextureParams params;
    params._width = _window->GetFramebufferSize().width;
    params._height = _window->GetFramebufferSize().height;
    params.flags = static_cast<TextureUsageFlags>(Tex_SAMPLED_OP | Tex_TRANSFER_DEST_OP);
    params.format = VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
    _uiRenderTarget = std::make_unique<RenderTarget>(renderContext, RenderTargetParams(params));
    _uiRenderTarget->Initialize();

    // For some reason we are only able to correctly load the page if we call this multiple times during initialization
    for (int i = 0; i < 5; ++i) {
        PumpMessages();
    }

    // Force first time render
    _renderer->Render();
    
    return true;
}

bool UISystem::Process(entt::registry& registry)
{
    const auto view = registry.view<UserInterfaceComponent>();

    for (auto entity : view) {
        auto& userInterfaceComponent = view.get<UserInterfaceComponent>(entity);
        userInterfaceComponent._uiRenderTarget = _uiRenderTarget;
    }
    
    PumpMessages();

    ultralight::Surface* surface = _view->surface();
    
    if (!surface->dirty_bounds().IsEmpty() && !_view->is_loading())
    {
        _renderer->Render();

        auto* bitmap_surface = dynamic_cast<ultralight::BitmapSurface*>(surface);
        const auto bitmap = bitmap_surface->bitmap();
        const auto* pixels = static_cast<const uint32_t*>(bitmap->LockPixels());
        _uiRenderTarget->GetTexture()->WritePixelData((void*)pixels, bitmap->size());

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

        bitmap->UnlockPixels();
        surface->ClearDirtyBounds();
    }
    
    return true;
}

void UISystem::PumpMessages() const
{
    if(_renderer) {
        _renderer->Update();
    }
}

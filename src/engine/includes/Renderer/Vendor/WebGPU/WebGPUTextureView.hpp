#pragma once
#include "Renderer/TextureView.hpp"
#include "webgpu/webgpu.hpp"

class WebGPUTextureView final : public TextureView {
public:
    using TextureView::TextureView;

    ~WebGPUTextureView() override {
        FreeView();
    }
    
    void CreateView(Format format, const Range &levels, TextureType textureType) override;
    void FreeView() override;

    [[nodiscard]] WGPUTextureView GetWebGPUTextureView() const {
        return _view;
    };

private:
    WGPUTextureView _view = nullptr;
};

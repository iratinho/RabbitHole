#pragma once
#include "Renderer/TextureResource.hpp"
#include "webgpu/webgpu.hpp"

class WebGPUTextureResource final : public TextureResource {
public:
    using TextureResource::TextureResource;
    ~WebGPUTextureResource() override {
        WebGPUTextureResource::FreeResource();
    };

    void CreateResource() override;
    void SetExternalResource(void *handle) override;
    void FreeResource() override;
    bool HasValidResource() override;
    void * Lock() override;
    void Unlock() override;

    [[nodiscard]] WGPUTexture GetWGPUTexture() const {
        return _nativeTexture;
    };

private:
    WGPUTexture _nativeTexture = nullptr;
};

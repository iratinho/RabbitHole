#pragma once

enum ESwapchainTextureType_ {
    _COLOR,
    _DEPTH
};

class Texture2D;
class Event;
class Device;

class Swapchain {
public:
    explicit Swapchain(Device* device);
    virtual ~Swapchain() = default;

    static std::unique_ptr<Swapchain> MakeSwapchain(Device* device);

public:
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    void MarkDirty();
    [[nodiscard]] bool IsDirty() const { return _isDirty; }

    virtual bool PrepareNextImage() = 0;
    virtual std::uint8_t GetImageCount() { return 1; }
    virtual std::shared_ptr<Texture2D> GetTexture(ESwapchainTextureType_ type) = 0;

    virtual std::shared_ptr<Event> GetSyncEvent() = 0;

protected:
    bool _isDirty = true;
    Device* _device = nullptr;
};

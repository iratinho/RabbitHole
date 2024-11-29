#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"

#include "window.hpp"
#include "webgpu/webgpu.hpp"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

namespace {
    void HandleWebGPUError(WGPUErrorType type, const char* message, void* userdata) {
        printf("WebGPU Error [%d]: %s\n", type, message);
    }
}

bool WebGPUDevice::Initialize() {
#ifdef __EMSCRIPTEN__
    // Descriptor is not yet implemented in the browser implementation
    _instance = wgpuCreateInstance(nullptr);
#else
    WGPUInstanceDescriptor descriptor {};
    descriptor.nextInChain = nullptr;

    _instance = wgpuCreateInstance(&descriptor);
#endif

    if (_instance == nullptr) {
        return false;
    }

    _adapter = CreateAdapter(_instance);
    if (_adapter == nullptr) {
        return false;
    }

    GetAdapterSupportedLimits(_adapter);
    GetAdapterSupportedFeatures(_adapter);
    GetAdapterProperties(_adapter);

    _device = CreateDevice(_adapter);
    if (_device == nullptr) {
        return false;
    }

    ReportDevice(_device);

    if(!GetWindow()) {
        assert(0 && "WebGPUDevice::Initialize failed to create surface.");
        return false;
    }

    GetWindow()->CreateSurface(nullptr);
    
    wgpuDeviceSetUncapturedErrorCallback(_device, HandleWebGPUError, nullptr);

    return Device::Initialize();
}

void WebGPUDevice::Shutdown() {
    if(_device) {
        wgpuDeviceRelease(_device);
    }

    if(_adapter) {
        wgpuAdapterRelease(_adapter);
    }
}

WGPUAdapter WebGPUDevice::CreateAdapter(WGPUInstance instance) {
    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * pUserData) {
        UserData& userData = *static_cast<UserData*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        } else {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuInstanceRequestAdapter(
        instance,
        nullptr,
        onAdapterRequestEnded,
        &userData);

#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__
    //assert(userData.requestEnded);

    return userData.adapter;
}

WGPUSupportedLimits WebGPUDevice::GetAdapterSupportedLimits(WGPUAdapter adapter) {    
    if (adapter == nullptr) {
        return {};
    }
    
    WGPUSupportedLimits limits;
    limits.nextInChain = nullptr;

#ifdef __EMSCRIPTEN__
	// Error in Chrome: Aborted(TODO: wgpuAdapterGetLimits unimplemented)
	// (as of September 4, 2023), so we hardcode values:
	// These work for 99.95% of clients (source: https://web3dsurvey.com/webgpu)
	limits.limits.minStorageBufferOffsetAlignment = 256;
	limits.limits.minUniformBufferOffsetAlignment = 256;

    return limits;
#else
    if(wgpuAdapterGetLimits(adapter, &limits)) {
        std::cout << "Adapter limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;

        return limits;
    }
#endif

    return {};
}

//EMSCRIPTEN_KEEPALIVE
std::vector<WGPUFeatureName> WebGPUDevice::GetAdapterSupportedFeatures(WGPUAdapter adapter) {
    std::size_t numFeatures = wgpuAdapterEnumerateFeatures(adapter, 0);

    std::vector<WGPUFeatureName> features;
    features.resize(numFeatures);

    wgpuAdapterEnumerateFeatures(adapter, features.data());

    return features;
}

WGPUAdapterProperties WebGPUDevice::GetAdapterProperties(WGPUAdapter adapter) {
    WGPUAdapterProperties properties {};
    properties.nextInChain = nullptr;

    wgpuAdapterGetProperties(adapter, &properties);

    std::cout << "Adapter properties:" << std::endl;
    std::cout << " - vendorID: " << properties.vendorID << std::endl;
    // if (properties.vendorName) {
    //     std::cout << " - vendorName: " << properties.vendorName << std::endl;
    // }
    if (properties.architecture) {
        std::cout << " - architecture: " << properties.architecture << std::endl;
    }
    // std::cout << " - deviceID: " << properties.deviceID << std::endl;
    // if (properties.name) {
    //     std::cout << " - name: " << properties.name << std::endl;
    // }
    // if (properties.driverDescription) {
    //     std::cout << " - driverDescription: " << properties.driverDescription << std::endl;
    // }
    std::cout << std::hex;
    std::cout << " - adapterType: 0x" << properties.adapterType << std::endl;
    std::cout << " - backendType: 0x" << properties.backendType << std::endl;
    std::cout << std::dec; // Restore decimal numbers

    return properties;
}

WGPUDevice WebGPUDevice::CreateDevice(WGPUAdapter adapter) {
    WGPUDeviceDescriptor descriptor {};
    descriptor.label = "WebGPUDevice";
    descriptor.defaultQueue.label = "DefaultQueue";
    descriptor.requiredFeatures = nullptr;
    descriptor.requiredFeatures = nullptr;
    //descriptor.requiredFeaturesCount = 0;
    descriptor.deviceLostCallback = nullptr;
    descriptor.deviceLostUserdata = nullptr;
    descriptor.nextInChain = nullptr;

    struct DeviceRequestData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };

    DeviceRequestData requestData;

    auto onAdapterRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const * message, void * pUserData) {
        DeviceRequestData& requestData = *static_cast<DeviceRequestData*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            requestData.device = device;
        } else {
            std::cout << "Could not get WebGPU device: " << message << std::endl;
        }
        requestData.requestEnded = true;
    };

    wgpuAdapterRequestDevice(adapter, nullptr, onAdapterRequestEnded, &requestData);

#ifdef __EMSCRIPTEN__
    while (!requestData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__


    //assert(requestData.requestEnded);

    return requestData.device;
}

// Function Taken from LearnWebGPU
void WebGPUDevice::ReportDevice(WGPUDevice device) {
	std::vector<WGPUFeatureName> features;
	size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
	features.resize(featureCount);
	wgpuDeviceEnumerateFeatures(device, features.data());

	std::cout << "Device features:" << std::endl;
	std::cout << std::hex;
	for (auto f : features) {
		std::cout << " - 0x" << f << std::endl;
	}
	std::cout << std::dec;

	WGPUSupportedLimits limits = {};
	limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
	bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
	bool success = wgpuDeviceGetLimits(device, &limits);
#endif

	if (success) {
		std::cout << "Device limits:" << std::endl;
		std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
		std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
		std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
		std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
		std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups << std::endl;
		std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: " << limits.limits.maxDynamicUniformBuffersPerPipelineLayout << std::endl;
		std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: " << limits.limits.maxDynamicStorageBuffersPerPipelineLayout << std::endl;
		std::cout << " - maxSampledTexturesPerShaderStage: " << limits.limits.maxSampledTexturesPerShaderStage << std::endl;
		std::cout << " - maxSamplersPerShaderStage: " << limits.limits.maxSamplersPerShaderStage << std::endl;
		std::cout << " - maxStorageBuffersPerShaderStage: " << limits.limits.maxStorageBuffersPerShaderStage << std::endl;
		std::cout << " - maxStorageTexturesPerShaderStage: " << limits.limits.maxStorageTexturesPerShaderStage << std::endl;
		std::cout << " - maxUniformBuffersPerShaderStage: " << limits.limits.maxUniformBuffersPerShaderStage << std::endl;
		std::cout << " - maxUniformBufferBindingSize: " << limits.limits.maxUniformBufferBindingSize << std::endl;
		std::cout << " - maxStorageBufferBindingSize: " << limits.limits.maxStorageBufferBindingSize << std::endl;
		std::cout << " - minUniformBufferOffsetAlignment: " << limits.limits.minUniformBufferOffsetAlignment << std::endl;
		std::cout << " - minStorageBufferOffsetAlignment: " << limits.limits.minStorageBufferOffsetAlignment << std::endl;
		std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers << std::endl;
		std::cout << " - maxVertexAttributes: " << limits.limits.maxVertexAttributes << std::endl;
		std::cout << " - maxVertexBufferArrayStride: " << limits.limits.maxVertexBufferArrayStride << std::endl;
		std::cout << " - maxInterStageShaderComponents: " << limits.limits.maxInterStageShaderComponents << std::endl;
		std::cout << " - maxComputeWorkgroupStorageSize: " << limits.limits.maxComputeWorkgroupStorageSize << std::endl;
		std::cout << " - maxComputeInvocationsPerWorkgroup: " << limits.limits.maxComputeInvocationsPerWorkgroup << std::endl;
		std::cout << " - maxComputeWorkgroupSizeX: " << limits.limits.maxComputeWorkgroupSizeX << std::endl;
		std::cout << " - maxComputeWorkgroupSizeY: " << limits.limits.maxComputeWorkgroupSizeY << std::endl;
		std::cout << " - maxComputeWorkgroupSizeZ: " << limits.limits.maxComputeWorkgroupSizeZ << std::endl;
		std::cout << " - maxComputeWorkgroupsPerDimension: " << limits.limits.maxComputeWorkgroupsPerDimension << std::endl;
	}
}

void WebGPUDevice::WaitForRequest() {
#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__
}

#include "Renderer/Vendor/WebGPU/WebGPUShader.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"

bool WebGPUShader::Compile() {
    // TODO Move to upstream
    _path = _params._shaderPath;
    
    const WebGPUDevice* wgpuDevice = (WebGPUDevice*)_device;
    if(!wgpuDevice) {
        assert(false && "Invalid gpu device when compiling shaders.");
        return false;
    }
    
    std::ifstream shader_file;
    shader_file.open(_path, std::ios::binary);

    if (!shader_file.is_open()) {
        return {};
    }

    std::stringstream shader_buffer;
    shader_buffer << shader_file.rdbuf();

    const std::string shaderCode = shader_buffer.str();

    WGPUShaderModuleWGSLDescriptor wgslShaderModuleDesc {};
    wgslShaderModuleDesc.code = shaderCode.c_str();
    wgslShaderModuleDesc.chain.sType = WGPUSType::WGPUSType_ShaderModuleWGSLDescriptor;
    wgslShaderModuleDesc.chain.next = nullptr;
    
//    WGPUShaderModuleGLSLDescriptor wgslShaderModuleDesc {};
//    wgslShaderModuleDesc.code = shaderCode.c_str();
//    wgslShaderModuleDesc.stage = TranslateShaderStage(_stage);
//    wgslShaderModuleDesc.chain.sType = WGPUSType::WGPUSType_ShaderModuleWGSLDescriptor;
//    wgslShaderModuleDesc.chain.next = nullptr;
  
//    WGPUShaderModuleSPIRVDescriptor wgslShaderModuleDesc {};
//    wgslShaderModuleDesc.code = (uint32_t*)shaderCode.data();
//    wgslShaderModuleDesc.codeSize = shaderCode.size();
//    wgslShaderModuleDesc.chain.sType = WGPUSType::WGPUSType_ShaderModuleSPIRVDescriptor;
//    wgslShaderModuleDesc.chain.next = nullptr;

    WGPUShaderModuleDescriptor shaderModuleDesc {};
    shaderModuleDesc.nextInChain = &wgslShaderModuleDesc.chain;
    
    _module = wgpuDeviceCreateShaderModule(wgpuDevice->GetWebGPUDevice(), &shaderModuleDesc);
    
    
    if(!_module) {
        assert(false && "Faild to create shader module.");
        return false;
    }
    
//    WGPUCompilationInfoCallback callback = [](WGPUCompilationInfoRequestStatus status, struct WGPUCompilationInfo const * compilationInfo, void * userdata) {
//        if (status == WGPUCompilationInfoRequestStatus_Error) {
//          for (uint32_t m = 0; m < compilationInfo->messageCount; ++m) {
//              WGPUCompilationMessage message = compilationInfo->messages[m];
//              std::cout << "lineNum: " << message.lineNum << std::endl;
//              std::cout << "linePos: " << message.linePos << std::endl;
//              std::cout << "Error: " << message.message << std::endl;
//          }
//        }
//    };
//
//    wgpuShaderModuleGetCompilationInfo(_module, callback, nullptr);

    return true;
}

#include <dawn/dawn_proc.h>
#include <dawn/dawn_wsi.h>
#include <dawn_native/DawnNative.h>
#include <dawn/webgpu_cpp.h>

#include<memory>
#include<algorithm>
#include<string>
#include <array>
#include <cstring>
#include <random>
#include<iostream>
#include<unistd.h>
#include <shaderc/shaderc.hpp>
#include <utils/ComboRenderPipelineDescriptor.h>
#include <utils/SystemUtils.h>
#include <utils/WGPUHelpers.h>
// #include<mutex>

static std::unique_ptr<dawn_native::Instance> instance;

wgpu::Device device;
wgpu::Queue queue;

static wgpu::BackendType backendType = wgpu::BackendType::Vulkan;
wgpu::Buffer resultBuffer;
std::array<wgpu::Buffer, 2> inputsBuffer;

wgpu::BindGroupLayout bgl;
wgpu::BindGroup bg;
wgpu::PipelineLayout plLayout;
wgpu::ShaderModule module;
wgpu::ComputePipeline pl;
wgpu::CommandEncoder encoder;
wgpu::ComputePassEncoder pass;

void PrintDeviceError(WGPUErrorType errorType,  const char* message, void*) {
    std::string errorTypeName = "";
    switch (errorType) {
        case WGPUErrorType_Validation:
            errorTypeName = "WGPUErrorTyp Validation";
            break;
        case WGPUErrorType_OutOfMemory:
            errorTypeName = "WGPUErrorTyp Out of memory";
            break;
        case WGPUErrorType_Unknown:
            errorTypeName = "WGPUErrorTyp Unknown";
            break;
        case WGPUErrorType_DeviceLost:
            errorTypeName = "WGPUErrorTyp Device lost";
            break;
        default: WGPUErrorType_Unknown:
            errorTypeName = "WGPUErrorTyp Unknown";
            return;
    }
    std::cout<<"device create error:" << errorTypeName<<std::endl;
}

wgpu::Device createCppDawnDevice() {
    instance = std::make_unique<dawn_native::Instance>();
    instance->DiscoverDefaultAdapters();
    // Get an adapter for the backend to use, and create the device.
    dawn_native::Adapter backendAdapter;
    {
        std::vector<dawn_native::Adapter> adapters = instance->GetAdapters();
        auto adapterIt = std::find_if(adapters.begin(), adapters.end(),
                                    [](const dawn_native::Adapter adapter) -> bool {
                                        wgpu::AdapterProperties properties;
                                        adapter.GetProperties(&properties);
                                        return properties.backendType == backendType;
                                    });
        backendAdapter = *adapterIt;
    }
    WGPUDevice backendDevice = backendAdapter.CreateDevice();
    DawnProcTable backendProcs = dawn_native::GetProcs();
    WGPUDevice cDevice = nullptr;
    DawnProcTable procs;
    procs = backendProcs;
    cDevice = backendDevice;
    dawnProcSetProcs(&procs);
    procs.deviceSetUncapturedErrorCallback(cDevice, PrintDeviceError, nullptr);
    return wgpu::Device::Acquire(cDevice);
}


void WaitABit() {
    device.Tick();
    usleep(100);
}

class BufferReadAsync{
public:
static void BufferMapReadCallback(WGPUBufferMapAsyncStatus status,
                                   const void* data,
                                   uint64_t,
                                   void* userdata)
{
    static_cast<BufferReadAsync*>(userdata)->mappedData = data;
}

const void* MapReadAsyncAndWait(const wgpu::Buffer& buffer) {
    buffer.MapReadAsync(BufferMapReadCallback, this);
    while(mappedData == nullptr) {
        WaitABit();
    }
    return mappedData;
}
void UnmapBuffer(const wgpu::Buffer& buffer) {
    buffer.Unmap();
    mappedData = nullptr;
}
private:
const void * mappedData = nullptr;
};

class BufferWriteAsync{
public:
static void BufferMapWriteCallback(WGPUBufferMapAsyncStatus status,
                                   void* data,
                                   uint64_t,
                                   void* userdata)
{
    userdata = data;
}
void* MapWriteAsyncAndWait(const wgpu::Buffer& buffer) {
    buffer.MapWriteAsync(BufferMapWriteCallback, this);
    while(mappedData == nullptr){
        WaitABit();
    }
    void* resultPointer = mappedData;
    mappedData = nullptr;
    return resultPointer;
}
void UnmapBuffer(const wgpu::Buffer& buffer) {
    buffer.Unmap();
    mappedData = nullptr;
}
private:
void * mappedData;
};

wgpu::Buffer CreateBufferFromData(const wgpu::Device& device,
                                    const void* data,
                                    uint64_t size,
                                    wgpu::BufferUsage usage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = size;
    descriptor.usage = usage | wgpu::BufferUsage::CopyDst;

    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
    buffer.SetSubData(0, size, data);
    return buffer;
}

wgpu::CreateBufferMappedResult CreateBufferMappedFromData(const wgpu::Device& device,
                                                        const void* data,
                                                        size_t size,
                                                        wgpu::BufferUsage usage){
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = size;
    descriptor.usage = usage | wgpu::BufferUsage::CopyDst;
    wgpu::CreateBufferMappedResult result = device.CreateBufferMapped(&descriptor);
    memcpy(result.data, data, size);
    return result;
}

void initBuffers(wgpu::Device& device){
    float const inputData1[] = {2.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float const inputData2[] = {4.0f, 2.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float const outputData[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    inputsBuffer[0] = CreateBufferMappedFromData(device, inputData1, sizeof(inputData1), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst).buffer;
    inputsBuffer[1] = CreateBufferMappedFromData(device, inputData2, sizeof(inputData2), wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst).buffer;
    wgpu::BufferDescriptor desc;
    desc.size = sizeof(outputData);
    desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    resultBuffer = device.CreateBuffer(& desc);
    inputsBuffer[0].Unmap();
    inputsBuffer[1].Unmap();

    //3 differenct ways to set buffer data tfjs use setSubData

    // for(int i = 0; i < 2; i++) {
    //     wgpu::BufferDescriptor desc;
    //     desc.size = sizeof(inputData1);
    //     desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    //     inputsBuffer[i] = device.CreateBuffer(&desc);
    // }

    // wgpu::BufferDescriptor desc;
    // desc.size = sizeof(outputData);
    // desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    // resultBuffer = device.CreateBuffer(&desc);

    // BufferWriteAsync tmp;
    // void* mappedData = tmp.MapWriteAsyncAndWait(inputsBuffer[0]);
    // memcpy(mappedData, inputData1, sizeof(inputData1));
    // tmp.UnmapBuffer(inputsBuffer[0]);
}

wgpu::BindGroupLayout MakeBindGroupLayout(
    const wgpu::Device& device,
    std::initializer_list<wgpu::BindGroupLayoutEntry> entriesInitializer) {
    std::vector<wgpu::BindGroupLayoutEntry> entries;
    for (const wgpu::BindGroupLayoutEntry& entry : entriesInitializer) {
        entries.push_back(entry);
    }

    wgpu::BindGroupLayoutDescriptor descriptor;
    descriptor.entryCount = static_cast<uint32_t>(entries.size());
    descriptor.entries = entries.data();
    return device.CreateBindGroupLayout(&descriptor);
}

wgpu::PipelineLayout MakeBasicPipelineLayout(const wgpu::Device& device,
                                                const wgpu::BindGroupLayout* bindGroupLayout) {
    wgpu::PipelineLayoutDescriptor descriptor;
    if (bindGroupLayout != nullptr) {
        descriptor.bindGroupLayoutCount = 1;
        descriptor.bindGroupLayouts = bindGroupLayout;
    } else {
        descriptor.bindGroupLayoutCount = 0;
        descriptor.bindGroupLayouts = nullptr;
    }
    return device.CreatePipelineLayout(&descriptor);
}

struct BindingInitializationHelper {
    BindingInitializationHelper(uint32_t binding, const wgpu::Sampler& sampler): binding(binding), sampler(sampler) {}
    BindingInitializationHelper(uint32_t binding, const wgpu::TextureView& textureView) : binding(binding), textureView(textureView) {}
    BindingInitializationHelper(uint32_t binding,
                                const wgpu::Buffer& buffer,
                                uint64_t offset = 0,
                                uint64_t size = wgpu::kWholeSize) 
                                : binding(binding), buffer(buffer), offset(offset), size(size) {}

    wgpu::BindGroupEntry GetAsBinding() const{
        wgpu::BindGroupEntry result;

        result.binding = binding;
        result.sampler = sampler;
        result.textureView = textureView;
        result.buffer = buffer;
        result.offset = offset;
        result.size = size;

        return result;
    }
    uint32_t binding;
    wgpu::Sampler sampler;
    wgpu::TextureView textureView;
    wgpu::Buffer buffer;
    uint64_t offset = 0;
    uint64_t size = 0;
};

wgpu::BindGroup MakeBindGroup(
    const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer) {
    std::vector<wgpu::BindGroupEntry> entries;
    for (const BindingInitializationHelper& helper : entriesInitializer) {
        entries.push_back(helper.GetAsBinding());
    }

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = layout;
    descriptor.entryCount = entries.size();
    descriptor.entries = entries.data();

    return device.CreateBindGroup(&descriptor);
}

wgpu::FenceCompletionStatus WaitForCompletedValue(wgpu::Device& device, wgpu::Fence& fence, uint64_t completedValue) 
{
    if (fence.GetCompletedValue() < completedValue) 
    {
        device.Tick();
        usleep(1000);
        std::cout<<"Waif for fence"<<std::endl;
    }
    if(fence.GetCompletedValue() != completedValue)
    {
        return wgpu::FenceCompletionStatus::Error;
    }

    return wgpu::FenceCompletionStatus::Success;
}

const char* computeShaderCode = R"(#version 450

  layout(std430, set = 0, binding = 0) readonly buffer FirstMatrix {
      vec2 size;
      float numbers[];
  } firstMatrix;

  layout(std430, set = 0, binding = 1) readonly buffer SecondMatrix {
      vec2 size;
      float numbers[];
  } secondMatrix;

  layout(std430, set = 0, binding = 2) buffer ResultMatrix {
      vec2 size;
      float numbers[];
  } resultMatrix;

  void main() {
    resultMatrix.size = vec2(firstMatrix.size.x, secondMatrix.size.y);

    ivec2 resultCell = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    float result = 0.0;
    for (int i = 0; i < firstMatrix.size.y; i++) {
      int a = i + resultCell.x * int(firstMatrix.size.y);
      int b = resultCell.y + i * int(secondMatrix.size.y);
      result += firstMatrix.numbers[a] * secondMatrix.numbers[b];
    }

    int index = resultCell.y + resultCell.x * int(secondMatrix.size.y);
    resultMatrix.numbers[index] = result;
  }
)";

    // enum class SingleShaderStage { Vertex, Fragment, Compute };

    //     class CompilerSingleton {
    //       public:
    //         static shaderc::Compiler* Get() {
    //             std::call_once(mInitFlag, &CompilerSingleton::Initialize);
    //             return mCompiler;
    //         }

    //       private:
    //         CompilerSingleton() = default;
    //         ~CompilerSingleton() = default;
    //         CompilerSingleton(const CompilerSingleton&) = delete;
    //         CompilerSingleton& operator=(const CompilerSingleton&) = delete;

    //         static shaderc::Compiler* mCompiler;
    //         static std::once_flag mInitFlag;

    //         static void Initialize() {
    //             mCompiler = new shaderc::Compiler();
    //         }
    //     };


    //     wgpu::ShaderModule CreateShaderModuleFromResult(
    //         const wgpu::Device& device,
    //         const shaderc::SpvCompilationResult& result) {
    //         // result.cend and result.cbegin return pointers to uint32_t.
    //         const uint32_t* resultBegin = result.cbegin();
    //         const uint32_t* resultEnd = result.cend();
    //         // So this size is in units of sizeof(uint32_t).
    //         ptrdiff_t resultSize = resultEnd - resultBegin;
    //         // SetSource takes data as uint32_t*.

    //         wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
    //         spirvDesc.codeSize = static_cast<uint32_t>(resultSize);
    //         spirvDesc.code = result.cbegin();

    //         wgpu::ShaderModuleDescriptor descriptor;
    //         descriptor.nextInChain = &spirvDesc;

    //         return device.CreateShaderModule(&descriptor);
    //     }

int main(int /*argc*/, char** /*argv*/)
{

    device = createCppDawnDevice();
    queue = device.GetDefaultQueue();

    initBuffers(device);
    std::cout << "buffer create succeed" <<std::endl;

    bgl = MakeBindGroupLayout(        device, {
                    {0, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                    {1, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                    {2, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},
                });
    plLayout = MakeBasicPipelineLayout(device, &bgl);
    bg = MakeBindGroup(device, bgl, 
    {
        {0, inputsBuffer[0], 0, sizeof(inputsBuffer[0])},
        {1, inputsBuffer[1], 0, sizeof(inputsBuffer[1])},
        {2, resultBuffer, 0, sizeof(resultBuffer)}
    
    });
    shaderc_shader_kind kind = shaderc_glsl_compute_shader;
    module = utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, computeShaderCode);
    // shaderc::Compiler* compiler = CompilerSingleton::Get();
    // auto result = compiler->CompileGlslToSpv(computeShaderCode, strlen(computeShaderCode), kind, "myshader?");
    // module=CreateShaderModuleFromResult(device, result);
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = plLayout;
    csDesc.computeStage.module = module;
    csDesc.computeStage.entryPoint = "main";
    pl = device.CreateComputePipeline(&csDesc);
    encoder = device.CreateCommandEncoder();
    pass = encoder.BeginComputePass();
    pass.SetPipeline(pl);
    pass.SetBindGroup(0,bg);
    pass.Dispatch(2,4);
    pass.EndPass();
    wgpu::CommandBuffer cmdBuffer = encoder.Finish();
    wgpu::Fence fence = queue.CreateFence();
    queue.Signal(fence, 1);
    queue.Submit(1, &cmdBuffer);
    if(WaitForCompletedValue(device, fence, 1u) == wgpu::FenceCompletionStatus::Error) {
        std::cout<<"Fence failed "<<std::endl;
    }
    fence.Release();
    device.Release();
    std::cout<<"test1 succeed"<<std::endl;
    return 0;
}

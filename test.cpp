#include <dawn/dawn_proc.h>
#include <dawn/dawn_wsi.h>
#include <dawn_native/DawnNative.h>
#include <dawn/webgpu_cpp.h>

#include <memory>
#include <algorithm>
#include <string>
#include <array>
#include <cstring>
#include <random>
#include <iostream>
#include <unistd.h>

const char* computeShaderCode = R"(#version 450
  layout(std430, set = 0, binding = 0)  buffer FirstMatrix {
      float numbers[];
  } firstMatrix;

  layout(std430, set = 0, binding = 1)  buffer SecondMatrix {
      float numbers[];
  } secondMatrix;

  layout(std430, set = 0, binding = 2) buffer ResultMatrix {
      float numbers[];
  } resultMatrix;

  void main() {
    vec2 size0 = vec2(2, 4);
    vec2 size1 = vec2(4, 2);
    ivec2 resultCell = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
    float result = 0.0;
    for (int i = 0; i < size0.y; i++) {
      int a = i + resultCell.x * int(size0.y);
      int b = resultCell.y + i * int(size1.y);
      result += firstMatrix.numbers[a] * secondMatrix.numbers[b];
    }

    int index = resultCell.y + resultCell.x * int(size1.y);
    resultMatrix.numbers[index] = result;
  }
)";
const uint32_t shaderBuffer[651] = {
  0x07230203, 0x00010000, 0x00080001, 0x00000068, 
  0x00000000, 0x00020011, 0x00000001, 0x0006000B, 
  0x00000001, 0x4C534C47, 0x6474732E, 0x3035342E, 
  0x00000000, 0x0003000E, 0x00000000, 0x00000001, 
  0x0006000F, 0x00000005, 0x00000004, 0x6E69616D, 
  0x00000000, 0x00000016, 0x00060010, 0x00000004, 
  0x00000011, 0x00000001, 0x00000001, 0x00000001, 
  0x00030003, 0x00000002, 0x000001C2, 0x00040005, 
  0x00000004, 0x6E69616D, 0x00000000, 0x00040005, 
  0x00000009, 0x657A6973, 0x00000030, 0x00040005, 
  0x0000000D, 0x657A6973, 0x00000031, 0x00050005, 
  0x00000012, 0x75736572, 0x6543746C, 0x00006C6C, 
  0x00080005, 0x00000016, 0x475F6C67, 0x61626F6C, 
  0x766E496C, 0x7461636F, 0x496E6F69, 0x00000044, 
  0x00040005, 0x00000022, 0x75736572, 0x0000746C, 
  0x00030005, 0x00000025, 0x00000069, 0x00030005, 
  0x00000030, 0x00000061, 0x00030005, 0x00000039, 
  0x00000062, 0x00050005, 0x00000043, 0x73726946, 
  0x74614D74, 0x00786972, 0x00050006, 0x00000043, 
  0x00000000, 0x626D756E, 0x00737265, 0x00050005, 
  0x00000045, 0x73726966, 0x74614D74, 0x00786972, 
  0x00060005, 0x0000004B, 0x6F636553, 0x614D646E, 
  0x78697274, 0x00000000, 0x00050006, 0x0000004B, 
  0x00000000, 0x626D756E, 0x00737265, 0x00060005, 
  0x0000004D, 0x6F636573, 0x614D646E, 0x78697274, 
  0x00000000, 0x00040005, 0x00000057, 0x65646E69, 
  0x00000078, 0x00060005, 0x00000062, 0x75736552, 
  0x614D746C, 0x78697274, 0x00000000, 0x00050006, 
  0x00000062, 0x00000000, 0x626D756E, 0x00737265, 
  0x00060005, 0x00000064, 0x75736572, 0x614D746C, 
  0x78697274, 0x00000000, 0x00040047, 0x00000016, 
  0x0000000B, 0x0000001C, 0x00040047, 0x00000042, 
  0x00000006, 0x00000004, 0x00050048, 0x00000043, 
  0x00000000, 0x00000023, 0x00000000, 0x00030047, 
  0x00000043, 0x00000003, 0x00040047, 0x00000045, 
  0x00000022, 0x00000000, 0x00040047, 0x00000045, 
  0x00000021, 0x00000000, 0x00040047, 0x0000004A, 
  0x00000006, 0x00000004, 0x00050048, 0x0000004B, 
  0x00000000, 0x00000023, 0x00000000, 0x00030047, 
  0x0000004B, 0x00000003, 0x00040047, 0x0000004D, 
  0x00000022, 0x00000000, 0x00040047, 0x0000004D, 
  0x00000021, 0x00000001, 0x00040047, 0x00000061, 
  0x00000006, 0x00000004, 0x00050048, 0x00000062, 
  0x00000000, 0x00000023, 0x00000000, 0x00030047, 
  0x00000062, 0x00000003, 0x00040047, 0x00000064, 
  0x00000022, 0x00000000, 0x00040047, 0x00000064, 
  0x00000021, 0x00000002, 0x00020013, 0x00000002, 
  0x00030021, 0x00000003, 0x00000002, 0x00030016, 
  0x00000006, 0x00000020, 0x00040017, 0x00000007, 
  0x00000006, 0x00000002, 0x00040020, 0x00000008, 
  0x00000007, 0x00000007, 0x0004002B, 0x00000006, 
  0x0000000A, 0x40000000, 0x0004002B, 0x00000006, 
  0x0000000B, 0x40800000, 0x0005002C, 0x00000007, 
  0x0000000C, 0x0000000A, 0x0000000B, 0x0005002C, 
  0x00000007, 0x0000000E, 0x0000000B, 0x0000000A, 
  0x00040015, 0x0000000F, 0x00000020, 0x00000001, 
  0x00040017, 0x00000010, 0x0000000F, 0x00000002, 
  0x00040020, 0x00000011, 0x00000007, 0x00000010, 
  0x00040015, 0x00000013, 0x00000020, 0x00000000, 
  0x00040017, 0x00000014, 0x00000013, 0x00000003, 
  0x00040020, 0x00000015, 0x00000001, 0x00000014, 
  0x0004003B, 0x00000015, 0x00000016, 0x00000001, 
  0x0004002B, 0x00000013, 0x00000017, 0x00000000, 
  0x00040020, 0x00000018, 0x00000001, 0x00000013, 
  0x0004002B, 0x00000013, 0x0000001C, 0x00000001, 
  0x00040020, 0x00000021, 0x00000007, 0x00000006, 
  0x0004002B, 0x00000006, 0x00000023, 0x00000000, 
  0x00040020, 0x00000024, 0x00000007, 0x0000000F, 
  0x0004002B, 0x0000000F, 0x00000026, 0x00000000, 
  0x00020014, 0x0000002E, 0x0003001D, 0x00000042, 
  0x00000006, 0x0003001E, 0x00000043, 0x00000042, 
  0x00040020, 0x00000044, 0x00000002, 0x00000043, 
  0x0004003B, 0x00000044, 0x00000045, 0x00000002, 
  0x00040020, 0x00000047, 0x00000002, 0x00000006, 
  0x0003001D, 0x0000004A, 0x00000006, 0x0003001E, 
  0x0000004B, 0x0000004A, 0x00040020, 0x0000004C, 
  0x00000002, 0x0000004B, 0x0004003B, 0x0000004C, 
  0x0000004D, 0x00000002, 0x0004002B, 0x0000000F, 
  0x00000055, 0x00000001, 0x0003001D, 0x00000061, 
  0x00000006, 0x0003001E, 0x00000062, 0x00000061, 
  0x00040020, 0x00000063, 0x00000002, 0x00000062, 
  0x0004003B, 0x00000063, 0x00000064, 0x00000002, 
  0x00050036, 0x00000002, 0x00000004, 0x00000000, 
  0x00000003, 0x000200F8, 0x00000005, 0x0004003B, 
  0x00000008, 0x00000009, 0x00000007, 0x0004003B, 
  0x00000008, 0x0000000D, 0x00000007, 0x0004003B, 
  0x00000011, 0x00000012, 0x00000007, 0x0004003B, 
  0x00000021, 0x00000022, 0x00000007, 0x0004003B, 
  0x00000024, 0x00000025, 0x00000007, 0x0004003B, 
  0x00000024, 0x00000030, 0x00000007, 0x0004003B, 
  0x00000024, 0x00000039, 0x00000007, 0x0004003B, 
  0x00000024, 0x00000057, 0x00000007, 0x0003003E, 
  0x00000009, 0x0000000C, 0x0003003E, 0x0000000D, 
  0x0000000E, 0x00050041, 0x00000018, 0x00000019, 
  0x00000016, 0x00000017, 0x0004003D, 0x00000013, 
  0x0000001A, 0x00000019, 0x0004007C, 0x0000000F, 
  0x0000001B, 0x0000001A, 0x00050041, 0x00000018, 
  0x0000001D, 0x00000016, 0x0000001C, 0x0004003D, 
  0x00000013, 0x0000001E, 0x0000001D, 0x0004007C, 
  0x0000000F, 0x0000001F, 0x0000001E, 0x00050050, 
  0x00000010, 0x00000020, 0x0000001B, 0x0000001F, 
  0x0003003E, 0x00000012, 0x00000020, 0x0003003E, 
  0x00000022, 0x00000023, 0x0003003E, 0x00000025, 
  0x00000026, 0x000200F9, 0x00000027, 0x000200F8, 
  0x00000027, 0x0004003D, 0x0000000F, 0x0000002A, 
  0x00000025, 0x0004006F, 0x00000006, 0x0000002B, 
  0x0000002A, 0x00050041, 0x00000021, 0x0000002C, 
  0x00000009, 0x0000001C, 0x0004003D, 0x00000006, 
  0x0000002D, 0x0000002C, 0x000500B8, 0x0000002E, 
  0x0000002F, 0x0000002B, 0x0000002D, 0x000400F6, 
  0x00000028, 0x00000027, 0x00000000, 0x000400FA, 
  0x0000002F, 0x00000029, 0x00000028, 0x000200F8, 
  0x00000029, 0x0004003D, 0x0000000F, 0x00000031, 
  0x00000025, 0x00050041, 0x00000024, 0x00000032, 
  0x00000012, 0x00000017, 0x0004003D, 0x0000000F, 
  0x00000033, 0x00000032, 0x00050041, 0x00000021, 
  0x00000034, 0x00000009, 0x0000001C, 0x0004003D, 
  0x00000006, 0x00000035, 0x00000034, 0x0004006E, 
  0x0000000F, 0x00000036, 0x00000035, 0x00050084, 
  0x0000000F, 0x00000037, 0x00000033, 0x00000036, 
  0x00050080, 0x0000000F, 0x00000038, 0x00000031, 
  0x00000037, 0x0003003E, 0x00000030, 0x00000038, 
  0x00050041, 0x00000024, 0x0000003A, 0x00000012, 
  0x0000001C, 0x0004003D, 0x0000000F, 0x0000003B, 
  0x0000003A, 0x0004003D, 0x0000000F, 0x0000003C, 
  0x00000025, 0x00050041, 0x00000021, 0x0000003D, 
  0x0000000D, 0x0000001C, 0x0004003D, 0x00000006, 
  0x0000003E, 0x0000003D, 0x0004006E, 0x0000000F, 
  0x0000003F, 0x0000003E, 0x00050084, 0x0000000F, 
  0x00000040, 0x0000003C, 0x0000003F, 0x00050080, 
  0x0000000F, 0x00000041, 0x0000003B, 0x00000040, 
  0x0003003E, 0x00000039, 0x00000041, 0x0004003D, 
  0x0000000F, 0x00000046, 0x00000030, 0x00060041, 
  0x00000047, 0x00000048, 0x00000045, 0x00000026, 
  0x00000046, 0x0004003D, 0x00000006, 0x00000049, 
  0x00000048, 0x0004003D, 0x0000000F, 0x0000004E, 
  0x00000039, 0x00060041, 0x00000047, 0x0000004F, 
  0x0000004D, 0x00000026, 0x0000004E, 0x0004003D, 
  0x00000006, 0x00000050, 0x0000004F, 0x00050085, 
  0x00000006, 0x00000051, 0x00000049, 0x00000050, 
  0x0004003D, 0x00000006, 0x00000052, 0x00000022, 
  0x00050081, 0x00000006, 0x00000053, 0x00000052, 
  0x00000051, 0x0003003E, 0x00000022, 0x00000053, 
  0x0004003D, 0x0000000F, 0x00000054, 0x00000025, 
  0x00050080, 0x0000000F, 0x00000056, 0x00000054, 
  0x00000055, 0x0003003E, 0x00000025, 0x00000056, 
  0x000200F9, 0x00000027, 0x000200F8, 0x00000028, 
  0x00050041, 0x00000024, 0x00000058, 0x00000012, 
  0x0000001C, 0x0004003D, 0x0000000F, 0x00000059, 
  0x00000058, 0x00050041, 0x00000024, 0x0000005A, 
  0x00000012, 0x00000017, 0x0004003D, 0x0000000F, 
  0x0000005B, 0x0000005A, 0x00050041, 0x00000021, 
  0x0000005C, 0x0000000D, 0x0000001C, 0x0004003D, 
  0x00000006, 0x0000005D, 0x0000005C, 0x0004006E, 
  0x0000000F, 0x0000005E, 0x0000005D, 0x00050084, 
  0x0000000F, 0x0000005F, 0x0000005B, 0x0000005E, 
  0x00050080, 0x0000000F, 0x00000060, 0x00000059, 
  0x0000005F, 0x0003003E, 0x00000057, 0x00000060, 
  0x0004003D, 0x0000000F, 0x00000065, 0x00000057, 
  0x0004003D, 0x00000006, 0x00000066, 0x00000022, 
  0x00060041, 0x00000047, 0x00000067, 0x00000064, 
  0x00000026, 0x00000065, 0x0003003E, 0x00000067, 
  0x00000066, 0x000100FD, 0x00010038, 
};

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
        default :
            errorTypeName = "WGPUErrorTyp Unknown";
            return;
    }
    std::cout<<"Device error:" << errorTypeName<<std::endl;
}

wgpu::Device createCppDawnDevice() {
    static wgpu::BackendType backendType = wgpu::BackendType::Vulkan;
    static std::unique_ptr<dawn_native::Instance> instance;
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
    dawnProcSetProcs(&backendProcs);
    backendProcs.deviceSetUncapturedErrorCallback(backendDevice, PrintDeviceError, nullptr);
    return wgpu::Device::Acquire(backendDevice);
}

wgpu::Device device = createCppDawnDevice();
wgpu::CreateBufferMappedResult createBufferMappedFromData( wgpu::Device& device,
                                                        const void* data,
                                                        size_t size,
                                                        wgpu::BufferUsage usage){
    wgpu::BufferDescriptor descriptor = {};
    descriptor.size = size;
    descriptor.usage = usage ;
    wgpu::CreateBufferMappedResult result = device.CreateBufferMapped(&descriptor);
    memcpy(result.data, data, size);
    return result;
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

int main(int /*argc*/, char** /*argv*/) 
{
    // create buffer
    std::array<wgpu::Buffer, 2> inputsBuffer;
    wgpu::Buffer resultBuffer;
    float const inputData1[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float const inputData2[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float const outputData[] = {50.0f, 60.0f, 114.0f, 140.0f};
    wgpu::BufferDescriptor desc2;
    desc2.size = sizeof(outputData);
    desc2.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    resultBuffer = device.CreateBuffer(& desc2);
    inputsBuffer[0] = createBufferMappedFromData(device, inputData1, sizeof(inputData1),
        wgpu::BufferUsage::Storage).buffer ;
    inputsBuffer[1] = createBufferMappedFromData(device, inputData2, sizeof(inputData2),
        wgpu::BufferUsage::Storage).buffer ;
    inputsBuffer[0].Unmap();
    inputsBuffer[1].Unmap();
    std::cout << "buffer create succeed" <<std::endl;

    // create bindgroup
    std::vector<wgpu::BindGroupLayoutEntry> bglEntries {
        {0, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer},
        {1, wgpu::ShaderStage::Compute, wgpu::BindingType::ReadonlyStorageBuffer},
        {2, wgpu::ShaderStage::Compute, wgpu::BindingType::StorageBuffer},};
    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = static_cast<uint32_t>(bglEntries.size());
    bglDesc.entries = bglEntries.data();
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    std::vector<wgpu::BindGroupEntry> bgEntries {
        { 0, inputsBuffer[0], 0, static_cast<uint64_t>(sizeof(inputsBuffer[0])), nullptr, nullptr },
        { 1, inputsBuffer[1], 0, static_cast<uint64_t>(sizeof(inputsBuffer[1])), nullptr, nullptr },
        { 2, resultBuffer, 0, static_cast<uint64_t>(sizeof(resultBuffer)), nullptr, nullptr } };
    wgpu::BindGroupDescriptor bgDesc;
    bgDesc.layout = bgl;
    bgDesc.entryCount = bgEntries.size();
    bgDesc.entries = bgEntries.data();
    wgpu::BindGroup bg = device.CreateBindGroup(&bgDesc);
    std::cout<<"create BindGroup succeed"<<std::endl;

    // create shader module
    uint32_t shaderSize =sizeof(shaderBuffer);
    wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
    spirvDesc.sType = wgpu::SType::ShaderModuleSPIRVDescriptor;
    spirvDesc.codeSize = shaderSize;
    spirvDesc.code = shaderBuffer;
    wgpu::ShaderModuleDescriptor descriptor;
    descriptor.nextInChain = &spirvDesc;
    wgpu::ShaderModule module = device.CreateShaderModule(&descriptor);
    std::cout<< "Shader module create succeed"<<std::endl;

    //create pipeline
    wgpu::PipelineLayoutDescriptor pplDesc;
    pplDesc.bindGroupLayoutCount = 1;
    pplDesc.bindGroupLayouts = &bgl;
    wgpu::PipelineLayout pplLayout = device.CreatePipelineLayout(&pplDesc);
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pplLayout;
    csDesc.computeStage.module = module;
    csDesc.computeStage.entryPoint = "main";
    wgpu::ComputePipeline pl = device.CreateComputePipeline(&csDesc);
    std::cout<<"ComputePipeline create succeed"<<std::endl;

    // create command buffer
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pl);
    pass.SetBindGroup(0,bg);
    pass.Dispatch(2,2);
    pass.EndPass();
    wgpu::CommandBuffer cmdBuffer = encoder.Finish();
    std::cout<<"CommandBuffer create succeed"<<std::endl;

    // create fence and submit cmdBuffer
    wgpu::Queue queue = device.GetDefaultQueue();
    wgpu::Fence fence = queue.CreateFence();
    uint64_t singalValue = 1u;
    uint64_t completedValue = 1u;

    queue.Signal(fence, singalValue);
    queue.Submit(1, &cmdBuffer);
    while (fence.GetCompletedValue() < completedValue) 
    {
        device.Tick();
        usleep(1000);
        std::cout<<"Waif for fence"<<std::endl;
    }
    if(fence.GetCompletedValue() != completedValue)
    {
        std::cout<<"Wait for fence failed"<<std::endl;
    }
    // BufferReadAsync tmp;
    // const void* mappedData = tmp.MapReadAsyncAndWait(resultBuffer);
    // void* resultData;
    // memcpy(resultData, mappedData, sizeof(mappedData));
    // float* resultArray = (float*) resultData;

    // release source
    fence.Release();
    queue.Release();
    device.Release();
    std::cout<<"test1 succeed"<<std::endl;
    return 0;
}
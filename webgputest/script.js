import glslangModule from "https://unpkg.com/@webgpu/glslang@0.0.8/dist/web-devel/glslang.js";

(async () => {
  if (!navigator.gpu) {
    console.log(
      "WebGPU is not supported. Enable chrome://flags/#enable-unsafe-webgpu flag."
    );
    return;
  }

  const adapter = await navigator.gpu.requestAdapter();
  const device = await adapter.requestDevice();

  // First Matrix

  const firstMatrix = new Float32Array([
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8
  ]);

  const [gpuBufferFirstMatrix, arrayBufferFirstMatrix] = device.createBufferMapped({
    size: firstMatrix.byteLength,
    usage: GPUBufferUsage.STORAGE
  });
  new Float32Array(arrayBufferFirstMatrix).set(firstMatrix);
  gpuBufferFirstMatrix.unmap();

  // Second Matrix

  const secondMatrix = new Float32Array([
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8
  ]);

  const [gpuBufferSecondMatrix, arrayBufferSecondMatrix] = device.createBufferMapped({
    size: secondMatrix.byteLength,
    usage: GPUBufferUsage.STORAGE
  });
  new Float32Array(arrayBufferSecondMatrix).set(secondMatrix);
  gpuBufferSecondMatrix.unmap();

  // Result Matrix

  const resultMatrixBufferSize =
    Float32Array.BYTES_PER_ELEMENT * 4;
  const resultMatrixBuffer = device.createBuffer({
    size: resultMatrixBufferSize,
    usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_SRC
  });

  // Bind group layout and bind group

  const bindGroupLayout = device.createBindGroupLayout({
    entries: [
      {
        binding: 0,
        visibility: GPUShaderStage.COMPUTE,
        type: "storage-buffer"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.COMPUTE,
        type: "storage-buffer"
      },
      {
        binding: 2,
        visibility: GPUShaderStage.COMPUTE,
        type: "storage-buffer"
      }
    ]
  });

  const bindGroup = device.createBindGroup({
    layout: bindGroupLayout,
    entries: [
      {
        binding: 0,
        resource: {
          buffer: gpuBufferFirstMatrix
        }
      },
      {
        binding: 1,
        resource: {
          buffer: gpuBufferSecondMatrix
        }
      },
      {
        binding: 2,
        resource: {
          buffer: resultMatrixBuffer
        }
      }
    ]
  });

  // Compute shader code (GLSL)

  const computeShaderCode = `#version 450

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
  `;

  // Pipeline setup

  const glslang = await glslangModule();

  const computePipeline = device.createComputePipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [bindGroupLayout]
    }),
    computeStage: {
      module: device.createShaderModule({
        code: glslang.compileGLSL(computeShaderCode, "compute")
      }),
      entryPoint: "main"
    }
  });

  // Commands submission

  const commandEncoder = device.createCommandEncoder();

  const passEncoder = commandEncoder.beginComputePass();
  passEncoder.setPipeline(computePipeline);
  passEncoder.setBindGroup(0, bindGroup);
  passEncoder.dispatch(2/* x */, 2 /* y */);
  passEncoder.endPass();

  // Get a GPU buffer for reading in an unmapped state.
  const gpuReadBuffer = device.createBuffer({
    size: resultMatrixBufferSize,
    usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.MAP_READ
  });

  // Encode commands for copying buffer to buffer.
  commandEncoder.copyBufferToBuffer(
    resultMatrixBuffer /* source buffer */,
    0 /* source offset */,
    gpuReadBuffer /* destination buffer */,
    0 /* destination offset */,
    resultMatrixBufferSize /* size */
  );

  // Submit GPU commands.
  const gpuCommands = commandEncoder.finish();
  device.defaultQueue.submit([gpuCommands]);

  // Read buffer.
  const arrayBuffer = await gpuReadBuffer.mapReadAsync();
  console.log(new Float32Array(arrayBuffer));
})();

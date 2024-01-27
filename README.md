# SimpleVulkanRenderer
 
SimpleVulkanRenderer is, as the name suggests, a simple Vulkan rendering engine designed to abstract away some of Vulkan's boiler plate code.
This engine is also designed around allow for multi-threaded rendering.

## General Structure

Most abstractions are named based on their Vulkan counter parts. For example, the wrapper for `VkBuffer` is `VulkanBuffer`.  
  
## Usage

The SimpleVulkanRenderer has the ability to auto initalize a Vulkan program so that you can focus on developing your application.

```c++
void main()
{
    auto renderer = std::make_shared<VulkanRenderer>();

    VulkanAutoInitSettings autoInitSettings;
    autoInitSettings.InstanceInfo.ApplicationName = "Test Application";
    autoInitSettings.InstanceInfo.ApplicationVersion = VK_MAKE_VERSION(1, 0, 0);
    autoInitSettings.SetupDebug = true;
    autoInitSettings.WindowHeight = HEIGHT;
    autoInitSettings.WindowWidth = WIDTH;
    autoInitSettings.WindowName = "Test Renderer Application";
}
```

First, a VulkanRenderer shared pointer should be constructed to act as the main renderer.  
Then the `VulkanAutoInitSettings` struct is used configure the settings of the Vulkan program. `SetupDebug` will enable Vulkan validation layers and spit out any logs into the console.  
  
SimpleVulkanRenderer will also take care of the construction of Physical and Logical devices. The GPU that supports all of the desired features will be selected to be used.  
The SimpleVulkanRenderer by default will create a graphics and presentation queue for use with the default renderer. Additional queues can be created by using the `CustomQueues`
property of the `VulkanAutoInitSettings`. It is important to note that the SimpleVulkanRenderer will assume that you want distinct queues for each queue descriptor.

```c++
VulkanQueueDescriptor queueDescriptor;
queueDescriptor.Type = COMPUTE_QUEUE;
queueDescriptor.Priority = 0.7f;
queueDescriptor.Name = "ResourceLoadingQueue";
autoInitSettings.CustomQueues.push_back(queueDescriptor);
```

Then, `renderer->AutoInitalize()` is called to make use of the auto initalization feature.
```c++
int main() {
    VulkanAutoInitSettings autoInitSettings;
    autoInitSettings.InstanceInfo.ApplicationName = "Test Application";
    autoInitSettings.InstanceInfo.ApplicationVersion = VK_MAKE_VERSION(1, 0, 0);
    autoInitSettings.SetupDebug = true;
    autoInitSettings.WindowHeight = HEIGHT;
    autoInitSettings.WindowWidth = WIDTH;
    autoInitSettings.WindowName = "Test Renderer Application";

    renderer->AutoInitialize(
        autoInitSettings,
        [](auto descriptorLayout) { 
            /* Create Default Descriptor Layout */
        },
        []() { 
            /* Pipeline Creation Stage */
            GraphicsPipelineDescriptor pipeline;
            pipeline.VertexShader = CreateVertexShader();
            pipeline.FragmentShader = CreateFragmentShader();
            return pipeline;
        },
        []() {
            /* General Loading Stage */
            SetupBuffers();
        },
        [](auto setBuilder) {
            /* Create the default descriptor sets for each framebuffer. */
        }
    );
}
```

`AutoInitalize` takes in 4 callback functions that are used to configure the program during the initalization process.
1. The first callback creates the descriptor layouts that are used by the vertex and fragment shaders. Things like uniform buffer and image sampler bindings are defined here.
```c++
[](auto descriptorLayout) {
    /* Create Default Descriptor Layout */
    // Describes the uniforms that are used in the shaders.
    descriptorLayout->UniformBufferBinding(/*Binding*/ 0, /*Count*/ 1, VK_SHADER_STAGE_VERTEX_BIT /* Shader Stage */);
    descriptorLayout->ImageSamplerBinding(1, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
}
```

2. The second callback allows you to customize the default graphics pipeline. The callback expects you to return the `GraphicsPipelienDescriptor` struct. The vertex and fragment shaders are required to be defined here.
```c++
[]() { 
/* Pipeline Creation Stage */
    GraphicsPipelineDescriptor pipeline;
    pipeline.VertexShader = CreateVertexShader();
    pipeline.FragmentShader = CreateFragmentShader();
    return pipeline;
}
```

3. The third callback is a general loading stage for lading data and setting up buffers. Generally buffers that are used for descriptor sets are defined and created here.

4. The fourth callback describes the default descriptor sets for the each framebuffer (by default the SimpleVulkanRenderer uses 2 framebuffers).
```c++
[](auto setBuilder) { 
    /* Create the default descriptor sets for each framebuffer. */
    VulkanTexture texture("textures/texture.jpg", renderer, renderer->mBufferUtilities);
    setBuilder->DescribeBuffer(0, 0, mappedUniformBuffers, sizeof(UniformBufferObject));
    setBuilder->DescribeImageSample(1, 0, texture.ImageView(), texture.Sampler());
}
```

### Buffer Utilities
SimpleVulkanRenderer provides buffer utilities to make managing VkBuffers easier.

```c++
auto bufferUtils = renderer->mBufferUtilities;
```

You can create a buffer with `CreateBuffer` and map them to memory with `MapMemory`. A `VulkanMappedBuffer` can store both a VkBuffer and its memory.

```c++
VulkanMappedBuffer modelMatrixBuffer;
bufferUtils->CreateBuffer(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, modelMatrixBuffer, modelMatrixBuffer);

// Map the buffer's memory:
bufferUtils->MapMemory(modelMatrixBuffer, 0, sizeof(glm::mat4) * 2, 0, modelMatrixBuffer.DirectMappedMemory());

// Copy data to the buffer.
memcpy(modelMatrixBuffer[currentImage].MappedMemory(), &modelMatrix, sizeof(glm::mat4));
```

### Rendering

To start drawing an image, `StartFrameDrawing()` needs to be called.

```c++
auto currentImage = renderer->StartFrameDrawing();
```

`StartFrameDrawing()` returns the index of the swapchain image that is to be drawn to.

After that commands can be recorded into the frame command buffer.

```c++
auto frameCommandBuffer = renderer->GetFrameCommandBuffer();
frameCommandBuffer->Reset();
frameCommandBuffer->StartCommandRecording();
```

`Reset()` resets the command buffer so the commands from the previous frame are erased.  
`StartCommandRecording()` sets the command buffer to start recording.
  
  
Then, the render pass to use needs to be defined. SimpleVulkanRender will create a default render pass with the auto initalizaiton process.

```c++
auto frameBuffer = renderer->SwapChain()->FrameBuffers()[currentImage];
auto extent = renderer->SwapChain()->Extent();
VkClearColorValue clearColor = {164 / 255.0, 236 / 255.0, 252 / 255.0, 1.0}; // Sky Blue
frameCommandBuffer->StartRenderPass(renderer->RenderPass(), frameBuffer, extent, clearColor);
```

The graphics pipeline can be binded using `BindPipeline`.
```c++
frameCommandBuffer->BindPipeline(renderer->PrimaryGraphicsPipeline()->Pipeline());
```

The final setup for rendering, is to setup the viewport scissoring.
```c++
frameCommandBuffer->SetViewportScissor(renderer->SwapChain()->Extent());
```

An object can be drawn by binding the vertex and index buffers.
```c++
frameCommandBuffer->BindVertexBuffer(object->VertexBuffer());
frameCommandBuffer->BindIndexBuffer(object->IndexBuffer());
frameCommandBuffer->BindVertexBuffer(object->ModelBuffer(), 0 /* offset */, 1 /* First Binding */); // Bind the model buffer.
frameCommandBuffer->BindDescriptorSet(renderer->PrimaryGraphicsPipeline()->PipelineLayout(), renderer->DescriptorHandler()->DescriptorSetBuilder()->GetBuiltDescriptorSets()[currentImage]);
frameCommandBuffer->DrawIndexed(object->IndiciesSize());
```

At the end of drawing the frame, the command recording needs to be closed:
```c++
frameCommandBuffer->EndRenderPass();
frameCommandBuffer->EndCommandRecording();
```

Then, we mark the end of frame drawing:
```c++
renderer->EndFrameDrawing(currentImage);
```

## Included Demo
The SimpleVulkanRenderer comes with demo that shows off Vulkan's multithreading capabilities with resource loading.  
  
The demo generates 16x16x16 voxel chunks in a thread pool. The entry point for the demo is the `main.cpp` file.  
The number of resource threads can be changed by modifing the `NUM_RESOURCE_THREADS` constant at the top of the file.
**NOTE:** The demo assumes that each thread can have it's own compute queue. Make sure `NUM_RESOURCE_THREADS` does not exceed
the number of compute queues that your GPU has available.  
  
The size of a voxel chunk (by default 16 x 16 x 16) can be changed in the `DemoConsts.hpp` header file with the constant `CHUNK_VOXEL_COUNT`.

### Resource Loading System
The diagram below shows the flow of how resources are loaded onto the GPU concurrently.  
![Flow diagram showing the resource loading](https://img.ryandw11.com/raw/rvshpr4a6.png)  
The demo uses a simplified version of what is shown above.  

### Images
![](https://img.ryandw11.com/raw/rvqte5d1a.png)  
![](https://img.ryandw11.com/raw/rvqtk1i1m.png)  

### Greedy Mesh Algorithm

To test resource loading, a primitive version of the Greedy Mesh Algorithm is used to optimize the mesh of each voxel chunk:  
![](https://img.ryandw11.com/raw/rvqt19lfv.png)

## Used Resources
- [Vulkan](https://www.vulkan.org/)
- [GLM](https://github.com/g-truc/glm)
- [GLFW](https://www.glfw.org/)
- [stb_image](https://github.com/nothings/stb)
- [siv::PerlinNoise](https://github.com/Reputeless/PerlinNoise) (Demo Only)
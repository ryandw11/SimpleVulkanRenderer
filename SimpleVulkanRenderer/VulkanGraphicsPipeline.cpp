#include "VulkanGraphicsPipeline.hpp"

#include <stdexcept>
#include <vector>

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineDescriptor descriptor)
    :
    mDescriptor( descriptor ),
    mVertexShader( descriptor.VertexShader ),
    mFragmentShader( descriptor.FragmentShader ),
    mOtherShaders( descriptor.OtherShaders )
{
    if (mVertexShader == nullptr)
    {
        throw std::runtime_error("Vertex shader must be defined!");
    }
    if (mFragmentShader == nullptr)
    {
        throw std::runtime_error("Fragment shader must be defined!");
    }
}

// TODO:: A bug needs to be fix where shaders will be killd when the window resizes.
void VulkanGraphicsPipeline::UpdatePipeline(VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
{
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { mVertexShader->GetShaderStage(), mFragmentShader->GetShaderStage() };

    for (auto shader : mOtherShaders)
    {
        shaderStages.push_back(shader->GetShaderStage());
    }

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = mDescriptor.InputTopology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Define the pipeline's viewport. Some GPUs allows multiple viewports and sciessor rectangles.
    // That requires a GPU feature to be enabled in the logical device creation.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1; // TODO :: Verify this

    // Setup the rasterizer, which takes the vertices from the vertex shader and
    // turns them into fragments to be colored by the fragment shader. Depth Testing and Face culling are done here.
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // Clamp fragments beyond the near and far planes.
    rasterizer.depthClampEnable = VK_TRUE;
    // If set to true, the geometry will never pass through the rasterizer stage. This disables any output to the frame buffer.
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // This determines how fragments are generated. There is also LINE (wireframe) and POINT. (Other modes than FILL require GPU Features)
    rasterizer.polygonMode = mDescriptor.PolygonMode;
    // Defines the thickness of the lines. (Anything thicker than 1.0 requers the wideLine GPU feature).
    rasterizer.lineWidth = mDescriptor.LineWidth;
    // The type of face culling to use.
    rasterizer.cullMode = mDescriptor.CullMode;
    // The vertex order for faces to be considered front-facing.
    rasterizer.frontFace = mDescriptor.VertexOrder;
    // Depth values can be alstered by adding a constant value or using the fragement's slope.
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling is a method of anti-aliasing. Enabling this requires a GPU Feature. I'll disable this for now.
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and Stencil testing can go here next.

    // Blend the old and new colors on the frame buffer.
    // Configuration per attached frame buffer.
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Creating the pipeline layout which handles shader uniforms.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    // Reference the descriptorSetLayout which contains info for the shader descriptors.
    // This is created using the createDescriptorSetLayout() method.
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    auto vertexInputStateInfo = mVertexShader->GetVertexInputStateInfo();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    // Fixed Function States
    pipelineInfo.pVertexInputState = &vertexInputStateInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    // Pipeline Layout
    pipelineInfo.layout = mPipelineLayout;
    // Render pass
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    // Pipeline Derivatives
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a graphics pipeline!");
    }

    // Destory shader modules as they are no longer needed.
    mVertexShader->DestroyShaderModuleIfNeeded(device);
    mFragmentShader->DestroyShaderModuleIfNeeded(device);
    for (auto shader : mOtherShaders)
    {
        shader->DestroyShaderModuleIfNeeded(device);
    }
}

void VulkanGraphicsPipeline::CleanupPipeline(VkDevice device)
{
    vkDestroyPipeline(device, mPipeline, nullptr);
    vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
}

VkPipeline VulkanGraphicsPipeline::Pipeline() const
{
    return mPipeline;
}

VkPipelineLayout VulkanGraphicsPipeline::PipelineLayout() const
{
    return mPipelineLayout;
}

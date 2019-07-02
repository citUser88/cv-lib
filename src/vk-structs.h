#pragma once

#include "vulkan/vulkan.hpp"

typedef struct
{
	vk::Image image;						
	vk::Framebuffer framebuffer;
	std::array<vk::ImageView, 2> views;
} vk_SwapchainImageResources;

typedef struct
{
	vk::Sampler					sampler;
	vk::Image					image;
	vk::Buffer					buffer;
	vk::ImageLayout			imageLayout;
	vk::MemoryAllocateInfo  mem_alloc;
	vk::DeviceMemory			mem;
	vk::ImageView				view;
	uint32_t						tex_width;
	uint32_t						tex_height;
} vk_texture_struct;

// Main VK objects structure
typedef struct VkS 
{
	vk::Instance			I;
	vk::PhysicalDevice	G;	 
	vk::Device				D;  
	vk::SurfaceKHR			S;  
	vk::SwapchainKHR		SC; 
	vk::RenderPass			RP;
	vk::Rect2D				rdrArea;
	vk::PhysicalDeviceProperties			Gprops;
	vk::PhysicalDeviceMemoryProperties	Gmemprops;
	vk::PhysicalDeviceFeatures				Gfeatures;
	
	std::vector<vk::DeviceQueueCreateInfo> DQCIs;
	std::vector<vk::QueueFamilyProperties>	QFPs;

	std::vector<uint32_t> QFIs;
	vk::Queue gQ;

	float prioriti = 0.5;
	uint32_t qFI = 0; // queue family index

	std::vector<vk_SwapchainImageResources> swapchainBuffers;

	vk::Format			sfcColorFormat;
	vk::Format			sfcDepthFormat;
	vk::ColorSpaceKHR sfcColorSpace;
	vk::Extent2D		sfcSize;

	std::vector<vk::Viewport> VPs;
	std::vector<vk::Rect2D>	  Scisrs;

	vk::SurfaceCapabilitiesKHR			sfcCapabilities;
	std::vector<vk::PresentModeKHR>	sfcPresentModes;

	std::vector<vk::Fence> waitFences;
	std::vector<vk::CommandBuffer> commandBuffers;
	vk::Semaphore renderCompleteSemaphore;
	vk::Semaphore presentCompleteSemaphore;

	uint32_t currentBuffer = 0;
	uint32_t imageIndex = 0;

	vk::CommandPool commandPool;

	//Rendering Loop		
	uint64_t frameCounter = 0;
	double frameTimer = 0.0;
	double fpsTimer = 0.0;
	double lastFPS = 0.0;	
	float roty = 0;
	std::chrono::time_point<std::chrono::steady_clock> tStart;


	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	} uboVS;

	// Vertex Buffer and attributes
	struct {
		vk::DeviceMemory memory; //handle->device memory
		vk::Buffer buffer;		 //handle->binding buffer
		vk::PipelineVertexInputStateCreateInfo inputState;
		vk::VertexInputBindingDescription inputBinding;
		std::vector<vk::VertexInputAttributeDescription> inputAttributes;
	} vertices;//struct vertices

	// Index Buffer
	struct Indices {
		vk::DeviceMemory memory = nullptr;
		vk::Buffer buffer = nullptr;
		uint32_t count = 0;
	} indices;//struct indices

	// Uniform block object
	struct {
		vk::DeviceMemory memory = nullptr;
		vk::Buffer buffer = nullptr;
		vk::DescriptorBufferInfo descriptor;
	} uniformDataVS;//struct uniformDataVS

	vk::DescriptorPool descriptorPool;
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout pipelineLayout;
	std::vector<vk::DescriptorSet> descriptorSets;
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

	std::vector<char> vertShaderCode;
	std::vector<char> fragShaderCode;
} VkS;
VkS vks;
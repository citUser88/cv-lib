// TODO: Split into separate .CPP / .H files, rebuild around CPU worker queue threading model
// Remove all redundancies

#pragma once

#define DEFAULT_FENCE_TIMEOUT 100000000000
#define USE_SWAPCHAIN_EXTENSIONS

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.hpp"

#include <functional>
#include <iostream>
#include <vector>
#include "FWK.h"
#include "SPIRV-interfacing.h"
#include "windows-interfacing.h"

#include "vk-structs.h"
#include "vk-init.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm-0.9.9.3/glm/glm/gtc/type_ptr.hpp>
#include <glm-0.9.9.3/glm/glm/glm.hpp>
#include <glm-0.9.9.3/glm/glm/gtc/matrix_transform.hpp>

std::string SHADER_PATH = { "D:\\shaders\\" };

float zoom = -5;

class O2VK
{
public:
	O2VK()      
	{	
      logString_to_devLog("ctor invoked: O2VK", true);
		try{ initvisual(); }
		catch (const vk::SystemError &e)
			{ logString_to_devLog("O2VK:  failure in CTOR during VULKAN initialization procedures.. -> BYEBYE", true);	exit(-1); }
		catch (...)
			{ OutputDebugString("ABLAASDSBDSA ... O2VK failed in its CTOR. -> BYEBYE");	exit(-88); }
		}//try catch	
	~O2VK(){ 
		logString_to_devLog("dtor invoked: O2VK", true);
	}//O2VK dtor
	void RenderFrame() {
		// WILL FIX, timing ordering is wrong, obviously
		vks.tStart = std::chrono::high_resolution_clock::now();
		vks.frameCounter++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - vks.tStart).count();
		vks.roty = vks.roty+0.01;
		if (vks.roty > 360) vks.roty = 0;
		vks.frameTimer = tDiff / 1000.0;
		vks.fpsTimer += tDiff;
		if (vks.fpsTimer > 1000.0/60) {			
			vks.lastFPS = roundf(1.0f / vks.frameTimer);
			vks.fpsTimer = 0.0;
			vks.frameCounter = 0;
			return;
		}//if vks.fpsTimer > 1000.0/60

		// Update Uniforms and matrices
		auto rotation = glm::vec3();		
		vks.uboVS.projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 256.0f);
		vks.uboVS.viewMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, zoom));
		vks.uboVS.modelMatrix = glm::mat4(1.0f);
		vks.uboVS.modelMatrix = glm::rotate(vks.uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		vks.uboVS.modelMatrix = glm::rotate(vks.uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		vks.uboVS.modelMatrix = glm::rotate(vks.uboVS.modelMatrix, glm::radians(vks.roty), glm::vec3(0.0f, 0.0f, 1.0f));
	
		// Map uniform buffer and update it
		void *pData;
		pData = vks.D.mapMemory(vks.uniformDataVS.memory, 0, sizeof(vks.uboVS));
		memcpy(pData, &vks.uboVS, sizeof(vks.uboVS));
		vks.D.unmapMemory(vks.uniformDataVS.memory);
		// end UpdateUniforms

		// Get the next present image
		vk::Result res;
		res = vks.D.acquireNextImageKHR(vks.SC, UINT64_MAX, 
			vks.presentCompleteSemaphore, nullptr, &vks.currentBuffer);
		vks.D.waitForFences(1, &vks.waitFences[vks.currentBuffer], VK_TRUE, UINT64_MAX);
		vks.D.resetFences(1, &vks.waitFences[vks.currentBuffer]);

		// Create color output kernel flag
		vk::PipelineStageFlags kernelPipelineStageFlags = 
			vk::PipelineStageFlagBits::eColorAttachmentOutput;

		auto kernel = vk::SubmitInfo(
			1,
			&vks.presentCompleteSemaphore,
			&kernelPipelineStageFlags,
			1,
			&vks.commandBuffers[vks.currentBuffer],
			1,
			&vks.renderCompleteSemaphore);//kernel = ...
		
		vks.gQ.submit(1, &kernel, vks.waitFences[vks.currentBuffer]);
		vks.gQ.presentKHR(
			vk::PresentInfoKHR(
				1,
				&vks.renderCompleteSemaphore,
				1,
				&vks.SC,
				&vks.currentBuffer,
				nullptr
			)//PIKHR()
		);//gQ.presentKHR()
	}//RenderFrame()
private:
	VkInit vkinit;

	uint32_t getMemoryTypeIndex(uint32_t typeBits, vk::MemoryPropertyFlags properties)
	{
		for (uint32_t i = 0; i < vks.Gmemprops.memoryTypeCount; ++i) {
			if ((typeBits & 1) == 1) {
				if ((vks.Gmemprops.memoryTypes[i].propertyFlags & properties) == properties)
				{					
					return i;
				}//if properties has flags contained in vks.Gmemprops.memorytypes[i].propertyFlags 
			}//if
			typeBits >>= 1;
		}//for
	throw "getMemoryTypeIndex:  Memory type bit not found";//static_cast<uint32_t>(1);
	};//auto lambda
//-------------------------------------------------------------------------------------
	void logvkError_exit(const vk::SystemError err) {
		logString_to_devLog("O2VK: vkCreateInstanceUnique encountered an error initializing", true);
		std::string errString = err.what();
		std::string logerrString("O2VK: this error: " + errString + "...\n");
		logString_to_devLog(logerrString, false);
		exit(-1);
	}
	void initvisual()
	{
		try { test_initvkapi(); }
		catch (const vk::SystemError& err){ logvkError_exit(err);}//VkSystemError
		catch (...){ logString_to_devLog("O2VK: caught an exception, and it was not processed.", true);	exit(-33);}
	}//initvk()
	void set_gfxQueue() { 
		vks.gQ = vks.D.getQueue(vks.qFI, 0); //vkGetDeviceQueue with DQCI(s) and eGraphics flag set 
	} //set_gfxQueue()
	void Surface() { 
		// Create Win32 Surface
		vk::Win32SurfaceCreateInfoKHR surfaceinfo = vk::Win32SurfaceCreateInfoKHR(vk::Win32SurfaceCreateFlagsKHR(), hInst, hWnd);//W32SfcCIKHR
		auto vksurfaceinfo = surfaceinfo.operator const VkWin32SurfaceCreateInfoKHR&();//VkW32SfcCIKHR&()
								//^ why this way AG?
		auto vksurface = VkSurfaceKHR();
		auto createwin32surface = vkCreateWin32SurfaceKHR(vks.I, &vksurfaceinfo, NULL, &vksurface);
		assert(createwin32surface == VK_SUCCESS);
		// Get surface Information
		vks.S = vk::SurfaceKHR(vksurface);
	}//Surface()
	void Color_Depth_Formats() {
		// Color Formats
		// locate best color / depth information formats
		auto surfaceFormats = vks.G.getSurfaceFormatsKHR(vks.S);

		if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
			vks.sfcColorFormat = vk::Format::eB8G8R8A8Unorm;//FALLBACK
		else
			vks.sfcColorFormat = surfaceFormats[0].format;
		vks.sfcColorSpace = surfaceFormats[0].colorSpace;

		// Depth Formats - Best first
		std::vector<vk::Format> depthFormats = {
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD16Unorm				
		};									  
		
		for (auto& format : depthFormats)	{
				auto depthFormatProperties = vks.G.getFormatProperties(format);
				// verify support for depth stencil attachment with optimal tiling
				if (depthFormatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)	{
					vks.sfcDepthFormat = format;
					break;
				}//dFProps.oTFs & vk::FormatFeatureFlagBits::eDepthStencilAttachment
		}// for loop find depth format props support depth stencil attachment and optimal tiling
	}//Color_Depth_Formats()		
	void RenderPass() {
		// Render Pass
		// Describe attachments to the framebuffer, color and depth buffers
		std::vector<vk::AttachmentDescription> attachmentDescriptions =
		{
			vk::AttachmentDescription(
				vk::AttachmentDescriptionFlags(),
				vks.sfcColorFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			),
			vk::AttachmentDescription(
				vk::AttachmentDescriptionFlags(),
				vks.sfcDepthFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			)
		};
		// Create an AttachmentReference for the color AttachmentDescription
		std::vector<vk::AttachmentReference> colorReferences = {
			vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal)
		};
		// Create an AttachmentReference for the depth AttachmentDescription
		std::vector<vk::AttachmentReference> depthReferences = {
			vk::AttachmentReference(1,	vk::ImageLayout::eDepthStencilAttachmentOptimal)
		};
		// Create SubpassDescription and pass in the color and depth AttachmentReferences
		std::vector<vk::SubpassDescription> subpasses = {
			vk::SubpassDescription(
				vk::SubpassDescriptionFlags(),
				vk::PipelineBindPoint::eGraphics,
				0,
				nullptr,
				static_cast<uint32_t>(colorReferences.size()),
				colorReferences.data(),
				nullptr,
				depthReferences.data(),
				0,
				nullptr
				)
		};//subpass list
		// Create SubpassDependency information for render Subpass
		std::vector<vk::SubpassDependency> dependencies =
		{
			vk::SubpassDependency(
				VK_SUBPASS_EXTERNAL, 0,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eMemoryRead,
				vk::AccessFlagBits::eColorAttachmentRead |
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::DependencyFlagBits::eByRegion
			),
			vk::SubpassDependency(
				0, VK_SUBPASS_EXTERNAL,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::AccessFlagBits::eColorAttachmentRead |
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::AccessFlagBits::eMemoryRead,
				vk::DependencyFlagBits::eByRegion
			)
		};//dependencies list

		//Render pass
		vks.RP = vks.D.createRenderPass(
			vk::RenderPassCreateInfo(
				vk::RenderPassCreateFlags(),
				static_cast<uint32_t>(attachmentDescriptions.size()),
				attachmentDescriptions.data(),
				1,//static_cast<uint32_t>(subpasses.size()),
				subpasses.data(),
				static_cast<uint32_t>(dependencies.size()),
				dependencies.data()
			)//RPCI
		);//vks.D.createRenderPass()
	}//RenderPass
	void Synchronizers() {		

		// Present and Render Semaphores
		vks.presentCompleteSemaphore = vks.D.createSemaphore(vk::SemaphoreCreateInfo());
		vks.renderCompleteSemaphore = vks.D.createSemaphore(vk::SemaphoreCreateInfo());

		// Individual swapchain buffer Fences
		vks.waitFences.resize(vks.swapchainBuffers.size());
		for (int i = 0; i < vks.waitFences.size(); i++) {
			vks.waitFences[i] = vks.D.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
		}//for i: 0-vks.waitFences.size(): wFncs[i] = dvc.createFence(vk::FenceCreateFlagBits::eSignaled));	
	}//Synchronizers()
	void CommandPool() {
		vks.commandPool = vks.D.createCommandPool(
			vk::CommandPoolCreateInfo(
				vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
				vks.qFI
			)
		);
		
		// Individual command buffers per swapchain buffer
		vks.commandBuffers = vks.D.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(vks.commandPool, vk::CommandBufferLevel::ePrimary,vks.swapchainBuffers.size()
			)//CBAI
		);//vks.d.allocateCommandBuffers()
	}//CommandPool()
	void Swapchain() {
		vks.sfcCapabilities = vks.G.getSurfaceCapabilitiesKHR(vks.S);
		assert(vks.sfcCapabilities.maxImageCount >= 3);
		vks.sfcPresentModes = vks.G.getSurfacePresentModesKHR(vks.S);
		vks.sfcSize = vk::Extent2D(width, height);
	
		// Esp. in Win32 Surface width and height must exactly match OS window region
		if (!(vks.sfcCapabilities.currentExtent.width == -1 || vks.sfcCapabilities.currentExtent.height == -1)) {
			vks.sfcSize = vks.sfcCapabilities.currentExtent;
		}
		// check/adjust 2D surface extents
		vks.rdrArea = vk::Rect2D(vk::Offset2D(), vks.sfcSize);

		vk::Viewport viewport = 
			vk::Viewport(0.0f, 0.0f, 
							static_cast<float>(vks.sfcSize.width), 
							static_cast<float>(vks.sfcSize.height), 0, 1.0f);
		vks.VPs = { viewport };//viewports
		vks.Scisrs = { vks.rdrArea };//scissors

		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;

		for (auto &pm : vks.sfcPresentModes) {
			if (pm == vk::PresentModeKHR::eMailbox) {
				presentMode = vk::PresentModeKHR::eMailbox;
				break;
			}// if
		}// vk::PresentModeKHR::eMailbox if any surface present modes are set to eMailbox (best non-tearing present mode)

		vks.QFIs.push_back(0);

		VkBool32 retBool;	
		vks.G.getSurfaceSupportKHR(0, vks.S, &retBool);

		vks.D.waitIdle();

		vks.SC = vks.D.createSwapchainKHR(
			vk::SwapchainCreateInfoKHR(
				vk::SwapchainCreateFlagsKHR(),
				vks.S,
				vks.sfcCapabilities.maxImageCount,
				vks.sfcColorFormat,
				vks.sfcColorSpace,
				vks.sfcSize,
				1,
				vk::ImageUsageFlagBits::eColorAttachment,
				vk::SharingMode::eExclusive,
				vks.QFIs.size(),
				vks.QFIs.data(),
				vk::SurfaceTransformFlagBitsKHR::eIdentity,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,
				presentMode,
				VK_TRUE,
				vk::SwapchainKHR()//vk::SwapchainKHR()
			)//SwpchCIKHR()
		);//device.createSwapchainKHR()
	}//Swapchain()
	void FrameBuffers() {
		auto swapchainImages = vks.D.getSwapchainImagesKHR(vks.SC);

		vk::Image depthImage = vks.D.createImage(
			vk::ImageCreateInfo(
				vk::ImageCreateFlags(),
				vk::ImageType::e2D,
				vks.sfcDepthFormat,
				vk::Extent3D(vks.sfcSize.width, vks.sfcSize.height, 1), 1, 1,
				vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive,
				vks.QFIs.size(),
				vks.QFIs.data(),
				vk::ImageLayout::eUndefined
			)//ICI
		);//device.createImage()

		// Search through GPU memory properties to see if this can be device local.
		auto depthMemoryReq = vks.D.getImageMemoryRequirements(depthImage);

		auto depthMemory = vks.D.allocateMemory(
			vk::MemoryAllocateInfo(
				depthMemoryReq.size,
				getMemoryTypeIndex(depthMemoryReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal
				)
			)//gtMemTypeIdx
		);//vks.D..allocateMemory()

		vks.D.bindImageMemory(
			depthImage,
			depthMemory,
			0
		);

		auto depthImageView = vks.D.createImageView(
			vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),
				depthImage,
				vk::ImageViewType::e2D,
				vks.sfcDepthFormat,
				vk::ComponentMapping(),
				vk::ImageSubresourceRange(
					vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
					0,
					1,
					0,
					1
				)
			)//IVCI
		);//vks.D.createImageView()

		vks.swapchainBuffers.resize(swapchainImages.size());
		for (int i = 0; i < swapchainImages.size(); i++)
		{
			// color
			vks.swapchainBuffers[i].image = swapchainImages[i];
			vks.swapchainBuffers[i].views[0] =
				vks.D.createImageView(
					vk::ImageViewCreateInfo(
						vk::ImageViewCreateFlags(),
						swapchainImages[i],
						vk::ImageViewType::e2D,
						vks.sfcColorFormat,
						vk::ComponentMapping(),
						vk::ImageSubresourceRange(
							vk::ImageAspectFlagBits::eColor,
							0,
							1,
							0,
							1
						)//ImgSubrcRng()					
					)//vk::IVCI
				);//vks.D.createImageView()

				// depth 
			vks.swapchainBuffers[i].views[1] = depthImageView;
			vks.swapchainBuffers[i].framebuffer = vks.D.createFramebuffer(
				vk::FramebufferCreateInfo(
					vk::FramebufferCreateFlags(),
					vks.RP,
					vks.swapchainBuffers[i].views.size(),
					vks.swapchainBuffers[i].views.data(),
					vks.sfcSize.width,
					vks.sfcSize.height,
					1
				)//FBCI
			);//vks.D.createFramebuffer()
		}//for loop to set the swapchainBuffers.image = swapchainImages[i]
		//end Framebuffers
	}//FrameBuffers()
	void initDescriptorPools() {
		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
			{ vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1) };
		vks.descriptorPool = vks.D.createDescriptorPool(
			vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlags(),
				1,
				static_cast<uint32_t>(descriptorPoolSizes.size()),
				descriptorPoolSizes.data()
			)
		);
	}//initDescriptorPools();
	void initDescriptorSets() {
		// Binding 0: Uniform buffer (Vertex shader)
 		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding(
				0,
				vk::DescriptorType::eUniformBuffer,
				1,
				vk::ShaderStageFlagBits::eVertex,
				nullptr
			)//DSLB()
		};//descriptorSetLayoutBindings

		vks.descriptorSetLayouts = {
			vks.D.createDescriptorSetLayout(
				vk::DescriptorSetLayoutCreateInfo(
					vk::DescriptorSetLayoutCreateFlags(),
					descriptorSetLayoutBindings.size(),
					descriptorSetLayoutBindings.data()
				)//DSLCI
			)//createDescriptorSetLayout()
		};//descriptorSetLayouts = {}

		vks.descriptorSets = 
			vks.D.allocateDescriptorSets(
				vk::DescriptorSetAllocateInfo(
					vks.descriptorPool,
					vks.descriptorSetLayouts.size(),
					vks.descriptorSetLayouts.data()
				)//DSAI
		);// allocateDescriptorSets()
	}//initDescriptorSets()
	void initResources() {
	
		initDescriptorPools();
		initDescriptorSets();

		struct Vertex
		{
			float position[3]; // 4 bytes each for 12 bytes total
			float color[3];	 // 4 bytes each for 12 bytes total
		};//struct Vertex	

		// Vertices and RGB colors of a simple test triangle
		std::vector<Vertex> vertexBuffer = {
			{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {-1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f} },
			{ { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f} }
		};//vertexBuffer
		uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);// (4*3*2*3)=72 bytes
	
		// Setup indices data for test triangle
		std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
		vks.indices.count = static_cast<uint32_t>(indexBuffer.size());
		uint32_t indexBufferSize = vks.indices.count * sizeof(uint32_t);

		void *data;

		struct StagingBuffer {
			vk::DeviceMemory	memory;
			vk::Buffer			buffer;
		};		
		struct {
			StagingBuffer vertices;
			StagingBuffer indices;
		} stagingBuffers;

		// Vertex Buffer
		stagingBuffers.vertices.buffer = vks.D.createBuffer(
			vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				vertexBufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive,
				vks.QFIs.size(),//vks.QFIs.size(),
				vks.QFIs.data()
			)//BuffCI()
		);//vks.D.createBuffer()

		vk::MemoryRequirements memReqs = vks.D.getBufferMemoryRequirements(stagingBuffers.vertices.buffer);

		// Mapping and copying data to GPU
		// Coherent, all writes are completed to the GPU immediately following unmapping the buffer
		uint32_t memtypidx = getMemoryTypeIndex(memReqs.memoryTypeBits, 	vk::MemoryPropertyFlagBits::eHostVisible |
																								vk::MemoryPropertyFlagBits::eHostCoherent);
		stagingBuffers.vertices.memory = vks.D.allocateMemory(
			vk::MemoryAllocateInfo(
				memReqs.size,
				memtypidx
			)//MAInfo
		);//vks.D.allocateMemory()

		// Map and copy
		data = vks.D.mapMemory(stagingBuffers.vertices.memory, 0, memReqs.size, vk::MemoryMapFlags());
		memcpy(data, vertexBuffer.data(), vertexBufferSize);
		vks.D.unmapMemory(stagingBuffers.vertices.memory);
		vks.D.bindBufferMemory(stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0);

		// Create GPU memory buffer to copy vertice data into 
		vks.vertices.buffer = vks.D.createBuffer(
			vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				vertexBufferSize,
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::SharingMode::eExclusive,
				vks.QFIs.size(),
				vks.QFIs.data()
			)//BCI
		);//vks.D.createBuffer()

		memReqs = vks.D.getBufferMemoryRequirements(vks.vertices.buffer);

		vks.vertices.memory = vks.D.allocateMemory(
			vk::MemoryAllocateInfo(
				memReqs.size,
				getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
			)//MemAllocInfo()
		);//vks.D.allocateMemory

		vks.D.bindBufferMemory(vks.vertices.buffer, vks.vertices.memory, 0);

		// Index Buffer for copying from device to GPU
		stagingBuffers.indices.buffer = vks.D.createBuffer(
			vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				indexBufferSize,
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::SharingMode::eExclusive,
				vks.QFIs.size(),
				vks.QFIs.data()
			)//BuffCInfo()
		);//vks.D.createBuffer

		memReqs = vks.D.getBufferMemoryRequirements(stagingBuffers.indices.buffer);
		stagingBuffers.indices.memory = vks.D.allocateMemory(
			vk::MemoryAllocateInfo(
				memReqs.size,
				getMemoryTypeIndex(memReqs.memoryTypeBits, 
				vk::MemoryPropertyFlagBits::eHostVisible | 
				vk::MemoryPropertyFlagBits::eHostCoherent)
			)//MemAllocInfo()
		);//vks.D.allocateMemory

		data = vks.D.mapMemory(stagingBuffers.indices.memory, 0, indexBufferSize, vk::MemoryMapFlags());
		memcpy(data, indexBuffer.data(), indexBufferSize);
		vks.D.unmapMemory(stagingBuffers.indices.memory);
		vks.D.bindBufferMemory(stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0);

		// Create GPU memory buffer to copy indices data into 
		vks.indices.buffer = vks.D.createBuffer(
			vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				indexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::SharingMode::eExclusive,
				vks.QFIs.size(),
				vks.QFIs.data()
			)//BuffCInfo()
		);//vks.D.createBuffer

		memReqs = vks.D.getBufferMemoryRequirements(vks.indices.buffer);
		vks.indices.memory = vks.D.allocateMemory(
			vk::MemoryAllocateInfo(
				memReqs.size,
				getMemoryTypeIndex(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal
				)//getMemoryTypeIndex()
			)//MemAllocInfo()
		);//vks.D.allocateMemory()

		vks.D.bindBufferMemory(vks.indices.buffer, vks.indices.memory, 0);

		auto getCommandBuffer = [&](bool begin)
		{
			vk::CommandBuffer cmdBuffer = vks.D.allocateCommandBuffers(
				vk::CommandBufferAllocateInfo(
					vks.commandPool,
					vk::CommandBufferLevel::ePrimary,
					1
				)//CBuffAllocInfo()
			)[0];//vks.D.allocateCommandBuffers
		
			if (begin) {
				cmdBuffer.begin(
					vk::CommandBufferBeginInfo()
				);//cmdBuffer.begin()
			}//if begin

			return cmdBuffer;
		};//getCommandBuffer


		// Create command buffer for copying the vertex and index information from the mapped memory to the GPU memory
		vk::CommandBuffer copyCmd = getCommandBuffer(true);

		// Put buffer region copies into command buffer
		std::vector<vk::BufferCopy> copyRegions =	{
			vk::BufferCopy(0, 0, vertexBufferSize)
		};//copyRegions

		// Vertex buffer
		copyCmd.copyBuffer(stagingBuffers.vertices.buffer, vks.vertices.buffer, copyRegions);

		// Index buffer
		copyRegions = {
			vk::BufferCopy(0, 0, indexBufferSize)
		};//copyRegions =
		// end Vertex Buffers

		copyCmd.copyBuffer(stagingBuffers.indices.buffer, vks.indices.buffer, copyRegions);

		// Finish with flushing command buffer
		auto flushCommandBuffer = [&](vk::CommandBuffer commandBuffer)
		{
			commandBuffer.end();

			std::vector<vk::SubmitInfo> submitInfos = {
				vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer, 0, nullptr)
			};//submitInfos

			// Create fence to ensure that the command buffer has finished executing
			vk::Fence fence = vks.D.createFence(vk::FenceCreateInfo());

			// Submit to queue
			vks.gQ.submit(submitInfos, fence);
			// Wait for the fence to signal that the command buffer has finished executing
			vks.D.waitForFences(1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
			vks.D.destroyFence(fence);
			vks.D.freeCommandBuffers(vks.commandPool, 1, &commandBuffer);
		};//flushCommandBuffer =

		flushCommandBuffer(copyCmd);

		// Destroy the staging buffers following completed flushing
		vks.D.destroyBuffer(stagingBuffers.vertices.buffer);
		vks.D.freeMemory(stagingBuffers.vertices.memory);
		vks.D.destroyBuffer(stagingBuffers.indices.buffer);
		vks.D.freeMemory(stagingBuffers.indices.memory);

		// Vertex input binding
		vks.vertices.inputBinding.binding = 0;
		vks.vertices.inputBinding.stride = sizeof(Vertex);
		vks.vertices.inputBinding.inputRate = vk::VertexInputRate::eVertex;

		// Input attribute bindings metadata for shader attribute locations and memory layouts
		// layout (location = 0) in vec3 inPos;
		//	layout (location = 1) in vec3 inColor;
		vks.vertices.inputAttributes.resize(2);
		// Attributes location 0: Position
		vks.vertices.inputAttributes[0].binding = 0;
		vks.vertices.inputAttributes[0].location = 0;
		vks.vertices.inputAttributes[0].format = vk::Format::eR32G32B32Sfloat; //no alpha 32-bit floats RGB
		vks.vertices.inputAttributes[0].offset = offsetof(Vertex, position);
		// Attributes location 1: Color 
		vks.vertices.inputAttributes[1].binding = 0;
		vks.vertices.inputAttributes[1].location = 1;
		vks.vertices.inputAttributes[1].format = vk::Format::eR32G32B32Sfloat;
		vks.vertices.inputAttributes[1].offset = offsetof(Vertex, color);

		// Assign to the vertex input state used for pipeline creation
		vks.vertices.inputState.flags = vk::PipelineVertexInputStateCreateFlags();
		vks.vertices.inputState.vertexBindingDescriptionCount = 1;
		vks.vertices.inputState.pVertexBindingDescriptions = &vks.vertices.inputBinding;
		vks.vertices.inputState.vertexAttributeDescriptionCount = 
			static_cast<uint32_t>(vks.vertices.inputAttributes.size());
		vks.vertices.inputState.pVertexAttributeDescriptions = vks.vertices.inputAttributes.data();

		//end Vertex buffers

 		// UniformBuffers

		// Vertex shader uniform buffer block
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		// Create a new buffer
		vks.uniformDataVS.buffer = vks.D.createBuffer(
			vk::BufferCreateInfo(
				vk::BufferCreateFlags(),
				sizeof(vks.uboVS),
				vk::BufferUsageFlagBits::eUniformBuffer
			)//BufferCInfo
		);//vks.D.createBuffer
		// Get memory requirements including size, alignment and memory type
		memReqs = vks.D.getBufferMemoryRequirements(vks.uniformDataVS.buffer);
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | 
			vk::MemoryPropertyFlagBits::eHostCoherent);
		// Allocate memory for the uniform buffer
		vks.uniformDataVS.memory = vks.D.allocateMemory(allocInfo);
		// Bind Memory to buffer
		vks.D.bindBufferMemory(vks.uniformDataVS.buffer, vks.uniformDataVS.memory, 0);

		// Vertex Shader uniform descriptor information
		vks.uniformDataVS.descriptor.buffer = vks.uniformDataVS.buffer;
		vks.uniformDataVS.descriptor.offset = 0;
		vks.uniformDataVS.descriptor.range = sizeof(vks.uboVS);
		// end UniformBuffers

		// Update Uniforms

		// MVP matrices for vertex shader uniform buffers
		vks.uboVS.projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 256.0f);
		vks.uboVS.viewMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, zoom));
		auto rotation = glm::vec3();
		vks.uboVS.modelMatrix = glm::mat4(1.0f);
		vks.uboVS.modelMatrix = glm::rotate(vks.uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		vks.uboVS.modelMatrix = glm::rotate(vks.uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		vks.uboVS.modelMatrix = glm::rotate(vks.uboVS.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		// Map uniform buffer and copy to GPU (initialize)
		void *pData;
		pData = vks.D.mapMemory(vks.uniformDataVS.memory, 0, sizeof(vks.uboVS));
		memcpy(pData, &vks.uboVS, sizeof(vks.uboVS));
		vks.D.unmapMemory(vks.uniformDataVS.memory);
		// end UpdateUniforms

		// DescriptorSetUpdate
		std::vector<vk::WriteDescriptorSet> descriptorWrites = {
			vk::WriteDescriptorSet(
				vks.descriptorSets[0],
				0,
				0,
				1,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				&vks.uniformDataVS.descriptor,
				nullptr
			)
		};//descriptorWrites

		vks.D.updateDescriptorSets(descriptorWrites, nullptr);
		// end DescriptorSetUpdate

		// Load Shaders
		static auto readfile = [&](const std::string& f) {
			std::string const filename = SHADER_PATH + f;
			std::ifstream file(filename, std::ios::ate | std::ios::binary);
			bool exists = (bool)file;
			if (!exists || !file.is_open()) {
				throw std::runtime_error("Failed to open shader file!");
			}//if exists and file.is_open()

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		};//readfile = [&](){}

		vks.vertShaderCode = readfile("triangle.vert.spv");
		vks.fragShaderCode = readfile("triangle.frag.spv");
		// end Load Shaders
	}//initResources()
	void initPipeline() {
		// Pipeline layouts

		vks.pipelineLayout = vks.D.createPipelineLayout(
			vk::PipelineLayoutCreateInfo(
				vk::PipelineLayoutCreateFlags(),
				vks.descriptorSetLayouts.size(),
				vks.descriptorSetLayouts.data(),
				0,
				nullptr
			)//PlLCI
		);//vks.D.createPipelineLayout

		vk::ShaderModule vertModule = vks.D.createShaderModule(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),
				vks.vertShaderCode.size(),
				(uint32_t*)vks.vertShaderCode.data()
			)//vertModule = vks.D.createShaderModule()
		);//createShaderModule()

		vk::ShaderModule fragModule = vks.D.createShaderModule(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),
				vks.fragShaderCode.size(),
				(uint32_t*)vks.fragShaderCode.data()
			)//fragModule = vks.D.createShaderModule()
		);//createShaderModule()

		auto pipelineCache = vks.D.createPipelineCache(vk::PipelineCacheCreateInfo());

		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStages = {
			vk::PipelineShaderStageCreateInfo(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eVertex,
				vertModule,
				"main",
				nullptr
			),
			vk::PipelineShaderStageCreateInfo(
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eFragment,
				fragModule,
				"main",
				nullptr
			)
		};//pipelineShaderStages

		auto pvi = vks.vertices.inputState;

		auto pia = vk::PipelineInputAssemblyStateCreateInfo(
			vk::PipelineInputAssemblyStateCreateFlags(),
			vk::PrimitiveTopology::eTriangleList
		);//PipelineInputAssemblyStateCreateInfo

		auto pv = vk::PipelineViewportStateCreateInfo(
			vk::PipelineViewportStateCreateFlagBits(),
			vks.VPs.size(),
			vks.VPs.data(),
			vks.Scisrs.size(),
			vks.Scisrs.data()
		);//PipelineViewportStateCreateInfo

		auto pr = vk::PipelineRasterizationStateCreateInfo(
			vk::PipelineRasterizationStateCreateFlags(),
			VK_FALSE,
			VK_FALSE,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eNone,
			vk::FrontFace::eCounterClockwise,
			VK_FALSE,
			0,
			0,
			0,
			1.0f
		);//PipelineRasterizationStateCreateInfo()

		auto pm = vk::PipelineMultisampleStateCreateInfo(
			vk::PipelineMultisampleStateCreateFlags(),
			vk::SampleCountFlagBits::e1
		);//PipelineMultisampleStateCreateInfo()

		// Depth and Stencil states
		auto pds = vk::PipelineDepthStencilStateCreateInfo(
			vk::PipelineDepthStencilStateCreateFlags(),
			VK_TRUE,
			VK_TRUE,
			vk::CompareOp::eLessOrEqual,
			VK_FALSE,
			VK_FALSE,
			vk::StencilOpState(),
			vk::StencilOpState(),
			0,
			0
		);//PipelineDepthStencilStateCreateInfo()

		// Blend State - How two overlapping primitives behave
		std::vector<vk::PipelineColorBlendAttachmentState>	colorBlendAttachments = {
			vk::PipelineColorBlendAttachmentState(
			 VK_FALSE,
			 vk::BlendFactor::eZero,
			 vk::BlendFactor::eOne,
			 vk::BlendOp::eAdd,
			 vk::BlendFactor::eZero,
			 vk::BlendFactor::eZero,
			 vk::BlendOp::eAdd,
			 vk::ColorComponentFlags(vk::ColorComponentFlagBits::eR | 
											 vk::ColorComponentFlagBits::eG | 
											 vk::ColorComponentFlagBits::eB | 
											 vk::ColorComponentFlagBits::eA)
			)//PipelineColorBlendAttachmentState()
		};//colorBlendAttachments

		auto pbs = vk::PipelineColorBlendStateCreateInfo(
			vk::PipelineColorBlendStateCreateFlags(),
			0,
			vk::LogicOp::eClear,
			colorBlendAttachments.size(),
			colorBlendAttachments.data()
		);//PipelineColorBlendStateCreateInfo()

		std::vector<vk::DynamicState>	dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};//dynamicStates

		auto pdy = vk::PipelineDynamicStateCreateInfo(
			vk::PipelineDynamicStateCreateFlags(),
			dynamicStates.size(),
			dynamicStates.data()
		);//PipelineDynamicStateCreateInfo()

		vks.graphicsPipeline = vks.D.createGraphicsPipeline(nullptr,//pipelineCache,
			vk::GraphicsPipelineCreateInfo(
				vk::PipelineCreateFlags(),
				pipelineShaderStages.size(),
				pipelineShaderStages.data(),
				&pvi,
				&pia,
				nullptr,
				&pv,
				&pr,
				&pm,
				&pds,
				&pbs,
				&pdy,
				vks.pipelineLayout,
				vks.RP,
				0
			)//GPlCI
		);//vks.D.createGraphicsPipeline
	}//initPipeline()
	void Commands() {
		std::vector<vk::ClearValue> clearValues = {
			vk::ClearColorValue(
				std::array<float, 4>{0.1f,0.6f,0.4f,1.0f}
			),
			vk::ClearDepthStencilValue(1.0f, 0)//CCV
		};//clearValues

		for (int32_t i = 0; i < vks.commandBuffers.size(); ++i) {
			vks.commandBuffers[i].begin(vk::CommandBufferBeginInfo());
			vks.commandBuffers[i].beginRenderPass(
				vk::RenderPassBeginInfo(
					vks.RP,
					vks.swapchainBuffers[i].framebuffer,
					vks.rdrArea,
					clearValues.size(),
					clearValues.data()
				),//RPBI
				vk::SubpassContents::eInline
			);//cBs[i].beginRenderPass()

			vks.commandBuffers[i].setViewport(0, vks.VPs); // from 0, vks.vp -> 0, 1, vks.VPs
			vks.commandBuffers[i].setScissor(0, vks.rdrArea); // from 1, vks.rdrArea -> 0, 1, vks.RdrArea

			// Bind graphicsPipeline and descriptor sets
			vks.commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, vks.graphicsPipeline);
			vks.commandBuffers[i].bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				vks.pipelineLayout,
				0,
				vks.descriptorSets,
				nullptr
			);//cBs[i].bindDescriptorSets()
			// Bind vertex / index buffers, draw indexed primitives, etc...
			std::array<vk::DeviceSize, 1>	offsets = { 0 };
			vks.commandBuffers[i].bindVertexBuffers(0, 1, &vks.vertices.buffer, offsets.data());
			vks.commandBuffers[i].bindIndexBuffer(vks.indices.buffer, 0, vk::IndexType::eUint32);
			vks.commandBuffers[i].drawIndexed(vks.indices.count, 1, 0, 0, 1);
			vks.commandBuffers[i].endRenderPass();
			vks.commandBuffers[i].end();
		}//for			
	}//Commands()
	void test_initvkapi()
	{			
		vkinit.init();

		set_gfxQueue();
		Surface();
		Color_Depth_Formats();
		RenderPass();
		Swapchain();
		FrameBuffers();
		Synchronizers();
		CommandPool();
		initResources();
		initPipeline();
		Commands();
	}//void test_initvkapi
};//O2VK
#pragma once

#include <string>
#include "vulkan/vulkan.hpp"
#include "vk-interfacing.hpp"
#include "vk-structs.h"

// add and work out namespace VKS in long run

class VkInit 
{
public:
	VkInit() {};
	void init()
	{			
		Extensions();
		Layers();		  
		create_VkInstance();
		PhysicalDevices();
		LogicalDevices();
		DeviceLayers();
		gfxQueue();
		create_VkDevice();
	}
	~VkInit()
	{															  
	}
private:
	std::vector<const char*> default_exts = { VK_KHR_SURFACE_EXTENSION_NAME,
															VK_KHR_WIN32_SURFACE_EXTENSION_NAME,VK_EXT_DEBUG_REPORT_EXTENSION_NAME 
															};
	std::vector<const char*> default_layers = { "VK_LAYER_LUNARG_standard_validation" };
	std::vector<const char*> default_dvcexts = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
																VK_EXT_DEBUG_MARKER_EXTENSION_NAME 
																};
	std::vector<const char*> default_device_layers = { "VK_LAYER_LUNARG_standard_validation", 
																		"VK_LAYER_RENDERDOC_Capture" 
																		};

	std::vector<const char*> devvalidlayers;
	std::vector<const char*> layers;
	std::vector<const char*> exts;
	std::vector<const char*> dvcexts;

	static std::vector<char const*> getDeviceExtensions()
	{
		std::vector<char const*> device_extensions;
		device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		return device_extensions;
	}
	void Extensions() {
		// sieve enumerated instance extensions
		//auto enumerated_exts = F();
		auto enumerated_exts = vk::enumerateInstanceExtensionProperties();
		for (auto &z : default_exts) {
			for (auto &i : enumerated_exts) {
				if (std::string(i.extensionName).compare(z) == 0) {
					exts.emplace_back(z);
					break;
				}// i.extensionName == z.default_exts[z]
			}// for i...
		}// for z...
	}//Extensions()
	void Layers() {
		// sieve enumerated instance layer properties
		//auto enumerated_layers = F();
		auto enumerated_layers = vk::enumerateInstanceLayerProperties();
		for (auto &z : default_layers) {
			for (auto &i : enumerated_layers) {
				if (std::string(i.layerName).compare(z) == 0) {
					layers.emplace_back(z);
					break;
				}// i.layerName == z.default_layers[z]
			}// for i...
		}// for z...
	}//Layers()
	void PhysicalDevices() {
		// enumerate and select 0th Physical device as GPU
		auto enumerated_physical_devices	= vks.I.enumeratePhysicalDevices();
		vks.G = enumerated_physical_devices[0];
		vks.Gprops		= vks.G.getProperties();
		vks.Gmemprops	= vks.G.getMemoryProperties();
	}//PhysicalDevices()
	void LogicalDevices() {
		// sieve physical device extension properties
		auto enumerated_device_extension_properties = vks.G.enumerateDeviceExtensionProperties();
		for (auto &z : default_dvcexts) {
			for (auto &i : enumerated_device_extension_properties) {
				if (std::string(i.extensionName).compare(z) == 0) {
					dvcexts.emplace_back(z);
					break;
				}// i.extensionName == z.desiredExtensions[z]
			}// for i...
		}// for z...
	}//LogicalDevices()
	void gfxQueue() 
	{
		// sieve queues for graphics queue
		vks.Gfeatures					= vks.G.getFeatures();
		vks.QFPs							= vks.G.getQueueFamilyProperties();

		vks.qFI = 0;//reset queueFamilyIndex variable

		for (auto& queueFamily : vks.QFPs) {
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {				
				vks.DQCIs.push_back( // create single graphics queue
					vk::DeviceQueueCreateInfo(	vk::DeviceQueueCreateFlags(),
														vks.qFI,// index of QF to create on logical device
														1,// # of Qs to create in the QF set by vks.qFI(queueFamilyIndex)
														&vks.prioriti
													)//DQCIs										
												);//push_back()
			break;
			}// qF.qF & eGraphics true
		vks.qFI++;//++ queue family index
		}//for	}//gfxQueue()
	}//gfxQueue()
	void DeviceLayers() {
		// locate GPU layers
		auto enumerated_device_layers = vks.G.enumerateDeviceLayerProperties();

		for (auto &z : default_device_layers) {
			for (auto &i : enumerated_device_layers) {
				if (std::string(i.layerName).compare(z) == 0) {
					devvalidlayers.emplace_back(z);
					break;
				}// i.layerName == z.desiredDeviceValidationLayer[z]
			}// for i...
		}// for z...
	}//DeviceLayers()
	void create_VkInstance() 
	{
		auto app_info = vk::ApplicationInfo(
				"ogiAPP",VK_MAKE_VERSION(1,0,0),"ogiFWK",VK_MAKE_VERSION(1,0,0),VK_API_VERSION_1_0);//appinfo initialization
		vks.I = vk::createInstance(
					vk::InstanceCreateInfo(
					vk::InstanceCreateFlags(),
						&app_info, layers.size(),layers.data(),
										exts.size(), exts.data()));//instance initialization
	} //create_VkInstance()
	void create_VkDevice() 
	{
		// create device
		vks.D = vks.G.createDevice(
			vk::DeviceCreateInfo(
				vk::DeviceCreateFlags(),
				vks.DQCIs.size(),
				vks.DQCIs.data(),
				devvalidlayers.size(),
				devvalidlayers.data(),
				dvcexts.size(),
				dvcexts.data(),
				&vks.Gfeatures
			)//dci
		);//gpu.createDevice()
	}//create_VkDevice()
};//class vkInit
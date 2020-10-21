#pragma once
#include "Platform.h"
#include <vulkan.h>
#include <cassert>
#include <conio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <sstream>
#include "Window.h"
#include "Build_Options.h"

class Renderer
{
private:
	VkInstance _instance  = nullptr;
	VkPhysicalDevice _gpu = nullptr;
	VkDevice _device = nullptr;
	VkQueue _queue = nullptr;
	int _queue_family_index = -1;
	VkPhysicalDeviceProperties _physical_device_properties{};
	VkPhysicalDeviceMemoryProperties _physical_device_memory_properties{};

	std::vector<const char *> _instance_layers;
	std::vector<const char *> _device_layers;
	std::vector<const char *> _instance_extensions;
	std::vector<const char *> _device_extensions;

	VkDebugReportCallbackEXT _debug_report_callbeck_ext;
	VkDebugReportCallbackCreateInfoEXT x_debug_report_create_info{};


	class Window *_window;
	void SetupLayersAndExtensions();
	void InitInstance();
	void DestroyInstance();
	void InitDevice();
	void DestroyDevice();
	void SetupDebug();
	void InitDebug();
	void DestroyDebug();
	void InitWindow();
	void DestroyWindow();
	void InitSurface();
	void DestroySurface();
	

public:
	Renderer();
	~Renderer();
	const VkInstance GetVulkanInstance() const;
	const VkPhysicalDevice GetVulkanPhysicalDevice() const;
	const VkDevice GetVulkanDevice() const;
	const VkQueue GetVulkanQueue() const;
	const uint32_t GetVulkanGraphicsQueueFamilyIndex() const;
	const VkResult ErrorReporting(VkResult perror) const;
	const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
};


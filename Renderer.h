#pragma once
#include <vulkan.h>
#include <cassert>
#include <conio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

class Renderer
{
private:
	VkInstance _instance  = nullptr;
	VkPhysicalDevice _gpu = nullptr;
	VkDevice _device = nullptr;
	int _queue_family_index = -1;
	std::vector<const char *> _instance_layers;
	std::vector<const char *> _device_layers;
	std::vector<const char *> _instance_extensions;
	std::vector<const char *> _device_extensions;
	VkDebugReportCallbackEXT _debug_report_callbeck_ext;

	void InitInstance();
	void DestroyInstance();
	void InitDevice();
	void DestroyDevice();
	void SetupDebug();
	void InitDebug();
	void DestroyDebug();
	void ErrorReporting(VkResult perror);
public:
	Renderer();
	~Renderer();
};


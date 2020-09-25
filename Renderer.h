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

	void InitInstance();
	void DestroyInstance();
	void InitDevice();
	void DestroyDevice();
	void ErrorReporting(VkResult perror);
public:
	Renderer();
	~Renderer();
};


#pragma once
#include"Platform.h"
#include<iostream>
#include "Renderer.h"

class Window
{
private:
	HWND _window_handle;
	HINSTANCE _app_instance;
	int _width=0;
	int _height=0;
	uint32_t _swapchain_image_count = 3;
	class Renderer* _r = NULL;
    VkSurfaceKHR _surface=VK_NULL_HANDLE;
	VkSurfaceCapabilitiesKHR _surface_capabilities = {};
	VkSurfaceFormatKHR _surface_format;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> _swapchain_images = {};
	std::vector<VkImageView> _swapchain_image_views = {};
	VkImage	_depth_stencil_image = VK_NULL_HANDLE;
	VkImageView _depth_stencil_image_view = VK_NULL_HANDLE;
	VkFormat _depth_stencil_format = VK_FORMAT_UNDEFINED;
	VkDeviceMemory _depth_stencil_image_memory = VK_NULL_HANDLE;
	bool _stencil_available = false;

	void InitOsSurface();
	void DestroyOsSurface();
	void InitSwapChain();
	void DestroySwapchain();
	void InitSwapChainImages();
	void DestroySwapchainImages();
	void InitDepthStencilImage();
	void DestroyDepthStencileImage();
public:
	Window(class Renderer *pr,const char * pname,int pwidth, int pheight);
	~Window();
};


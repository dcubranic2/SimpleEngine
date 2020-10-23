#include "Window.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Window::Window(class Renderer *prenderer,const char* pname,int pwidth,int pheight)
{
	_app_instance = GetModuleHandle(NULL);
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Vulkan Window Class";

    wchar_t xtext_wchar[30];
    size_t xlength = 0;
    mbstowcs_s(&xlength, xtext_wchar, pname, strlen(pname));

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = _app_instance;
    wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

    // Create the window.

    _window_handle = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        xtext_wchar,// Window text
        (WS_OVERLAPPED |  // Window style
            WS_CAPTION |  
            WS_MINIMIZEBOX | WS_SYSMENU),
        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, pwidth, pheight,

        NULL,           // Parent window    
        NULL,           // Menu
        _app_instance,  // Instance handle
        NULL            // Additional application data
    );

    if (_window_handle == NULL)
    {
        std::exit(-1);
    }

    ShowWindow(_window_handle, SW_SHOWNORMAL);

    RECT xrect;
    GetClientRect(_window_handle, &xrect);
    _width = xrect.right-xrect.left;
    _height = xrect.bottom-xrect.top;

    _r = prenderer; // set pointer to renderer

    InitOsSurface();
    InitSwapChain();
    InitSwapChainImages();
    InitDepthStencilImage();

    // Process Windows Messages
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    //UnregisterClass(CLASS_NAME, _app_instance);
}

Window::~Window()
{
    DestroyDepthStencileImage();
    DestroySwapchainImages();
    DestroySwapchain();
    DestroyOsSurface();
    DestroyWindow(_window_handle);
}
void Window::InitOsSurface()
{
    VkWin32SurfaceCreateInfoKHR xwin32surface_create_info {};
    xwin32surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    xwin32surface_create_info.hwnd = _window_handle;
    xwin32surface_create_info.hinstance = _app_instance;

    _r->ErrorReporting(vkCreateWin32SurfaceKHR(_r->GetVulkanInstance(), &xwin32surface_create_info, nullptr, &_surface));

    //after we get VkSurfaceKHR from Window we must see if graphics card suports this surface and CHOOSE appropiate surface format
    //for later use in Swapchain

    VkBool32 xsupported = false;
    _r->ErrorReporting(vkGetPhysicalDeviceSurfaceSupportKHR(_r->GetVulkanPhysicalDevice(), _r->GetVulkanGraphicsQueueFamilyIndex(), _surface, &xsupported));
    if (!xsupported)
    {
        assert(0 && "Vulkan Error: Physical device does not support presentation on this surface!");
        std::exit(-1);
    }

    //additional check if surface size iz same as window size and if not set it to be equal
    VkResult xresult=_r->ErrorReporting(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_r->GetVulkanPhysicalDevice(), _surface, &_surface_capabilities));
    if(xresult >= VK_SUCCESS)
    {
        if (_surface_capabilities.currentExtent.width != _width)
            _width = _surface_capabilities.currentExtent.width;
        if (_surface_capabilities.currentExtent.height != _height)
            _height = _surface_capabilities.currentExtent.height;
    }
    {
        uint32_t xsurface_formats_no = 0;
        _r->ErrorReporting(vkGetPhysicalDeviceSurfaceFormatsKHR(_r->GetVulkanPhysicalDevice(), _surface, &xsurface_formats_no, nullptr));
        if (xsurface_formats_no == 0)
        {
            assert(0 && "Vulkan Error: Surface format missing");
            std::exit(-1);
        }
        std::vector<VkSurfaceFormatKHR> x_surface_formats(xsurface_formats_no);
        _r->ErrorReporting(vkGetPhysicalDeviceSurfaceFormatsKHR(_r->GetVulkanPhysicalDevice(), _surface, &xsurface_formats_no, x_surface_formats.data()));
        if (x_surface_formats[0].format== VK_FORMAT_UNDEFINED)
        {
            _surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            _surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else 
        {
            _surface_format = x_surface_formats[0];
        }
    }
}
void Window::InitSwapChain()
{
    if (_swapchain_image_count > _surface_capabilities.maxImageCount)
        _swapchain_image_count = _surface_capabilities.maxImageCount;
    if (_swapchain_image_count < _surface_capabilities.minImageCount)
        _swapchain_image_count = _surface_capabilities.minImageCount;
    
    VkPresentModeKHR x_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    {
        uint32_t xpresenter_mode_count = 0;
        _r->ErrorReporting(vkGetPhysicalDeviceSurfacePresentModesKHR(_r->GetVulkanPhysicalDevice(), _surface, &xpresenter_mode_count, nullptr));
        std::vector<VkPresentModeKHR> x_supportet_presenter_modes(xpresenter_mode_count);
        _r->ErrorReporting(vkGetPhysicalDeviceSurfacePresentModesKHR(_r->GetVulkanPhysicalDevice(), _surface, &xpresenter_mode_count, x_supportet_presenter_modes.data()));
        for (size_t i = 0; i < x_supportet_presenter_modes.size(); i++)
        {
            if (x_supportet_presenter_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                x_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }

    }

    VkSwapchainCreateInfoKHR x_swapchain_create_info{};
    x_swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    x_swapchain_create_info.surface = _surface;
    x_swapchain_create_info.minImageCount = _swapchain_image_count;
    x_swapchain_create_info.imageFormat = _surface_format.format;
    x_swapchain_create_info.imageColorSpace = _surface_format.colorSpace;
    x_swapchain_create_info.imageExtent.width = _width;
    x_swapchain_create_info.imageExtent.height = _height;
    x_swapchain_create_info.imageArrayLayers = 1; // number of stereo images
    x_swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    x_swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    x_swapchain_create_info.queueFamilyIndexCount = 0;
    x_swapchain_create_info.pQueueFamilyIndices = nullptr;
    x_swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    x_swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    x_swapchain_create_info.presentMode = x_present_mode;
    x_swapchain_create_info.clipped = VK_TRUE;
    x_swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    _r->ErrorReporting(vkCreateSwapchainKHR(_r->GetVulkanDevice(), &x_swapchain_create_info, nullptr, &_swapchain));
}
void Window::DestroySwapchain()
{
    vkDestroySwapchainKHR(_r->GetVulkanDevice(),_swapchain, nullptr);
}
void Window::InitSwapChainImages()
{
    {
        _r->ErrorReporting(vkGetSwapchainImagesKHR(_r->GetVulkanDevice(), _swapchain, &_swapchain_image_count, nullptr));
        _swapchain_images.resize(_swapchain_image_count);
        _swapchain_image_views.resize(_swapchain_image_count);
        _r->ErrorReporting(vkGetSwapchainImagesKHR(_r->GetVulkanDevice(), _swapchain, &_swapchain_image_count, _swapchain_images.data()));
    }

    // for each Image you must create ImageView
    for (uint32_t i = 0;i < _swapchain_image_count;i++) 
    {
        VkImageViewCreateInfo _ximage_view_create_info{};
        _ximage_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        _ximage_view_create_info.pNext = nullptr;
        _ximage_view_create_info.flags= VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
        _ximage_view_create_info.image=_swapchain_images[i];
        _ximage_view_create_info.viewType= VK_IMAGE_VIEW_TYPE_2D;
        _ximage_view_create_info.format = _surface_format.format;
        _ximage_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        _ximage_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        _ximage_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        _ximage_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        _ximage_view_create_info.subresourceRange.aspectMask= VK_IMAGE_ASPECT_COLOR_BIT;
        _ximage_view_create_info.subresourceRange.baseArrayLayer = 0;
        _ximage_view_create_info.subresourceRange.layerCount = 1;
        _ximage_view_create_info.subresourceRange.baseMipLevel = 0;
        _ximage_view_create_info.subresourceRange.levelCount = 1;

        _r->ErrorReporting(vkCreateImageView(_r->GetVulkanDevice(), &_ximage_view_create_info, nullptr, &_swapchain_image_views[i]));
    }
}
void Window::DestroySwapchainImages()
{
    //destroy all VkImageViews
    //VkImages are destroyed when you destroy swapchain
    for (uint32_t i = 0;i < _swapchain_image_count;i++)
    {
        vkDestroyImageView(_r->GetVulkanDevice(), _swapchain_image_views[i], nullptr);
    }
}
void Window::InitDepthStencilImage()
{
    {
        std::vector<VkFormat> x_posible_formats
        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM
        };

        for (auto f : x_posible_formats)
        {
            //find what depth stencil format is supported by GPU
            VkFormatProperties x_format_properties;
            vkGetPhysicalDeviceFormatProperties(_r->GetVulkanPhysicalDevice(), f, &x_format_properties);
            if (x_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                _depth_stencil_format = f;
                break;
            }
        }
        //if we did not find any supported format exit
        if (_depth_stencil_format == VK_FORMAT_UNDEFINED)
        {
            assert(0 && "Vulkan Error: Depth stencil format not selected.");
            std::exit(-1);
        }
        if ((_depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
            (_depth_stencil_format == VK_FORMAT_D24_UNORM_S8_UINT)  ||
            (_depth_stencil_format == VK_FORMAT_D16_UNORM_S8_UINT)  ||
            (_depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT))
        {
            _stencil_available = true;
        }

    }

    uint32_t x_queue_family_indexes = 0;
    VkImageCreateInfo _x_image_create_info = {};
    _x_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    _x_image_create_info.pNext = nullptr;
    _x_image_create_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    _x_image_create_info.imageType = VK_IMAGE_TYPE_2D;
    _x_image_create_info.format=_depth_stencil_format;
    _x_image_create_info.extent.width =_width;
    _x_image_create_info.extent.height = _height;
    _x_image_create_info.extent.depth = 1;
    _x_image_create_info.mipLevels = 1;
    _x_image_create_info.arrayLayers = 1;
    _x_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT; // for multisampling
    _x_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    _x_image_create_info.usage= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    _x_image_create_info.sharingMode= VK_SHARING_MODE_EXCLUSIVE;
    _x_image_create_info.queueFamilyIndexCount = 0;
    _x_image_create_info.pQueueFamilyIndices=&x_queue_family_indexes;
    _x_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // we do not care for initial layaout

    _r->ErrorReporting(vkCreateImage(_r-> GetVulkanDevice(), &_x_image_create_info, nullptr, &_depth_stencil_image));

    //for image find memory requirements and memory index
    VkMemoryRequirements x_memory_requrements = {};
    vkGetImageMemoryRequirements(_r->GetVulkanDevice(), _depth_stencil_image, &x_memory_requrements);

    uint32_t x_memory_index= _r->FindMemoryTypeIndex(&_r->GetPhysicalDeviceMemoryProperties(), &x_memory_requrements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo x_memory_allocation_info;
    x_memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    x_memory_allocation_info.pNext = nullptr;
    x_memory_allocation_info.allocationSize = x_memory_requrements.size; // from image
    x_memory_allocation_info.memoryTypeIndex = x_memory_index;           // from GPU
    _r->ErrorReporting(vkAllocateMemory(_r->GetVulkanDevice(), &x_memory_allocation_info, nullptr, &_depth_stencil_image_memory));

    _r->ErrorReporting(vkBindImageMemory(_r->GetVulkanDevice(), _depth_stencil_image, _depth_stencil_image_memory, 0));

    VkImageViewCreateInfo x_image_view_create_info = {};
    x_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    x_image_view_create_info.pNext = nullptr;
    x_image_view_create_info.flags = VK_IMAGE_VIEW_CREATE_FLAG_BITS_MAX_ENUM;
    x_image_view_create_info.image = _depth_stencil_image;
    x_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    x_image_view_create_info.format = _depth_stencil_format;
    x_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    x_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    x_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    x_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    x_image_view_create_info.subresourceRange.aspectMask= VK_IMAGE_ASPECT_DEPTH_BIT | (_stencil_available? VK_IMAGE_ASPECT_STENCIL_BIT : 0x0);
    x_image_view_create_info.subresourceRange.baseArrayLayer = 0;
    x_image_view_create_info.subresourceRange.baseMipLevel = 0;
    x_image_view_create_info.subresourceRange.layerCount = 1;
    x_image_view_create_info.subresourceRange.levelCount = 1;

    _r->ErrorReporting(vkCreateImageView(_r->GetVulkanDevice(),&x_image_view_create_info, nullptr, &_depth_stencil_image_view));
}
void Window::DestroyDepthStencileImage()
{
    vkDestroyImageView(_r->GetVulkanDevice(), _depth_stencil_image_view, nullptr);
    vkFreeMemory(_r->GetVulkanDevice(), _depth_stencil_image_memory, nullptr);
    vkDestroyImage(_r->GetVulkanDevice(), _depth_stencil_image, nullptr);
}
void Window::DestroyOsSurface()
{
    vkDestroySurfaceKHR(_r->GetVulkanInstance(), _surface, nullptr);
}
void OnSize(HWND hwnd, UINT flag, int width, int height)
{
    // Handle resizing
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return FALSE; // message is handled
            break;
        }
        case WM_SIZE:
        {
            int width = LOWORD(lParam);  // Macro to get the low-order word.
            int height = HIWORD(lParam); // Macro to get the high-order word.

            // Respond to the message:
            OnSize(hwnd, (UINT)wParam, width, height);
            return FALSE;
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // All painting occurs here, between BeginPaint and EndPaint.
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
            return FALSE;
            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return TRUE; // message is unhandled
}




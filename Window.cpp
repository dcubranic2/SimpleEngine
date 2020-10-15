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

    {
        _r->ErrorReporting(vkGetSwapchainImagesKHR(_r->GetVulkanDevice(), _swapchain, &_swapchain_image_count, nullptr));
        std::vector<VkImage> x_swapchain_images(_swapchain_image_count);
        _r->ErrorReporting(vkGetSwapchainImagesKHR(_r->GetVulkanDevice(), _swapchain, &_swapchain_image_count, x_swapchain_images.data()));
    }

}
void Window::DestroySwapchain()
{
    vkDestroySwapchainKHR(_r->GetVulkanDevice(),_swapchain, nullptr);
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




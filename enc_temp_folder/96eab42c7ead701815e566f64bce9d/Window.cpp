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
    _width = xrect.right+1;
    _height = xrect.bottom+1;

    _r = prenderer; // set pointer to renderer

    InitOsSurface();

    // Process Windows Messages
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterClass(CLASS_NAME, _app_instance);
}

Window::~Window()
{
    DestroyOsSurface();
    DestroyWindow(_window_handle);
}
void Window::InitOsSurface()
{
    VkWin32SurfaceCreateInfoKHR xwin32surface_create_info {};
    xwin32surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    xwin32surface_create_info.hwnd = _window_handle;
    xwin32surface_create_info.hinstance = _app_instance;

    vkCreateWin32SurfaceKHR(_r->GetVulkanInstance(), &xwin32surface_create_info, nullptr, &_surface);
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




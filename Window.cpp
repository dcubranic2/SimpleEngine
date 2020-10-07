#include "Window.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Window::Window(const char* pname)
{
	_app_instance = GetModuleHandle(NULL);
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Vulkan Window Class";

    wchar_t xtext_wchar[30];
    size_t xlength = 0;
    mbstowcs_s(&xlength, xtext_wchar, pname, strlen(pname) + 1);

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = _app_instance;
    wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        xtext_wchar,// Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,           // Parent window    
        NULL,           // Menu
        _app_instance,  // Instance handle
        NULL            // Additional application data
    );

    if (hwnd == NULL)
    {
        std::exit(-1);
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);

}


Window::~Window()
{


}
void OnSize(HWND hwnd, UINT flag, int width, int height)
{
    // Handle resizing
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SIZE:
        {
            int width = LOWORD(lParam);  // Macro to get the low-order word.
            int height = HIWORD(lParam); // Macro to get the high-order word.

            // Respond to the message:
            OnSize(hwnd, (UINT)wParam, width, height);
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}




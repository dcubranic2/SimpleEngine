#pragma once
#ifdef _WIN32
#include<Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR 1
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#else
#error Platform not yet supported
#endif

#pragma once
#ifdef _WIN32
#include<Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#else
#error Platform not yet supported
#endif

#include "Renderer.h"
#include "Window.h"




void Renderer::SetupLayersAndExtensions()
{
	_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	_instance_extensions.push_back(PLATFORM_SURFACE_EXTENSION_NAME);
}

void Renderer::InitInstance()
{
	VkApplicationInfo xapplication_info {};
	xapplication_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	xapplication_info.apiVersion = VK_API_VERSION_1_0; //version of the Vulkan API
	xapplication_info.pApplicationName = "SimpleEngineVulkanApi";
	xapplication_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo xinstance_create_info {};
	xinstance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	xinstance_create_info.pApplicationInfo = &xapplication_info;
	xinstance_create_info.enabledExtensionCount = _instance_extensions.size();
	xinstance_create_info.ppEnabledExtensionNames = _instance_extensions.data();
	xinstance_create_info.enabledLayerCount = _instance_layers.size();
	xinstance_create_info.ppEnabledLayerNames = _instance_layers.data();
	xinstance_create_info.pNext = &x_debug_report_create_info; // this is to debug CreateInstance
	ErrorReporting(vkCreateInstance(&xinstance_create_info, nullptr, &_instance));
}
VKAPI_ATTR VkBool32 VKAPI_CALL pvkDebugReportCallbackEXT(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData)
{
	std::ostringstream xostr;
	xostr << "Debug messages:" << std::endl;
	xostr << "===============" << std::endl;
	switch (flags) {
		case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
			xostr << "Info";
		break;
		case VK_DEBUG_REPORT_WARNING_BIT_EXT:
			xostr << "Warning";
		break;
		case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
			xostr << "Perf. Warning";
		break;
		case VK_DEBUG_REPORT_ERROR_BIT_EXT:
			xostr << "Error";
		break;
		case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
			xostr << "Debug";
		break;
	}
	xostr << pMessage << "@layer:"<< pLayerPrefix<<std::endl;
	std::cout << xostr.str();
	#ifdef _WIN32
	if (flags==VK_DEBUG_REPORT_ERROR_BIT_EXT)
		MessageBoxA(NULL, xostr.str().c_str(), "Error Message", 0);
	#endif // _WIN32

	return false;
}

void Renderer::DestroyInstance()
{
	vkDestroyInstance(_instance, nullptr);
	_instance = nullptr;
}

void Renderer::InitDevice()
{
	{
		//1. return number of physical devices - GPUs
		uint32_t x_device_count;
		vkEnumeratePhysicalDevices(_instance, &x_device_count, nullptr);

		std::vector<VkPhysicalDevice> x_devices(x_device_count);

		//2. return all physical devices - all GPUs
		vkEnumeratePhysicalDevices(_instance, &x_device_count, x_devices.data());

		//3. take first device as main
		_gpu = x_devices[0];

		// if you need GPU name or GPU type
		VkPhysicalDeviceProperties xdevice_properties{};
		vkGetPhysicalDeviceProperties(_gpu, &xdevice_properties);
	}
	{
		uint32_t x_queue_family_propeties_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &x_queue_family_propeties_count, nullptr);
		std::vector<VkQueueFamilyProperties> x_queue_family_properties(x_queue_family_propeties_count);
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &x_queue_family_propeties_count, x_queue_family_properties.data());
		bool xfound = false;
		for (size_t x = 0;x < x_queue_family_properties.size();x++)
		{
			if (x_queue_family_properties[x].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				xfound = true;
				if (xfound)
				{
					_queue_family_index = x;
					break;
				}
			}
		}

		if (!xfound)
		{
			assert(0 && "Vulkan ERROR: Que family supporting graphics not found.");
			std::exit(-1);
		}
	}
	{
		std::cout << "Vulkan instance layers:" << std::endl;
		uint32_t x_instance_layer_count = 0;
		vkEnumerateInstanceLayerProperties(&x_instance_layer_count,nullptr);
		std::vector<VkLayerProperties> x_layer_properties(x_instance_layer_count);
		vkEnumerateInstanceLayerProperties(&x_instance_layer_count, x_layer_properties.data());
		for (uint32_t x = 0; x < x_instance_layer_count;x++)
		{
			std::cout << x_layer_properties[x].layerName << "|"<< x_layer_properties[x].description << "|" << x_layer_properties[x].implementationVersion<< "|" << x_layer_properties[x].specVersion << std::endl;
		}
	}
	{
		std::cout << "Vulkan device layers:" << std::endl;
		uint32_t x_device_layer_count = 0;
		vkEnumerateDeviceLayerProperties(_gpu,&x_device_layer_count, nullptr);
		std::vector<VkLayerProperties> x_device_properties(x_device_layer_count);
		vkEnumerateDeviceLayerProperties(_gpu, &x_device_layer_count, x_device_properties.data());
		for (uint32_t x = 0; x < x_device_layer_count;x++)
		{
			std::cout << x_device_properties[x].layerName << "|" << x_device_properties[x].description << "|" << x_device_properties[x].implementationVersion << "|" << x_device_properties[x].specVersion << std::endl;
		}
	}
	// 4. choose graphics family on GPU with one queue --> Intel cards generally have this configuration
	float x_queue_priorities[]{1.0f};
	VkDeviceQueueCreateInfo x_device_queue_create_info {};
	x_device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	x_device_queue_create_info.queueFamilyIndex = _queue_family_index;
	x_device_queue_create_info.queueCount = 1;
	x_device_queue_create_info.pQueuePriorities = x_queue_priorities;

	VkDeviceCreateInfo x_device_create_info {};
	x_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	x_device_create_info.queueCreateInfoCount = 1;
	x_device_create_info.pQueueCreateInfos = &x_device_queue_create_info;
	x_device_create_info.enabledExtensionCount = _device_extensions.size();
	x_device_create_info.ppEnabledExtensionNames = _device_extensions.data();
	x_device_create_info.enabledLayerCount = _device_layers.size();
	x_device_create_info.ppEnabledLayerNames = _device_layers.data();

	ErrorReporting(vkCreateDevice(_gpu, &x_device_create_info, nullptr, &_device));

	//get queue from device
	vkGetDeviceQueue(_device, _queue_family_index, 0, &_queue);
}

void Renderer::DestroyDevice()
{
	vkDestroyDevice(_device, nullptr);
}
void Renderer::InitWindow()
{
	_window = new Window(this,"My Window", 800, 600);
}
void Renderer::DestroyWindow()
{
	delete _window;
}

void Renderer::InitSurface()
{
	//vkSurface 
}

void Renderer::DestroySurface()
{
}

#ifdef BUILD_ENABLE_VULKAN_DEBUG
void Renderer::SetupDebug()
{
	x_debug_report_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	x_debug_report_create_info.pfnCallback = pvkDebugReportCallbackEXT;
	x_debug_report_create_info.flags =
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT &
		VK_DEBUG_REPORT_WARNING_BIT_EXT &
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT &
		VK_DEBUG_REPORT_ERROR_BIT_EXT &
		VK_DEBUG_REPORT_DEBUG_BIT_EXT &
		VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;

	_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
	_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	_device_layers.push_back("VK_LAYER_KHRONOS_validation");
	_device_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	//_instance_extensions.push_back("VK_EXT_debug_report");
}


//typedef  VkResult  ( VKAPI_PTR *fpn_vkCreateDebugReportCallbackEXT)(VkInstance, const VkDebugReportCallbackCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugReportCallbackEXT*);
//typedef void (VKAPI_PTR * fpn_vkDestroyDebugReportCallbackEXT)(VkInstance, VkDebugReportCallbackEXT, const VkAllocationCallbacks*);
//fpn_vkCreateDebugReportCallbackEXT fp_vkCreateDebugReportCallbackEXT = nullptr;
//fpn_vkDestroyDebugReportCallbackEXT fp_vkDestroyDebugReportCallbackEXT = nullptr;

PFN_vkCreateDebugReportCallbackEXT fp_vkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT fp_vkDestroyDebugReportCallbackEXT;

void Renderer::InitDebug()
{
	fp_vkCreateDebugReportCallbackEXT =(PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fp_vkDestroyDebugReportCallbackEXT =(PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");

	if (fp_vkCreateDebugReportCallbackEXT == nullptr || fp_vkDestroyDebugReportCallbackEXT == nullptr)
	{
		assert(0 && "Vulkan ERROR : Can't fetch debug function pointers.");
		std::exit(-1);
	}
	ErrorReporting(fp_vkCreateDebugReportCallbackEXT(_instance, &x_debug_report_create_info, NULL, &_debug_report_callbeck_ext));
}

void Renderer::DestroyDebug()
{
	fp_vkDestroyDebugReportCallbackEXT(_instance, _debug_report_callbeck_ext, nullptr);
	_debug_report_callbeck_ext = NULL;
}
#else
void Renderer::SetupDebug()
{
}
void Renderer::InitDebug()
{
}
void Renderer::DestroyDebug()
{
}
#endif // BUILD_ENABLE_VULKAN_DEBUG

#ifdef BUILD_ENABLE_VULKAN_RUNTIME_DEBUG
void Renderer::ErrorReporting(VkResult perror)
{
	if (perror < 0) 
	{
		switch (perror)
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			std::cout << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
			break;
		case VK_ERROR_DEVICE_LOST:
			std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			std::cout << "VK_ERROR_MEMORY_MAP_FAILED" << std::endl;
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			std::cout << "VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			std::cout << "VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			std::cout << "VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			std::cout << "VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			std::cout << "VK_ERROR_TOO_MANY_OBJECTS " << std::endl;
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			std::cout << "VK_ERROR_FORMAT_NOT_SUPPORTED" << std::endl;
			break;
		case VK_ERROR_FRAGMENTED_POOL:
			std::cout << "VK_ERROR_FRAGMENTED_POOL" << std::endl;
			break;
		case VK_ERROR_UNKNOWN:
			std::cout << "VK_ERROR_UNKNOWN" << std::endl;
			break;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			std::cout << "VK_ERROR_OUT_OF_POOL_MEMORY" << std::endl;
			break;
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			std::cout << "VK_ERROR_INVALID_EXTERNAL_HANDLE" << std::endl;
			break;
		case VK_ERROR_FRAGMENTATION:
			std::cout << "VK_ERROR_FRAGMENTATION" << std::endl;
			break;
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			std::cout << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" << std::endl;
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			std::cout << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" << std::endl;
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			std::cout << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" << std::endl;
			break;
		case VK_ERROR_VALIDATION_FAILED_EXT:
			std::cout << "VK_ERROR_VALIDATION_FAILED_EXT" << std::endl;
			break;
		case VK_ERROR_INVALID_SHADER_NV:
			std::cout << "VK_ERROR_INVALID_SHADER_NV" << std::endl;
			break;
		case VK_ERROR_INCOMPATIBLE_VERSION_KHR:
			std::cout << "VK_ERROR_INCOMPATIBLE_VERSION_KHR" << std::endl;
			break;
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			std::cout << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" << std::endl;
			break;
		case VK_ERROR_NOT_PERMITTED_EXT:
			std::cout << "VK_ERROR_NOT_PERMITTED_EXT" << std::endl;
			break;
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			std::cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << std::endl;
			break;
		default:
			std::cout << "VK_ERROR_UNDEFINED" << std::endl;
			break;
		}
		assert(0 && "Vulkan ERROR: Runtime error in Vulkan.");
		std::exit(-1);
	}
}
const VkInstance Renderer::GetVulkanInstance() const
{
	return _instance;
}
const VkPhysicalDevice Renderer::GetVulkanPhysicalDevice() const
{
	return _gpu;
}
const VkDevice Renderer::GetVulkanDevice() const
{
	return _device;
}
const VkQueue Renderer::GetVulkanQueue() const
{
	return _queue;
}
const uint32_t Renderer::GetVulkanGraphicsQueueFamilyIndex() const
{
	return _queue_family_index;
}

#else
void Renderer::ErrorReporting(VkResult perror)
{
}
#endif //BUILD_ENABLE_VULKAN_RUNTIME_DEBUG

Renderer::Renderer()
{
	SetupLayersAndExtensions();
	SetupDebug();
	InitInstance();
	InitDebug();
	InitDevice();
	InitWindow();

	/* Create a command pool */
	VkCommandPool xcmd_pool;
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = _queue_family_index;
	cmd_pool_info.flags = 0;

	auto res = vkCreateCommandPool(_device, &cmd_pool_info, NULL, &xcmd_pool);
	assert(res == VK_SUCCESS);	
}

Renderer::~Renderer()
{
	DestroyDevice();
	DestroyDebug();
	DestroyInstance();
	DestroyWindow();
}

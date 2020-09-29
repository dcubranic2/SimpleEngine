#include "Renderer.h"




void Renderer::InitInstance()
{
	VkApplicationInfo xapplication_info {};
	xapplication_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	xapplication_info.apiVersion = VK_API_VERSION_1_2; //version of the Vulkan API
	xapplication_info.pApplicationName = "SimpleEngineVulkanApi";
	xapplication_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo xinstance_create_info {};
	xinstance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	xinstance_create_info.pApplicationInfo = &xapplication_info;
	xinstance_create_info.enabledExtensionCount = _instance_extensions.size();
	xinstance_create_info.ppEnabledExtensionNames = _instance_extensions.data();
	xinstance_create_info.enabledLayerCount = _instance_layers.size();
	xinstance_create_info.ppEnabledLayerNames = _instance_layers.data();
	ErrorReporting(vkCreateInstance(&xinstance_create_info, nullptr, &_instance));
}
VkBool32 VKAPI_CALL vkDebugReportCallbackEXT(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData)
{
	std::cout << pMessage << std::endl;
	return true;
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
		uint32_t x_instance_layer_count = 0;
		vkEnumerateInstanceLayerProperties(&x_instance_layer_count,nullptr);
		std::vector<VkLayerProperties> x_layer_properties(x_instance_layer_count);
		vkEnumerateInstanceLayerProperties(&x_instance_layer_count, x_layer_properties.data());
		for (uint32_t x = 0; x < x_instance_layer_count;x++)
		{
			std::cout << x_layer_properties[x].layerName << std::endl;
		}
	}
	{
		uint32_t x_device_layer_count = 0;
		vkEnumerateDeviceLayerProperties(_gpu,&x_device_layer_count, nullptr);
		std::vector<VkLayerProperties> x_device_properties(x_device_layer_count);
		vkEnumerateDeviceLayerProperties(_gpu, &x_device_layer_count, x_device_properties.data());
		for (uint32_t x = 0; x < x_device_layer_count;x++)
		{
			std::cout << x_device_properties[x].layerName << std::endl;
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
}

void Renderer::DestroyDevice()
{
	vkDestroyDevice(_device, nullptr);
}

void Renderer::SetupDebug()
{
	_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
	_device_layers.push_back("VK_LAYER_KHRONOS_validation");
	_instance_extensions.push_back("VK_EXT_debug_report");
}

typedef VkResult(*fpn_vkCreateDebugReportCallbackEXT)(VkInstance, const VkDebugReportCallbackCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugReportCallbackEXT*);
typedef void (*fpn_vkDestroyDebugReportCallbackEXT)(VkInstance, VkDebugReportCallbackEXT, const VkAllocationCallbacks*);
fpn_vkCreateDebugReportCallbackEXT fp_vkCreateDebugReportCallbackEXT = nullptr;
fpn_vkDestroyDebugReportCallbackEXT fp_vkDestroyDebugReportCallbackEXT = nullptr;

void Renderer::InitDebug()
{
	fp_vkCreateDebugReportCallbackEXT =(fpn_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
	fp_vkDestroyDebugReportCallbackEXT =(fpn_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");

	if (fp_vkCreateDebugReportCallbackEXT == nullptr || fp_vkDestroyDebugReportCallbackEXT == nullptr)
	{
		assert(0 && "Vulkan ERROR : Can't fetch debug function pointers.");
		std::exit(-1);
	}

	VkDebugReportCallbackCreateInfoEXT x_debug_report_create_info{};
	x_debug_report_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	x_debug_report_create_info.pfnCallback = &vkDebugReportCallbackEXT;
	x_debug_report_create_info.flags =
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT &
		VK_DEBUG_REPORT_WARNING_BIT_EXT &
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT &
		VK_DEBUG_REPORT_ERROR_BIT_EXT &
		VK_DEBUG_REPORT_DEBUG_BIT_EXT &
		VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
	fp_vkCreateDebugReportCallbackEXT(_instance, &x_debug_report_create_info, nullptr, &_debug_report_ext);
}

void Renderer::DestroyDebug()
{
	fp_vkDestroyDebugReportCallbackEXT(_instance, _debug_report_ext, nullptr);
	_debug_report_ext = NULL;
}

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

Renderer::Renderer()
{
	SetupDebug();
	InitInstance();
	InitDebug();
	InitDevice();
	
}

Renderer::~Renderer()
{
	DestroyDevice();
	DestroyDebug();
	DestroyInstance();
}

#include "VkCSetup.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
VkCS_DefaultCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                     void *pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    fprintf(stderr, "Validation layer: %s\n", pCallbackData->pMessage);
  }
  return VK_FALSE;
}

VkResult VkCS_CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != NULL) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void VkCS_DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  PFN_vkDestroyDebugUtilsMessengerEXT func =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != NULL) {
    func(instance, debugMessenger, pAllocator);
  }
}

bool VkCS_isDeviceSuitable(VkPhysicalDevice *Device) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(*Device, &deviceProperties);

  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(*Device, NULL, &extensionCount, NULL);

  VkExtensionProperties *availableExtensions =
      malloc(extensionCount * sizeof(VkExtensionProperties));
  vkEnumerateDeviceExtensionProperties(*Device, NULL, &extensionCount,
                                       availableExtensions);

  bool SwapChainSupport = false;

  for (int i = 0; i < extensionCount; i++) {
    if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
               availableExtensions[i].extensionName) == 0) {
      SwapChainSupport = true;
      break;
    }
  }

  free(availableExtensions);

  return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
         SwapChainSupport;
};

/// function to create a vulkan instance
InstanceBuilderReturn VkCS_BuildInstance(InstanceBuilder *Builder) {
  InstanceBuilderReturn IBR;

  VkApplicationInfo AppInfo = {};
  AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  AppInfo.pApplicationName = Builder->AppName;
  AppInfo.applicationVersion = Builder->AppVersion;
  AppInfo.pEngineName = Builder->EngineName;
  AppInfo.engineVersion = Builder->EngineVersion;
  AppInfo.apiVersion = Builder->ApiVersion;

  VkInstanceCreateInfo InstanceCreateInfo = {};
  InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  InstanceCreateInfo.pApplicationInfo = &AppInfo;
  InstanceCreateInfo.enabledExtensionCount = Builder->ExtentionCount;
  InstanceCreateInfo.ppEnabledExtensionNames = Builder->ExtentionNames;
  InstanceCreateInfo.enabledLayerCount = Builder->LayerCount;
  InstanceCreateInfo.ppEnabledLayerNames = Builder->LayerNames;

  if (Builder->EnableDebugMessager) {
    bool FoundValidaitonLayer = false;
    for (int i = 0; i < Builder->LayerCount; i++) {
      if (strcmp("VK_LAYER_KHRONOS_validation", Builder->LayerNames[i]) == 0) {
        FoundValidaitonLayer = true;
        break;
      }
    }

    if (!FoundValidaitonLayer) {
      VkCS_LOG(
          "[VkCS Error]: if Builder.EnableDebugMessager is set to true "
          "Builder.LayerNames must contain VK_LAYER_KHRONOS_validation!\n");
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    if (Builder->DebugCallback != NULL) {
      debugCreateInfo.pfnUserCallback = Builder->DebugCallback;
    } else {
      debugCreateInfo.pfnUserCallback = VkCS_DefaultCallback;
    }
    InstanceCreateInfo.pNext =
        (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  }

  if (Builder->EnableDebugMessager) {
    bool FoundValidaitonLayer = false;
    for (int i = 0; i < Builder->ExtentionCount; i++) {
      if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                 Builder->ExtentionNames[i]) == 0) {
        FoundValidaitonLayer = true;
        break;
      }
    }

    if (!FoundValidaitonLayer) {
      VkCS_LOG("[VkCS Error]: if Builder.EnableDebugMessager is set to true "
               "Builder.ExtentionNames must contain "
               "VK_EXT_DEBUG_UTILS_EXTENSION_NAME!\n");
    }
  }

  if (Builder->ExtentionCount > 0) {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties *extensions =
        malloc(extensionCount * sizeof(VkExtensionProperties));

    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    int FoundExtentions = 0;
    for (int i = 0; i < Builder->ExtentionCount; i++) {
      for (int j = 0; j < extensionCount; j++) {
        if (strcmp(extensions[j].extensionName, Builder->ExtentionNames[i]) ==
            0) {
          ++FoundExtentions;
          break;
        }
      }
      if (FoundExtentions == Builder->ExtentionCount) {
        break;
      }
    };

    if (FoundExtentions != Builder->ExtentionCount) {
      VkCS_LOG("[VkCS Error]: Not all requested extentions could be found!\n");
    };

    free(extensions);
  }

  if (Builder->LayerCount > 0) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *layers = malloc(layerCount * sizeof(VkLayerProperties));

    vkEnumerateInstanceLayerProperties(&layerCount, layers);
    int FoundLayers = 0;
    for (int i = 0; i < Builder->LayerCount; i++) {
      for (int j = 0; j < layerCount; j++) {
        if (strcmp(layers[j].layerName, Builder->LayerNames[i]) == 0) {
          ++FoundLayers;
          break;
        }

        if (FoundLayers == Builder->LayerCount) {
          break;
        }
      }
    };
    if (FoundLayers != Builder->LayerCount) {
      VkCS_LOG("[VkCS Error]: Not all requested layers could be found!\n");
    };
    free(layers);
  }

  VkDebugUtilsMessengerCreateInfoEXT DebugMessangerCreateInfo = {};
  DebugMessangerCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  DebugMessangerCreateInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  DebugMessangerCreateInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  if (Builder->DebugCallback != NULL) {
    DebugMessangerCreateInfo.pfnUserCallback = Builder->DebugCallback;
  } else {
    DebugMessangerCreateInfo.pfnUserCallback = VkCS_DefaultCallback;
  }
  DebugMessangerCreateInfo.pUserData = NULL;

  VkResult Result = vkCreateInstance(&InstanceCreateInfo, NULL, &IBR.Instance);
  if (Result != VK_SUCCESS) {
    VkCS_LOG("[VkCS Error]: Failed to create Vulkan Instance!\n");
  }

  if (VkCS_CreateDebugUtilsMessengerEXT(IBR.Instance, &DebugMessangerCreateInfo,
                                        NULL,
                                        &IBR.DebugMessenger) != VK_SUCCESS) {
    VkCS_LOG("[VkCS Error]: Failed to create Debug Messenger\n");
  };

  return IBR;
};

PhysicalDeviceBuilderReturn
VkCS_BuildPhysicalDevice(PhysicalDeviceBuilder *Builder) {
  PhysicalDeviceBuilderReturn PDBR;

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(*Builder->Instance, &deviceCount, NULL);

  VkPhysicalDevice *physicalDevices =
      malloc(deviceCount * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(*Builder->Instance, &deviceCount, physicalDevices);

  int DevIndex = -1;
  for (int i = 0; i < deviceCount; i++) {
    if (Builder->isSuitableDevice != NULL) {
      if (Builder->isSuitableDevice(&physicalDevices[i])) {
        DevIndex = i;
        break;
      }
    } else {
      if (VkCS_isDeviceSuitable(&physicalDevices[i])) {
        DevIndex = i;
        break;
      }
    }
  }

  if (DevIndex == -1) {
    VkCS_LOG("[VkCS Error]: No suitable device found!");
  } else {
    PDBR.PhysicalDevice = physicalDevices[DevIndex];
  }

  free(physicalDevices);

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(PDBR.PhysicalDevice,
                                           &queueFamilyCount, NULL);

  VkQueueFamilyProperties *queueFamilies =
      malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(PDBR.PhysicalDevice,
                                           &queueFamilyCount, queueFamilies);

  PDBR.GraphicsQueueIndex = -1;
  PDBR.PresentQueueIndex = -1;
  for (int i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      PDBR.GraphicsQueueIndex = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(PDBR.PhysicalDevice, i,
                                         *Builder->Surface, &presentSupport);
    if (presentSupport) {
      PDBR.PresentQueueIndex = i;
    }

    if (PDBR.GraphicsQueueIndex != -1 && PDBR.PresentQueueIndex != -1) {
      break;
    }
  }

  return PDBR;
};

DeviceBuilderReturn VkCS_BuildLogicalDevice(DeviceBuilder *Builder) {
  DeviceBuilderReturn DBR;

  int count = 1;
  if (Builder->PDBuilderReturn->GraphicsQueueIndex !=
      Builder->PDBuilderReturn->PresentQueueIndex) {
    count = 2;
  }

  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo QueueCreateInfo = {};
  QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueCreateInfo.queueFamilyIndex =
      Builder->PDBuilderReturn->PresentQueueIndex;
  QueueCreateInfo.queueCount = 1;
  QueueCreateInfo.pQueuePriorities = &queuePriority;

  VkDeviceQueueCreateInfo GraphicsQueuecreateInfo = {};
  if (count > 1) {
    GraphicsQueuecreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    GraphicsQueuecreateInfo.queueFamilyIndex =
        Builder->PDBuilderReturn->GraphicsQueueIndex;
    GraphicsQueuecreateInfo.queueCount = 1;
    GraphicsQueuecreateInfo.pQueuePriorities = &queuePriority;
  }

  VkDeviceCreateInfo DeviceCreateInfo = {};
  DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  if (count > 1) {
    VkDeviceQueueCreateInfo QueueInfos[] = {QueueCreateInfo,
                                            GraphicsQueuecreateInfo};
    DeviceCreateInfo.pQueueCreateInfos = QueueInfos;
  } else {
    DeviceCreateInfo.pQueueCreateInfos = &QueueCreateInfo;
  }

  DeviceCreateInfo.queueCreateInfoCount = count;
  DeviceCreateInfo.pEnabledFeatures = Builder->deviceFeatures;
  DeviceCreateInfo.enabledLayerCount = Builder->LayerCount;
  DeviceCreateInfo.ppEnabledLayerNames = Builder->LayerNames;
  DeviceCreateInfo.enabledExtensionCount = 1;

  const char *DeviceExtentions = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  DeviceCreateInfo.ppEnabledExtensionNames = &DeviceExtentions;

  if (vkCreateDevice(Builder->PDBuilderReturn->PhysicalDevice,
                     &DeviceCreateInfo, NULL, &DBR.Device) != VK_SUCCESS) {
    VkCS_LOG("[VkCS Error]: Failed to create logical Device");
  }

  vkGetDeviceQueue(DBR.Device, Builder->PDBuilderReturn->GraphicsQueueIndex, 0,
                   &DBR.GraphicsQueue);
  vkGetDeviceQueue(DBR.Device, Builder->PDBuilderReturn->PresentQueueIndex, 0,
                   &DBR.PresentQueue);

  return DBR;
};

SwapChainBuilderReturn VkCS_BuildSwapChain(SwapChainBuilder *Builder) {
  SwapChainBuilderReturn SCBR;
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR *formats = NULL;
  VkPresentModeKHR *presentModes = NULL;

  return SCBR;
};
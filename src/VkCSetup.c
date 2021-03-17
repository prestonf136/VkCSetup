#include "VkCSetup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

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

  printf("Device Name: %s\n", deviceProperties.deviceName);

  return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
};

/// function to create a vulkan instance
InstanceBuilderReturn VkC_BuildInstance(InstanceBuilder *Builder) {
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
      LOG("[VkC Error]: if Builder.EnableDebugMessager is set to true "
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
      LOG("[VkC Error]: if Builder.EnableDebugMessager is set to true "
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
      LOG("[VkC Error]: Not all requested extentions could be found!\n");
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
      LOG("[VkC Error]: Not all requested layers could be found!\n");
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
    LOG("[VkC Error]: Failed to create Vulkan Instance!\n");
  }

  if (VkCS_CreateDebugUtilsMessengerEXT(IBR.Instance, &DebugMessangerCreateInfo,
                                        NULL,
                                        &IBR.DebugMessenger) != VK_SUCCESS) {
    LOG("[VkC Error]: Failed to create Debug Messenger");
  };

  return IBR;
};

PhysicalDeviceBuilderReturn
VkC_BuildPhysicalDevice(PhysicalDeviceBuilder Builder) {
  PhysicalDeviceBuilderReturn PDBR;

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(*Builder.Instance, &deviceCount, NULL);

  VkPhysicalDevice *physicalDevices =
      malloc(deviceCount * sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(*Builder.Instance, &deviceCount, physicalDevices);

  int DevIndex = -1;
  for (int i = 0; i < deviceCount; i++) {
    if (Builder.isSuitableDevice != NULL) {
      if (Builder.isSuitableDevice(&physicalDevices[i])) {
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

  free(physicalDevices);

  return PDBR;
};
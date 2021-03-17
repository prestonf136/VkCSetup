#include "VkCSetup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  VkResult Result = vkCreateInstance(&InstanceCreateInfo, NULL, &IBR.Instance);
  if (Result != VK_SUCCESS) {
    LOG("[VkC Error]: Failed to create Vulkan Instance!\n");
  }

  return IBR;
};
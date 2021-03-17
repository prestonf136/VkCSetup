#include "../include/VkCSetup.h"
#include <stdio.h>
#include <stdlib.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

  return VK_FALSE;
}

bool isDeviceSuitable(VkPhysicalDevice *Device) {
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(*Device, &deviceProperties);

  printf("Device Name: %s\n", deviceProperties.deviceName);

  return true;
};

int main() {
  InstanceBuilder IB;
  IB.AppName = "Hello World!";
  IB.AppVersion = VK_MAKE_VERSION(1, 0, 0);
  IB.EngineName = "Vleng";
  IB.EngineVersion = VK_MAKE_VERSION(1, 0, 0);
  IB.ApiVersion = VK_API_VERSION_1_0;

  IB.LayerCount = 1;
  const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
  IB.LayerNames = validationLayers;

  IB.ExtentionCount = 2;
  const char *Extentions[] = {"VK_KHR_surface",
                              VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
  IB.ExtentionNames = Extentions;

  IB.EnableDebugMessager = true;
  IB.DebugCallback = NULL;

  InstanceBuilderReturn IBR = VkC_BuildInstance(&IB);

  PhysicalDeviceBuilder DB;
  DB.isSuitableDevice = isDeviceSuitable;
  DB.Instance = &IBR.Instance;

  VkC_BuildPhysicalDevice(DB);

  VkCS_DestroyDebugUtilsMessengerEXT(IBR.Instance, IBR.DebugMessenger, NULL);
}

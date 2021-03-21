#include "../include/VkCSetup.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

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

  return true;
};

int main() {
  if (!glfwInit()) {
    VkCS_LOG("GLFW NOT INITIALIZED");
  }

  InstanceBuilder IB;
  IB.AppName = "Hello World!";
  IB.AppVersion = VK_MAKE_VERSION(1, 0, 0);
  IB.EngineName = "Vleng";
  IB.EngineVersion = VK_MAKE_VERSION(1, 0, 0);
  IB.ApiVersion = VK_API_VERSION_1_0;

  IB.LayerCount = 1;
  const char *validationLayers[] = {"VK_LAYER_KHRONOS_validation"};
  IB.LayerNames = validationLayers;

  uint32_t count;
  const char **extensions = glfwGetRequiredInstanceExtensions(&count);

  const char **Exts = malloc((count + 1) * sizeof(const char *));

  for (int i = 0; i < count; i++) {
    Exts[i] = extensions[i];
  }

  Exts[count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

  IB.ExtentionCount = count + 1;
  IB.ExtentionNames = Exts;

  IB.EnableDebugMessager = true;
  IB.DebugCallback = NULL;

  InstanceBuilderReturn IBR = VkCS_BuildInstance(&IB);

  PhysicalDeviceBuilder DB;
  DB.isSuitableDevice = isDeviceSuitable;
  DB.Instance = &IBR.Instance;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow *window;
  window = glfwCreateWindow(640, 480, "Window Title", NULL, NULL);

  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(IBR.Instance, window, NULL, &surface) !=
      VK_SUCCESS) {
    VkCS_LOG("no surface");
  };
  DB.Surface = &surface;

  PhysicalDeviceBuilderReturn DBRP = VkCS_BuildPhysicalDevice(&DB);

  DeviceBuilder DBB = {};
  VkPhysicalDeviceFeatures deviceFeatures = {};
  DBB.deviceFeatures = &deviceFeatures;

  DBB.LayerCount = 1;
  DBB.LayerNames = validationLayers;
  DBB.PDBuilderReturn = &DBRP;

  VkCS_BuildLogicalDevice(&DBB);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  VkCS_DestroyDebugUtilsMessengerEXT(IBR.Instance, IBR.DebugMessenger, NULL);
}

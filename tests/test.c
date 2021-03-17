#include "../include/VkCSetup.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  InstanceBuilder IB;
  IB.AppName = "Hello World!";
  IB.AppVersion = VK_MAKE_VERSION(1, 0, 0);
  IB.EngineName = "Vleng";
  IB.EngineVersion = VK_MAKE_VERSION(1, 0, 0);
  IB.ApiVersion = VK_API_VERSION_1_0;

  IB.LayerCount = 1;
  const char *validationLayers[] = {"VK_LAYER_KHRONOS_svalidation"};
  IB.LayerNames = validationLayers;

  IB.ExtentionCount = 2;
  const char *Extentions[] = {"VK_KHR_susrface",
                              VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
  IB.ExtentionNames = Extentions;

  IB.EnableDebugMessager = true;

  VkC_BuildInstance(&IB);
}

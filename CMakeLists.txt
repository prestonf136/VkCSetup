project (VulkanCSetup)
cmake_minimum_required(VERSION 3.10)

find_package(Vulkan QUIET)

add_library(VkCSetup INTERFACE)
add_library(VkCSetup::VkCSetup ALIAS VkCSetup)

add_library(
    VkCSetupL
    "include/VkCSetup.h"
    "src/VkCSetup.c"
)

target_include_directories(VkCSetupL PUBLIC "include/")
target_link_libraries(VkCSetupL INTERFACE Vulkan::Vulkan VkCSetupL)

add_executable(Test "tests/test.c")
target_link_libraries(Test VkCSetupL)
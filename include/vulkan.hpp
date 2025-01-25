#pragma once

#define GLFW_INCLUDE_VULKAN

#include "../include/particle.hpp"
#include "../include/renderer.hpp"
#include "../utils/vkbootstrap/VkBootstrap.h"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <iostream>

class VulkanCompute {
public:
    VulkanCompute() { initVulkan(); }
    ~VulkanCompute() { cleanup(); }

private:
    void initVulkan();
    void cleanup();

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkDevice _device;
    VkPhysicalDevice _chosenGPU;
    VkQueue _computeQueue;
    uint32_t _computeQueueFamily;
  };

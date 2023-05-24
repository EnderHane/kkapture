#pragma once
#include <vulkan/vulkan.h>

#include "video_vulkan_functions.h"


MAKEVKHOOKPAIR(vkCreateDevice);
MAKEVKHOOKPAIR(vkDestroyDevice);
MAKEVKHOOKPAIR(vkCreateSwapchainKHR);
MAKEVKHOOKPAIR(vkDestroySwapchainKHR);
MAKEVKHOOKPAIR(vkQueuePresentKHR);


MAKEVKDYN(vkGetDeviceProcAddr);
MAKEVKDYN(vkGetPhysicalDeviceQueueFamilyProperties);
MAKEVKDYN(vkGetPhysicalDeviceMemoryProperties);


MAKEVKDYN(vkGetDeviceQueue);
MAKEVKDYN(vkCreateCommandPool);
MAKEVKDYN(vkAllocateCommandBuffers);
MAKEVKDYN(vkCreateSemaphore);
MAKEVKDYN(vkCreateFence);

MAKEVKDYN(vkDestroySemaphore);
MAKEVKDYN(vkDestroyFence);
MAKEVKDYN(vkDestroyCommandPool);


MAKEVKDYN(vkGetSwapchainImagesKHR);
MAKEVKDYN(vkCreateImage);
MAKEVKDYN(vkGetImageMemoryRequirements);
MAKEVKDYN(vkAllocateMemory);
MAKEVKDYN(vkBindImageMemory);
MAKEVKDYN(vkGetImageSubresourceLayout);
MAKEVKDYN(vkMapMemory);

MAKEVKDYN(vkDestroyImage);
MAKEVKDYN(vkFreeMemory);


MAKEVKDYN(vkBeginCommandBuffer);
MAKEVKDYN(vkCmdPipelineBarrier);
MAKEVKDYN(vkCmdCopyImage);
MAKEVKDYN(vkEndCommandBuffer);
MAKEVKDYN(vkQueueSubmit);
MAKEVKDYN(vkWaitForFences);
MAKEVKDYN(vkResetFences);

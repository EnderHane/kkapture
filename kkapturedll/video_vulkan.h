#pragma once
#include <vulkan/vulkan.h>

#define _DYN(name) D_##name
#define MAKEVKDYN(name) static decltype(##name) * _DYN(name) = nullptr;
#define WINDYN(module,name) _DYN(name) = (PFN_##name)GetProcAddress(##module, #name);
#define VKDEVDYN(device,func) _DYN(func) = (PFN_##func)_DYN(vkGetDeviceProcAddr)(##device, #func);

#define ___C(a,b) a##b
#define __C(a,b) ___C(a,b)
#define _MINE(name) __C(Mine_, _DYN(name))
#define _REAL(name) __C(Real_, _DYN(name))
#define MAKEVKHOOKPAIR(name) static decltype(##name) _MINE(name);static decltype(##name) * _REAL(name) = nullptr;
#define WINHK(module,name) HookDLLFunction(&_REAL(name), ##module, #name, _MINE(name));
#define HOOKVKDEVICEPROC(device, func) HookFunctionInit(&_REAL(func),(PFN_##func)_DYN(vkGetDeviceProcAddr)(##device, #func),_MINE(func));


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

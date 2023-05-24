#pragma once
#include <vulkan/vulkan.h>

#define _DYN(name) D_##name
#define MAKE_FNPTR(name) static decltype(##name) * _DYN(name) = nullptr;
#define WIN_INIT_PROC_ADDR(module,name) _DYN(name) = (PFN_##name)GetProcAddress(##module, #name);
#define VKDEVICE_INIT_PROC_ADDR(device,func) _DYN(func) = (PFN_##func)_DYN(vkGetDeviceProcAddr)(##device, #func);

#define _MINE(name) Mine_##name
#define _REAL(name) Real_##name
#define MAKE_HOOK_PAIR(name) static decltype(##name) _MINE(name);static decltype(##name) * _REAL(name) = nullptr;
#define HOOK_DLL(module,name) HookDLLFunction(&_REAL(name), ##module, #name, _MINE(name));
#define HOOK_VKDEVICE_PROC(device, func) HookFunctionInit(&_REAL(func),(PFN_##func)_DYN(vkGetDeviceProcAddr)(##device, #func),_MINE(func));

#define VK_FNPTRS \
XXX_MACRO(vkGetDeviceProcAddr) \
XXX_MACRO(vkGetPhysicalDeviceQueueFamilyProperties) \
XXX_MACRO(vkGetPhysicalDeviceMemoryProperties) 

#define VKDEVICE_HOOKS \
XXX_MACRO(vkDestroyDevice) \
XXX_MACRO(vkCreateSwapchainKHR) \
XXX_MACRO(vkDestroySwapchainKHR) \
XXX_MACRO(vkQueuePresentKHR) 

#define VKDEVICE_FNPTRS \
XXX_MACRO(vkGetDeviceQueue) \
XXX_MACRO(vkCreateCommandPool) \
XXX_MACRO(vkAllocateCommandBuffers) \
XXX_MACRO(vkCreateSemaphore) \
XXX_MACRO(vkCreateFence) \
XXX_MACRO(vkDestroySemaphore) \
XXX_MACRO(vkDestroyFence) \
XXX_MACRO(vkDestroyCommandPool) \
XXX_MACRO(vkGetSwapchainImagesKHR) \
XXX_MACRO(vkCreateImage) \
XXX_MACRO(vkGetImageMemoryRequirements) \
XXX_MACRO(vkAllocateMemory) \
XXX_MACRO(vkBindImageMemory) \
XXX_MACRO(vkGetImageSubresourceLayout) \
XXX_MACRO(vkMapMemory) \
XXX_MACRO(vkDestroyImage) \
XXX_MACRO(vkFreeMemory) \
XXX_MACRO(vkBeginCommandBuffer) \
XXX_MACRO(vkCmdPipelineBarrier) \
XXX_MACRO(vkCmdCopyImage) \
XXX_MACRO(vkEndCommandBuffer) \
XXX_MACRO(vkQueueSubmit) \
XXX_MACRO(vkWaitForFences) \
XXX_MACRO(vkResetFences) 


MAKE_HOOK_PAIR(vkCreateDevice);


#define XXX_MACRO MAKE_HOOK_PAIR
VKDEVICE_HOOKS
#undef XXX_MACRO

#define XXX_MACRO MAKE_FNPTR
VK_FNPTRS
VKDEVICE_FNPTRS
#undef XXX_MACRO

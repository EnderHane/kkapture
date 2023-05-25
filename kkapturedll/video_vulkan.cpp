#include "stdafx.h"
#include <vector>
#include <cassert>

#include "video_vulkan.h"

#include "video.h"
#include "videoencoder.h"


static VkDevice device = VK_NULL_HANDLE;
static VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
static VkQueue queue = VK_NULL_HANDLE;
static VkCommandPool commandPool = VK_NULL_HANDLE;
static VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
static VkSemaphore copySemaphore = VK_NULL_HANDLE;
static VkFence copyFence = VK_NULL_HANDLE;

static VkSwapchainKHR swapchain = VK_NULL_HANDLE;
static uint32_t width = 0, height = 0;
static std::vector<VkImage> swapchainImages = {};
static VkImage image = VK_NULL_HANDLE;
static VkDeviceMemory memory = VK_NULL_HANDLE;
static unsigned char* imageData = nullptr;
static uint64_t rowPitch = 0;

static bool formatReady = false;


uint32_t getQueueFamilyIndex(VkPhysicalDevice physicalDeviceIn, uint32_t queueFlagBit)
{
	uint32_t queueFamilyCount = 0;
	_DYN(vkGetPhysicalDeviceQueueFamilyProperties)(physicalDeviceIn, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	_DYN(vkGetPhysicalDeviceQueueFamilyProperties)(physicalDeviceIn, &queueFamilyCount, queueFamilies.data());
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilies.at(i).queueFlags & queueFlagBit)
		{
			return i;
		}
	}
	return 0;
}


uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDeviceIn, uint32_t typeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	_DYN(vkGetPhysicalDeviceMemoryProperties)(physicalDeviceIn, &deviceMemoryProperties);
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}
	return 0;
}


static bool grabFrameVulkan(
	VkImage imagePresenting,
	uint32_t waitSemaphoreCount,
	const VkSemaphore* pWaitSemaphores,
	bool* semaphoreOverride)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	_DYN(vkBeginCommandBuffer)(::commandBuffer, &beginInfo);

	// Transition swapchain image layout
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = imagePresenting;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	_DYN(vkCmdPipelineBarrier)(::commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	// Transition destination image layout
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.image = ::image;
	_DYN(vkCmdPipelineBarrier)(commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	VkImageCopy region = {};
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.layerCount = 1;
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.layerCount = 1;
	region.extent.width = ::width;
	region.extent.height = ::height;
	region.extent.depth = 1;
	_DYN(vkCmdCopyImage)(::commandBuffer,
		imagePresenting, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		::image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region);

	// Transition swapchain image layout back
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.image = imagePresenting;
	_DYN(vkCmdPipelineBarrier)(::commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	// Transition destination image layout back
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = 0;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	// Set this to the original layout of the destination image
	// before it was transitioned to TRANSFER_DST_OPTIMAL
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.image = ::image;
	_DYN(vkCmdPipelineBarrier)(::commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	_DYN(vkEndCommandBuffer)(::commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &::commandBuffer;
	submitInfo.waitSemaphoreCount = waitSemaphoreCount;
	submitInfo.pWaitSemaphores = pWaitSemaphores;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &::copySemaphore;
	auto rQueueSubmit = _DYN(vkQueueSubmit)(::queue, 1, &submitInfo, ::copyFence);
	if (rQueueSubmit != VK_SUCCESS)
	{
		printLog("video/vulkan: submission failed, VkResult %d\n", rQueueSubmit);
		return false;
	}
	*semaphoreOverride = true;
	_DYN(vkWaitForFences)(::device, 1, &::copyFence, VK_TRUE, UINT64_MAX);
	_DYN(vkResetFences)(::device, 1, &::copyFence);

	blitAndFlipBGRAToCaptureData(::imageData, ::rowPitch);
	return true;
}


static VkResult __stdcall _MINE(vkQueuePresentKHR)(
	VkQueue queue_,
	const VkPresentInfoKHR* pPresentInfo)
{
	VkPresentInfoKHR presentInfo{ *pPresentInfo };
	if (::formatReady && params.CaptureVideo && pPresentInfo->pSwapchains[0] == ::swapchain)
	{
		// printLog("video/vulkan: presenting Image %d in Swapchain # %x\n", pPresentInfo->pImageIndices[0], ::swapchain);
		bool override = false;
		if (grabFrameVulkan(
			swapchainImages[pPresentInfo->pImageIndices[0]],
			pPresentInfo->waitSemaphoreCount,
			pPresentInfo->pWaitSemaphores,
			&override
		))
		{
			encoder->WriteFrame(captureData);
		}
		if (override)
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &::copySemaphore;
		}
	}
	auto r = _REAL(vkQueuePresentKHR)(queue_, &presentInfo);
	nextFrame();
	return r;
}

static VkResult __stdcall _MINE(vkCreateDevice)(
	VkPhysicalDevice physicalDevice_,
	const VkDeviceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDevice* pDevice)
{
	assert(::device == VK_NULL_HANDLE);

	VkResult ret = _REAL(vkCreateDevice)(physicalDevice_, pCreateInfo, pAllocator, pDevice);
	if (ret == VK_SUCCESS)
	{
		printLog("video/vulkan: Device # %x created\n", *pDevice);
		::physicalDevice = physicalDevice_;
		::device = *pDevice;

#define XXX_MACRO(f) HOOK_VKDEVICE_PROC(::device,##f)
		VKDEVICE_HOOKS;
#undef XXX_MACRO

#define XXX_MACRO(f) VKDEVICE_INIT_PROC_ADDR(::device,##f)
		VKDEVICE_FNPTRS;
#undef XXX_MACRO

		uint32_t queueFamilyIndex = getQueueFamilyIndex(::physicalDevice, VK_QUEUE_TRANSFER_BIT);
		_DYN(vkGetDeviceQueue)(::device, queueFamilyIndex, 0, &::queue);
		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		_DYN(vkCreateCommandPool)(::device, &commandPoolInfo, nullptr, &::commandPool);
		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandPool = commandPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;
		_DYN(vkAllocateCommandBuffers)(::device, &commandBufferInfo, &commandBuffer);
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		_DYN(vkCreateSemaphore)(::device, &semaphoreInfo, nullptr, &::copySemaphore);
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		_DYN(vkCreateFence)(::device, &fenceInfo, nullptr, &::copyFence);
	}
	return ret;
}

static void __stdcall _MINE(vkDestroyDevice)(
	VkDevice device_,
	const VkAllocationCallbacks* pAllocator)
{
	assert(::device = device_);
	printLog("video/vulkan: Device # %x destroyed\n", device_);

	::queue = VK_NULL_HANDLE;
	_DYN(vkDestroySemaphore)(::device, ::copySemaphore, nullptr);
	_DYN(vkDestroyFence)(::device, ::copyFence, nullptr);
	::copySemaphore = VK_NULL_HANDLE;
	::copyFence = VK_NULL_HANDLE;
	_DYN(vkDestroyCommandPool)(::device, ::commandPool, nullptr);
	::commandPool = VK_NULL_HANDLE;
	_REAL(vkDestroyDevice)(device_, pAllocator);
	::device = VK_NULL_HANDLE;
	::physicalDevice = VK_NULL_HANDLE;

#define XXX_MACRO(f) _DYN(##f)=nullptr;
	VKDEVICE_FNPTRS;
#undef XXX_MACRO

}

static VkResult __stdcall _MINE(vkCreateSwapchainKHR)(
	VkDevice device_,
	const VkSwapchainCreateInfoKHR* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkSwapchainKHR* pSwapchain)
{
	assert(::swapchain == VK_NULL_HANDLE && ::device == device_);

	VkResult ret = _REAL(vkCreateSwapchainKHR)(device_, pCreateInfo, pAllocator, pSwapchain);
	if (ret == VK_SUCCESS)
	{
		::swapchain = *pSwapchain;
		printLog("video/vulkan: Swapchain # %x created\n", *pSwapchain);
		printLog("video/vulkan: Swapchain # %x has image format %d\n", *pSwapchain, pCreateInfo->imageFormat);
		if (pCreateInfo->imageFormat == VK_FORMAT_B8G8R8A8_UNORM)
		{
			::formatReady = true;
		}
		else
		{
			::formatReady = false;
		}

		::width = pCreateInfo->imageExtent.width;
		::height = pCreateInfo->imageExtent.height;
		setCaptureResolution(::width, ::height);
		uint32_t imageCount;
		_DYN(vkGetSwapchainImagesKHR)(::device, *pSwapchain, &imageCount, nullptr);
		::swapchainImages.resize(imageCount);
		_DYN(vkGetSwapchainImagesKHR)(::device, *pSwapchain, &imageCount, ::swapchainImages.data());

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		imageCreateInfo.extent.width = pCreateInfo->imageExtent.width;
		imageCreateInfo.extent.height = pCreateInfo->imageExtent.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		_DYN(vkCreateImage)(::device, &imageCreateInfo, nullptr, &::image);

		// Create memory to back up the image
		VkMemoryRequirements memoryRequirements;
		VkMemoryAllocateInfo memoryAllocationInfo{};
		memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		_DYN(vkGetImageMemoryRequirements)(::device, ::image, &memoryRequirements);
		memoryAllocationInfo.allocationSize = memoryRequirements.size;
		memoryAllocationInfo.memoryTypeIndex =
			getMemoryTypeIndex(
				::physicalDevice,
				memoryRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
			);
		_DYN(vkAllocateMemory)(::device, &memoryAllocationInfo, nullptr, &::memory);
		_DYN(vkBindImageMemory)(::device, ::image, ::memory, 0);

		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkSubresourceLayout subResourceLayout;
		_DYN(vkGetImageSubresourceLayout)(::device, ::image, &subResource, &subResourceLayout);
		::rowPitch = subResourceLayout.rowPitch;
		_DYN(vkMapMemory)(::device, ::memory, subResourceLayout.offset, VK_WHOLE_SIZE, 0, (void**)&::imageData);

	}
	return ret;
}

static void __stdcall _MINE(vkDestroySwapchainKHR)(
	VkDevice device_,
	VkSwapchainKHR swapchain_,
	const VkAllocationCallbacks* pAllocator)
{
	assert(::device == device_ && ::swapchain == swapchain_);
	printLog("video/vulkan: Swapchain # %x destroyed\n", swapchain_);

	::width = 0;
	::height = 0;
	::formatReady = false;
	::swapchainImages.clear();
	_DYN(vkDestroyImage)(::device, ::image, nullptr);
	_DYN(vkFreeMemory)(::device, ::memory, nullptr);
	::image = VK_NULL_HANDLE;
	::memory = VK_NULL_HANDLE;
	::imageData = nullptr;
	::rowPitch = 0;

	_REAL(vkDestroySwapchainKHR)(device_, swapchain_, pAllocator);
	::swapchain = VK_NULL_HANDLE;
}

void initVideo_Vulkan()
{
	HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
	if (vulkan)
	{

#define XXX_MACRO(f) WIN_INIT_PROC_ADDR(vulkan,##f)
		VK_FNPTRS;
#undef XXX_MACRO

		HOOK_DLL(vulkan, vkCreateDevice);
	}
}

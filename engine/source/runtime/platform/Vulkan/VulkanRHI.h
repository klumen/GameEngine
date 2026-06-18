#pragma once

#include "runtime/function/render/RHI.h"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include <optional>

extern "C"
{
	typedef struct GLFWwindow GLFWwindow;
}

namespace Lumen
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;
		std::optional<uint32_t> ComputeFamily;

		bool IsComplete() const
		{
			return GraphicsFamily.has_value() &&
				PresentFamily.has_value() &&
				ComputeFamily.has_value();
		}
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanRHI : public RHI
	{
	public:
		VulkanRHI(const RHIInfo& info);
		~VulkanRHI();

		void SetVSync(bool enabled) override;
		void SetDepthTest(bool enable) override;
		void SetBlend(bool enable) override;
		void SwapBuffers() override;

		void RenderMesh(Shared<RenderPipeline> pipeline, const glm::mat4& transform, AssetHandle mesh, const std::vector<AssetHandle>& materials) override;
		void RenderSprites(Shared<RenderPipeline> pipeline, const SpriteBatch& batch) override;

	public:
		VkInstance GetInstance() const { return m_Instance; }

	private:
		GLFWwindow* m_Window{ nullptr };

		 static constexpr uint8_t s_MaxFramesInFlight = 3;

		VkInstance m_Instance{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
		
		// physical device
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE }; // implicitly cleaned up by VkInstance
		VkPhysicalDeviceProperties m_Properties{ };
		VkPhysicalDeviceFeatures m_Features{ };
		VkPhysicalDeviceMemoryProperties m_MemoryProperties{ };
		QueueFamilyIndices m_QueueFamilyIndices{ };

		// logical device
		VkDevice m_Device{ VK_NULL_HANDLE };
		VkPhysicalDeviceFeatures m_EnabledFeatures{ };
		VkQueue m_GraphicsQueue{ VK_NULL_HANDLE }; // implicitly cleaned up by VkDevice
		VkQueue m_ComputeQueue{ VK_NULL_HANDLE };
		VkCommandPool m_GraphicsCommandPool{ VK_NULL_HANDLE };
		VkCommandPool m_ComputeCommandPool{ VK_NULL_HANDLE };

		// memory allocator
		VmaAllocator m_Allocator{ VK_NULL_HANDLE };
		uint64_t m_TotalAllocatedBytes = 0;

		// swapchain
		VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };
		VkQueue m_PresentQueue{ VK_NULL_HANDLE };
		VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
		std::vector<VkImage> m_SwapchainImages; // implicitly cleaned up by VkSwapchainKHR
		VkFormat m_SwapchainImageFormat{ VK_FORMAT_UNDEFINED };
		VkExtent2D m_SwapchainExtent{ 0, 0 };
		std::vector<VkImageView> m_SwapchainImageViews;

		std::vector<VkCommandBuffer> m_DrawCommandBuffers;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;

		VkSemaphore m_PresentComplete;
		VkSemaphore m_RenderComplete;
		std::vector<VkFence> m_WaitFences;
		VkSubmitInfo m_SubmitInfo;

		VkImage m_DepthImage;
		VmaAllocation m_DepthImageMemory;
		VkImageView m_DepthImageView;

		VkRenderPass m_RenderPass;
		uint32_t m_CurrentBufferIndex = 0;

		uint32_t m_MsaaSamples{ 0 };

	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateMemoryAllocator();
		void CreateSwapchain();

		void RecreateSwapchain();
		void DestroySwapchain();

	private:
		std::vector<const char*> GetRequiredExtensions() const;
		bool CheckValidationLayerSupport() const;
		VkDebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo() const;

		uint32_t RateDeviceSuitability(VkPhysicalDevice device) const;
		bool IsDeviceSuitable(VkPhysicalDevice device) const;
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
		SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const;
		VkSampleCountFlagBits GetMaxUsableSampleCount() const;

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		VkFormat FindDepthFormat() const;
	};
}
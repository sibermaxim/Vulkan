#pragma once

#include <windows.h>
#include <chrono>
#include <string>
#include <array>
#include "vulkan/vulkan.h"

#include "Camera.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanUIOverlay.h"
#include "VulkanTexture.h"

using namespace std;

#define WIN32_LEAN_AND_MEAN

#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"

class VulkanBase
{
	string shaderDir = "glsl";
	
	uint32_t destWidth;
	uint32_t destHeight;

	bool resizing = false;
	bool viewUpdated = false;
	
	string GetWindowTitle();
	void WindowResize();
	void HandleMouseMove(int32_t x, int32_t y);
	
	void NextFrame();

	void UpdateOverlay();
	
	void InitSwapchain();
	void CreateCommandPool();
	void SetupSwapChain();
	void CreateCommandBuffers();
	void СreateSynchronizationPrimitives();
	void CreatePipelineCache();
	void DestroyCommandBuffers();
	
public:

	bool prepared = false;
	uint32_t width = 1280;
	uint32_t height = 720;

	/** @brief Last frame time measured using a high performance timer (if available) */
	float frameTimer = 1.0f;

	string title = "Vulkan Example";
	string name = "vulkanExample";
	uint32_t apiVersion = VK_API_VERSION_1_2;

	vks::UIOverlay UIOverlay;
	
	Camera camera;
	vec2 mousePos;
	
	struct Settings
	{
		bool validation = false;
		bool fullscreen = false;
		/** @brief Set to true if v-sync will be forced for the swapchain */
		bool vsync = false;
		/** @brief Enable UI overlay */
		bool overlay = false;
	}settings;

	VkClearColorValue defaultClearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };

	struct {
		vec2 axisLeft = vec2(0.0f);
		vec2 axisRight = vec2(0.0f);
	} gamePadState;

	struct {
		bool left = false;
		bool right = false;
		bool middle = false;
	} mouseButtons;

	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;
	bool paused = false;
	
	struct {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} depthStencil;
	
	VulkanBase(bool enableValidation = false);
	virtual ~VulkanBase();

	/** @brief Encapsulated physical and logical vulkan device */
	vks::VulkanDevice* vulkanDevice;

	VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);
	
#if defined(_WIN32)

	HWND window;
	HINSTANCE windowInstance;

	static void SetupConsole(string title);
	void HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND SetupWindow(HINSTANCE hInstance, WNDPROC wndProc);

	//точка входа для основного цикла программы
	void RenderLoop();

#endif

	bool InitVulkan();
	virtual VkResult CreateInstance(bool validation);

	/** Prepare the next frame for workload submission by acquiring the next swap chain image */
	void PrepareFrame();
	/** @brief Presents the current image to the swap chain */
	void SubmitFrame();
	/** @brief (Virtual) Default image acquire + submission and command buffer submission function */
	virtual void RenderFrame();
	/** @brief (Pure virtual) Render function to be implemented by the sample application */
	virtual void Render() = 0;
	/** @brief (Virtual) Called when the camera view has changed */
	virtual void ViewChanged();
	virtual void MouseMoved(double x, double y, bool& handled);
	/** @brief Prepares all Vulkan resources and functions required to run the sample */
	virtual void prepare();
	/** @brief (Virtual) Setup default depth and stencil views */
	virtual void SetupDepthStencil();
	/** @brief (Virtual) Setup default framebuffers for all requested swapchain images */
	virtual void SetupFrameBuffer();
	/** @brief (Virtual) Setup a default renderpass */
	virtual void SetupRenderPass();
	/** @brief (Virtual) Called when the window has been resized, can be used by the sample application to recreate resources */
	virtual void WindowResized();
	/** @brief (Virtual) Called after the physical device features have been read, can be used to set features to enable on the device */
	virtual void GetEnabledFeatures();
	/** @brief (Virtual) Called when resources have been recreated that require a rebuild of the command buffers (e.g. frame buffer), to be implemented by the sample application */
	virtual void BuildCommandBuffers();

	/** @brief Adds the drawing commands for the ImGui overlay to the given command buffer */
	void DrawUI(const VkCommandBuffer commandBuffer);
	/** @brief (Virtual) Called when the UI overlay is updating, can be used to add custom elements to the overlay */
	virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay);
protected:

	//Подчет числа кадров для отображения
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
	chrono::time_point<chrono::high_resolution_clock> lastTimestamp;

	// Returns the path to the root of the glsl or hlsl shader directory.
	string GetShadersPath() const;
	
	//Экземпляр Vulkan
	VkInstance instance;
	// Physical device (GPU) that Vulkan will use
	VkPhysicalDevice physicalDevice;
	// Stores physical device properties (for e.g. checking device limits)
	VkPhysicalDeviceProperties deviceProperties;
	// Stores the features available on the selected physical device (for e.g. checking if a feature is available)
	VkPhysicalDeviceFeatures deviceFeatures;
	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	/** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
	VkPhysicalDeviceFeatures enabledFeatures{};
	/** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
	vector<const char*> enabledDeviceExtensions;
	vector<const char*> enabledInstanceExtensions;
	/** @brief Optional pNext structure for passing extension structures to device creation */
	void* deviceCreatepNextChain = nullptr;
	/** @brief Logical device, application's view of the physical device (GPU) */
	VkDevice device;
	// Handle to the device graphics queue that command buffers are submitted to
	VkQueue queue;
	// Depth buffer format (selected during Vulkan initialization)
	VkFormat depthFormat;
	// Command buffer pool
	VkCommandPool cmdPool;
	/** @brief Pipeline stages used to wait at for graphics queue submissions */
	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// Contains command buffers and semaphores to be presented to the queue
	VkSubmitInfo submitInfo;
	// Command buffers used for rendering
	std::vector<VkCommandBuffer> drawCmdBuffers;
	// Global render pass for frame buffer writes
	VkRenderPass renderPass;
	// List of available frame buffers (same as number of swap chain images)
	std::vector<VkFramebuffer>frameBuffers;
	// Active frame buffer index
	uint32_t currentBuffer = 0;
	// Descriptor set pool
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	// List of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> shaderModules;
	// Pipeline cache object
	VkPipelineCache pipelineCache;
	// Wraps the swap chain to present images (framebuffers) to the windowing system
	VulkanSwapChain swapChain;
	
	// Synchronization semaphores
	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
	} semaphores;
	std::vector<VkFence> waitFences;
	
};


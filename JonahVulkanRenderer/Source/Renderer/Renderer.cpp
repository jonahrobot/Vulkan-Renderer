#include "Renderer.h"
#include "Instance.h"


namespace renderer::detail {
	VkInstance CreateVulkanInstance(bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport);
};

namespace renderer {

	// Unnamed namespace to show functions below are pure Utility with no internal state. 
	// Cannot see or access private data of Renderer class.
	namespace{

		GLFWwindow* CreateGLFWWindow() {

			int GLFWErrorCode = glfwInit();
			if (GLFWErrorCode == GLFW_FALSE) {
				throw std::runtime_error("GLFW did not initialize correctly.");
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		}

		VkSwapchainKHR CreateSwapChain() {

		}

		VkPhysicalDevice CreatePhysicalDevice() {

		}
		VkDevice CreateLogicalDevice() {

		}
	}

	Renderer::Renderer() {

		window = CreateGLFWWindow();
		vulkan_instance = renderer::detail::CreateVulkanInstance(UseValidationLayers, ValidationLayersToSupport);

	}

	Renderer::~Renderer() {

	}

	void Renderer::Draw() {

	}

}// namespace renderer
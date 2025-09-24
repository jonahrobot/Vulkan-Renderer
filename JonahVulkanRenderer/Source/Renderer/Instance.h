#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vector>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace renderer {

class Instance {
public:
	Instance() = delete;
	~Instance() = delete;

	static VkInstance CreateInstance(const GLFWwindow* window, const std::vector<const char*>& ValidationLayers);

private:

	static bool CheckValidationLayerSupport(const std::vector<const char*>& ValidationLayers);

};

} // namespace renderer
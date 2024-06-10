#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#define GLFW_INCLUDE_VULKAN
#pragma clang diagnostic pop

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600

#define nullptr NULL

typedef struct {
    bool isGraphicsFamilySet;
    uint32_t graphicsFamily;

    bool isPresentFamilySet;
    uint32_t presentFamily;
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;

    uint32_t formatsCount;
    VkSurfaceFormatKHR *formats;

    uint32_t presentModesCount;
    VkPresentModeKHR *presentModes;
} SwapChainSupportDetails;

uint32_t rateDeviceSuitability(VkPhysicalDevice device);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

bool checkDeviceExtensionSupport(VkPhysicalDevice device, char **deviceExtensions, size_t deviceExtensionsCount);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, int availableFormatsCount);

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, int availablePresentModesCount);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window);

uint32_t *readFile(const char *filePath, size_t *len);

VkShaderModule createShaderModule(VkDevice device, const uint32_t *code, size_t codeSize);

__attribute__((unused)) void printAvailableExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    VkExtensionProperties *extensions = calloc(extensionCount, sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);

    printf("available extensions:\n");
    for (int i = 0; i < extensionCount; ++i) {
        printf("\t%3d. %s\n", i, extensions[i].extensionName);
    }
    fflush(stdout);
}

bool checkValidationSupport(const char **validationLayers, const size_t validationLayerCount) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    VkLayerProperties *availableLayers = calloc(layerCount, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    bool layerFound = false;
    for (int i = 0; i < validationLayerCount; ++i) {
        const char *layerName = validationLayers[i];
        for (int j = 0; j < layerCount; ++j) {
            if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            free(availableLayers);
            return false;
        }
    }

    free(availableLayers);
    return true;
}

char **getRequiredExtensions(uint32_t *extensionsCount) {
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(extensionsCount);
    (*extensionsCount) = (*extensionsCount) + 3;

    char **extensions = calloc(*extensionsCount, sizeof(char *));
    for (int i = 0; i < *extensionsCount - 3; ++i) {
        extensions[i] = strdup(glfwExtensions[i]);
    }
    extensions[*extensionsCount - 1] = strdup(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions[*extensionsCount - 2] = strdup(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions[*extensionsCount - 3] = strdup(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(__attribute__((unused)) VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              __attribute__((unused)) VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, __attribute__((unused)) void *pUserData) {

    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);
    fflush(stderr);

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                                         "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                                           "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, char **deviceExtensions,
                      const size_t deviceExtensionsCount) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions, deviceExtensionsCount);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = swapChainSupport.formatsCount > 0 && swapChainSupport.presentModesCount > 0;
    }

    return indices.isGraphicsFamilySet && indices.isPresentFamilySet && extensionsSupported && swapChainAdequate;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, char **deviceExtensions, const size_t deviceExtensionsCount) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    VkExtensionProperties *availableExtensions = (VkExtensionProperties *) malloc(
            extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions);
    for (size_t i = 0; i < deviceExtensionsCount; i++) {
        bool extensionFound = false;
        for (size_t j = 0; j < extensionCount; j++) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) {
            fprintf(stderr, "%s device extension not found", deviceExtensions[i]);
            free(availableExtensions);
            return false;
        }
    }
    free(availableExtensions);
    return true;
}

__attribute__((unused)) VkPhysicalDevice
pickPhysicalDevice(const VkPhysicalDevice *devices, const size_t devicesCount) {
    struct Candidate {
        uint32_t rating;
        VkPhysicalDevice device;
    };
    struct Candidate *candidates = calloc(devicesCount, sizeof(struct Candidate));

    for (size_t i = 0; i < devicesCount; ++i) {
        VkPhysicalDevice device = devices[i];
        candidates[i].rating = rateDeviceSuitability(device);
        candidates[i].device = device;
    }

    size_t maxRatingIndex = 0;
    uint32_t maxRating = 0;
    for (int i = 0; i < devicesCount; ++i) {
        if (candidates[i].rating > maxRating) {
            maxRatingIndex = i;
        }
    }
    printf("max rating: %d", maxRating);

    free(candidates);
    return devices[maxRatingIndex];
}

uint32_t rateDeviceSuitability(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    VkQueueFamilyProperties *queueFamilies = calloc(queueFamilyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    QueueFamilyIndices indices = {};
    for (int i = 0; i < queueFamilyCount; ++i) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.isPresentFamilySet = true;
            indices.presentFamily = i;
        }
        VkQueueFamilyProperties queueFamily = queueFamilies[i];
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.isGraphicsFamilySet = true;
            indices.graphicsFamily = i;
        }
        if (indices.isGraphicsFamilySet) {
            break;
        }
    }

    return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatsCount, nullptr);
    if (details.formatsCount > 0) {
        details.formats = calloc(details.formatsCount, sizeof(*details.formats));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &details.formatsCount, details.formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModesCount, nullptr);
    if (details.presentModesCount > 0) {
        details.presentModes = calloc(details.presentModesCount, sizeof(*details.presentModes));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &details.presentModesCount, details.presentModes);
    }

    return details;
}

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *availableFormats, const int availableFormatsCount) {
    for (int i = 0; i < availableFormatsCount; ++i) {
        const VkSurfaceFormatKHR availableFormat = availableFormats[i];
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, int availablePresentModesCount) {
    for (int i = 0; i < availablePresentModesCount; ++i) {
        const VkPresentModeKHR availablePresentMode = availablePresentModes[i];
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {.width = (uint32_t) width, .height = (uint32_t) height};
        if (actualExtent.width < capabilities->minImageExtent.width) {
            actualExtent.width = capabilities->minImageExtent.width;
        } else if (actualExtent.width > capabilities->maxImageExtent.width) {
            actualExtent.width = capabilities->maxImageExtent.width;
        }

        if (actualExtent.height < capabilities->minImageExtent.height) {
            actualExtent.height = capabilities->minImageExtent.height;
        } else if (actualExtent.height > capabilities->maxImageExtent.height) {
            actualExtent.height = capabilities->maxImageExtent.height;
        }

        return actualExtent;
    }
}

uint32_t *readFile(const char *filePath, size_t *len) {
    FILE *file = fopen(filePath, "rb");
    fseek(file, 0, SEEK_END);
    *len = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint32_t *buffer = calloc(*len, sizeof(buffer));
    fread(buffer, 1, *len, file);
    fclose(file);
    return buffer;
}

__attribute__((unused)) VkShaderModule
createShaderModule(VkDevice device, const uint32_t *code, const size_t codeSize) {
    if (code == nullptr || codeSize < 1) {
        perror("invalid code input!");
        exit(EXIT_FAILURE);
    }
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (const uint32_t *) code;
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        perror("failed to create shader module!");
        exit(EXIT_FAILURE);
    }
    return shaderModule;
}

int main(void) {
    // region globals
    const size_t validationLayersCount = 1;
    char **validationLayers = calloc(validationLayersCount, sizeof(char *));
    validationLayers[0] = strdup("VK_LAYER_KHRONOS_validation");

    const size_t deviceExtensionsCount = 2;
    char **deviceExtensions = calloc(deviceExtensionsCount, sizeof(*deviceExtensions));
    deviceExtensions[0] = strdup(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions[1] = strdup("VK_KHR_portability_subset"); // `VUID-VkDeviceCreateInfo-pProperties-04451` fix
    // endregion globals

    // region initWindow
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
    if (window == nullptr) {
        perror("failed to create window");
        exit(EXIT_FAILURE);
    }
    // endregion

    // region initVulkan
    // region createInstance
    if (!checkValidationSupport((const char **) validationLayers, validationLayersCount)) {
        perror("validation layers requested, but not available");
        exit(EXIT_FAILURE);
    }
    VkInstance instance;
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    populateDebugMessengerCreateInfo(&debugCreateInfo);

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
    uint32_t extensionCount = 0;
    char **extensions = (char **) getRequiredExtensions(&extensionCount);
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = (const char *const *) extensions;
    instanceCreateInfo.enabledLayerCount = validationLayersCount;
    instanceCreateInfo.ppEnabledLayerNames = (const char *const *) validationLayers;
    if (vkCreateInstance(&instanceCreateInfo, NULL, &instance) != VK_SUCCESS) {
        perror("failed to create instance");
        exit(EXIT_FAILURE);
    }
    // endregion createInstance
    // region setup debug logger
    VkDebugUtilsMessengerEXT debugMessenger;
    if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        perror("failed to set up debug messenger!");
        exit(EXIT_FAILURE);
    }
    // endregion
    // region window surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        perror("failed to create window surface!");
        exit(EXIT_FAILURE);
    }
    // endregion
    // region pickup physical device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        perror("failed to find GPUs with Vulkan support!");
        exit(EXIT_FAILURE);
    }
    VkPhysicalDevice *devices = calloc(deviceCount, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    for (int i = 0; i < deviceCount; ++i) {
        if (isDeviceSuitable(devices[i], surface, deviceExtensions, deviceExtensionsCount)) {
            physicalDevice = devices[i];
            break;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        perror("failed to find a suitable GPU");
        exit(EXIT_FAILURE);
    }
    // endregion
    // region logical devices and queues
    VkDevice device;
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.enabledLayerCount = validationLayersCount;
    createInfo.ppEnabledLayerNames = (const char *const *) validationLayers;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = deviceExtensionsCount;
    createInfo.ppEnabledExtensionNames = (const char *const *) deviceExtensions;
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        perror("failed to create logical device!");
        exit(EXIT_FAILURE);
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    // endregion
    // region swap chain
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats,
                                                               (int) swapChainSupport.formatsCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes,
                                                         (int) swapChainSupport.presentModesCount);
    VkExtent2D extent = chooseSwapExtent(&swapChainSupport.capabilities, window);
    uint32_t swapChainImageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        swapChainImageCount > swapChainSupport.capabilities.maxImageCount) {
        swapChainImageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = swapChainImageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
        perror("failed to create swap chain!");
        exit(EXIT_FAILURE);
    }

    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
    VkImage *swapChainImages = calloc(swapChainImageCount, sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages);

    __attribute__((unused)) VkFormat swapChainImageFormat = surfaceFormat.format;
    __attribute__((unused)) VkExtent2D swapChainExtent = extent;
    // endregion
    // region image views
    uint32_t swapChainImageViewsCount = swapChainImageCount;
    VkImageView *swapChainImageViews = calloc(swapChainImageViewsCount, sizeof(VkImageView));
    for (int i = 0; i < swapChainImageViewsCount; ++i) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapChainImageFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            perror("failed ot create image views!");
            exit(EXIT_FAILURE);
        }
    }
    // endregion
    // region render passes
    /**
 * <h2>Attachment description</h2>
 *
 * <p>Before creating the pipeline, we need to tell Vulkan about the
 * <i>framebuffer attachments</i> that will be used while rendering.</p>
 *
 * We need to specify:
 * <ul>
 * <li>how many color and depth buffers there will be
 * <li>how many samples to use for each of them
 * <li>how their contents should be handled throughout the rendering ops.
 * </ul>
 *
 * <p>All of the info is wrapped in a <i>render pass</i> object.<p>
 *
 * <p>Textures and frame-buffers in Vulkan are represented by
 * <code>VkImage</code> objects with a certain pixel format,
 * however the layout of the pixels in memory can change based on what
 * we're trying to do with the an image.
 *
 * <dl>
 * <dt><code>format</code></dt>
 * <dd>The <code>format</code> of the colour attachment should match the
 * format of the swap chain images.</dd>
 * <dt><code>samples</code></dt>
 * <dd>for <i>multisampling</i></dd>
 * <dt><code>loadOp</code></dt>
 * <dd>determines what to do with the data in the attachment before
 * rendering. <i>applies to color and depth data</i> Possible values:
 * <ul>
 * <li><code>VK_ATTACHMENT_LOAD_OP_LOAD</code>: Preserve the existing contents
 * of the attachment
 * <li><code>VK_ATTACHMENT_LOAD_OP_CLEAR</code>: Clear the values to a
 * constant at the start
 * <li><code>VK_ATTACHMENT_OP_DONT_CARE</code>: Existing contents are undefined;
 * we don't care about them
 * </ul>
 * </dd>
 * <dt><code>storeOp</code></dt>
 * <dd>determines what to do with the data in the attachment after rendering.
 * <i>applies to color and depth data</i>
 * Possible values:
 * <ul>
 * <li><code>VK_ATTACHMENT_STORE_OP_STORE</code>: Rendered contents will be
 * stored in memory and can be read later
 * <li><code>VK_ATTACHMENT_STORE_OP_DONT_CARE</code>: Contents of the
 * framebuffer will be undefined after the rendering operation
 * </ul>
 * </dd>
 * <dt><code>stencilLoadOp</code></dt>
 * <dd>Same as <code>loadOp</code>, but for <i>stencil data</i></dd>
 * <dt><code>stencilStoreOp</code></dt>
 * <dd>Same as <code>storeOp</code>, but for <i>stencil data</i></dd>
 * <dt><code>initialLayout</code></dt>
 * <dd>specifies which layout the image will have before the render pass.
 * Possible values:
 * <ul>
 * <li><code>VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL</code>: Images used as
 * colour attachment
 * <li><code>VK_IMAGE_LAYOUT_PRESENT_SRC_KHR</code>: Images to be presented
 * in the swap chain
 * <li><code>VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL</code>: Images to be used
 * as destination for a memory copy operation
 * </ul>
 * </dd>
 * <dt><code>finalLayout</code></dt>
 * <dd>specifies the layout to automatically transition to when the render
 * pass finishes. Possible values are the same as <code>initialLayout</code></dd>
 * </dl>
 */
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    /**
     * <h2>Subpasses and attachment references</h2>
     *
     * <p>A single <i>render pass</i> can consist of multiple <i>subpasses</i>.
     * Subpasses are subsequent rendering operations that depend on the contents
     * of framebuffers in previous passes.</p>
     *
     * <p>E.g., a sequence of post-processing effects that are applied one after
     * another. If you group these rendering operations into one render pass,
     * then Vulkan is able to reorder the operations and conserve memory
     * bandwidth for possibly better performance.</p>
     *
     * <p>
     * The <code>VkAttachmentReference</code> struct
     * <dl>
     * <dt><code>attachment</code></dt>
     * <dd>Specifies which attachment to reference by its index in the attachment
     * description array.</dd>
     * <dt><code>layout</code></dt>
     * <dd>Specifies which layout we would like the attachment to have during
     * a subpass that uses this reference.</dd>
     * </dl>
     * </p>
     *
     * <p>
     * The subpass is described using a <code>VkSubpassDescription</code> struct.
     * <dl>
     * <dt><code>pipelineBindPoint</code></dt>
     * <dd>Specifies this subpass to be a Graphics subpass</dd>
     * <dt><code>pColorAttachments</code></dt>
     * <dd>
     * <p>reference to the attachment referenced from the fragment shader.<p>
     * E.g., in our example, the index of the attachment in this array
     * is directly referenced from the fragment shader with
     * <code>layout(location = 0) out vec4 outColor</code></p>
     * </dd>
     * <dt><code>pInputAttachments</code></dt>
     * <dd>Attachments that are read from a shader</dd>
     * <dt><code>pResolveAttachments</code></dt>
     * <dd>Attachments used for multisampling color attachments</dd>
     * <dt><code>pDepthStencilAttachment</code></dt>
     * <dd>Attachment for depth and stencil data</dd>
     * <dt><code>pPreserveAttachments</code></dt>
     * <dd>Attachments that are not used by this subpass, but for which the data
     * must be preserved.</dd>
     * </dl>
     * </p>
     */
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    /**
     * <h2>Render pass</h2>
     *
     *
     */
    VkRenderPass renderPass;
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        perror("failed to create render pass!");
        exit(EXIT_FAILURE);
    }
    // endregion
    // region graphics pipeline
    size_t vertShaderCodeLen = 0;
    uint32_t *const vertShaderCode = readFile("/Users/samuel_paul_v/CLionProjects/vulkan_in_c/vert.spv",
                                              &vertShaderCodeLen);
    size_t fragShaderCodeLen = 0;
    uint32_t *const fragShaderCode = readFile("/Users/samuel_paul_v/CLionProjects/vulkan_in_c/frag.spv",
                                              &fragShaderCodeLen);
    VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode, vertShaderCodeLen);
    VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode, fragShaderCodeLen);

    VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
    vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageInfo.module = vertShaderModule;
    vertexShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // region fixed functions
    /**
     * <h2>Dynamic State</h2>
     *
     * most of the pipeline state needs to be baked into the pipeline state.
     * But a limited amount of the state can actually be changed without
     * recreating the pipeline at draw time.
     */
    const uint32_t dynamicStatesCount = 2;
    VkDynamicState *dynamicStates = calloc(2, sizeof(dynamicStates));
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStatesCount;
    dynamicState.pDynamicStates = dynamicStates;

    /**
     * <h2>Vertex input</h2>
     *
     * The <code>VkPipelineVertexInputStateCreateInfo</code> structure
     * describes the format of the vertex data that will be passes to the
     * vertex shader. Describes in two ways:
     *
     * <ol>
     * <li><strong>Binding</strong>: spacing between data and whether
     * the data is per-vertex or per-instance</li>
     * <li><strong>Attribute descriptions</strong>: type of the attributes
     * passed to the vertex shader, which binding to load them from and at
     * which offset.</li>
     * </ol>
     */
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    /**
     * <h2>Input Assembly</h2>
     *
     * The <code>VkPipelineInputAssemblyStateCreateInfo</code> struct describes
     * two things:
     * <ul>
     * <li>what kind of geometry will be drawn from the vertices
     * <ul>
     * <li><code>VK_PRIMITIVE_TOPOLOGY_POINT_LIST</code>: points from vertices
     * <li><code>VK_PRIMITIVE_TOPOLOGY_LINE_LIST</code>: line from every 2
     * vertices without reuse
     * <li><code>VK_PRIMITIVE_TOPOLOGY_LINE_STRIP</code>: the end vertex of
     * every line is used as start vertex for the next line
     * <li><code>VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST</code>: triangle from
     * every 3 vertices without reuse
     * <li><code>VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP</code>: the second and
     * the third vertex of every triangle are used as first two vertices of
     * the next triangle
     * </ul>
     * </li>
     * <li>if primitive restarts should be enabled.</li>
     * </ul>
     */
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    /**
     * <h2>Viewports and scissors</h2>
     *
     * <dl>
     * <dt>Viewport</dt>
     * <dd>Describes the region of the framebuffer that the output will be rendered
     * to.</dd>
     * <dt>Scissors</dt>
     * <dd>Defines the regions where pixels will be actually stored. Any pixels
     * outside the scissors rectangles will be discarded by the rasterizer.</dd>
     * </dl>
     */
    VkViewport viewport = {};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    /**
     * <h2>Raterizer</h2>
     *
     * <p>Rasterizer takes the geometry that is shaped by the vertices from the
     * vertex shader and turns it into fragments to be coloured by the fragment
     * shader.</p>
     *
     * <p>It also performs <strong>depth testing</strong>, <strong>face culling
     * </strong>
     * and the <strong>scissor test</strong>, and it can be configured to output
     * fragments that fill entire polygons or just the edges (<strong>wireframe
     * rendering</strong>).</p>
     *
     * <p>Defined by <code>VkPipelineRasterizationStateCreateInfo</code> struct:
     * <dl>
     * <dt><code>depthClampEnable</code></dt>
     * <dd>if <code>VK_TRUE</code>, fragments that are beyond the near and far
     * planes are clamped to them as opposed to discarding them. <i>This is
     * useful in special cases like shadow maps.</i></dd>
     * <dt><code>rasterizerDiscardEnable</dt>
     * <dd>If <code>VK_TRUE</code>, then geometry never passes through the
     * rasterizer state. <i>This basically disables any output to the
     * framebuffer</i></dd>
     * <dt><code>polygonMode</code></dt>
     * <dd>determines how fragments are generated for geometry. The following
     * modes are available:
     * <ul>
     * <li><code>VK_POLYGON_MODE_FILL</code>: fill the area of the polygon with
     * fragments
     * <li><code>VK_POLYGON_MODE_LINE</code>: polygon edges are drawn as lines
     * <li><code>VK_POLYGON_MODE_POINT</code>: polygon vertices are drawn as points
     * </ul></dd>
     * <dt><code>lineWidth</code></dt>
     * <dd>Describes the thickness of lines in terms of number of fragments.
     * The max line width that is supported depends on the hardware and any line
     * thicker than <code>1.0f</code> requires us to enable the
     * <code>wideLine</code> GPU feature.</dd>
     * <dt><code>cullMode</code></dt>
     * <dd>Determines the type of face culling to use. We can choose to:
     * <ul>
     * <li>disable culling
     * <li>cull the front faces
     * <li>cull the back faces
     * <li>cull both front and back faces
     * </ul></dd>
     * <dt><code>frontFace</code></dt>
     * <dd>Specifies the vertex order for faces to be considered front-facing
     * and can be <i>clockwise</i> or <i>counterclockwise</i>.</dd>
     * <dt><code>depthBias*</code></dt>
     * <dd>The rasterizer can alter the depth values by adding a constant value
     * or biasing them based on a fragment's slope. <i>This is sometimes used for
     * shadow mapping.</i></dd>
     * </dl>
     * </p>
    */
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    /**
     * <h2>Multisampling</h2>
     *
     * The <code>VkPipelineMultisampleStateCreateInfo</code> struct configures
     * multisampling, which is one of the ways to perform <code>anti-aliasing</code>.
     * It works by combining the fragment shader results of multiple polygons that
     * rasterize to the same pixel.
     */
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    /**
     * <h2>Color blending</h2>
     *
     * After a fragment shader has returned a colour, it needs to be combined
     * with the colour that is already in the framebuffer. This transformation
     * is known as colour blending and there are two ways to do it:
     *
     * <ul>
     * <li>Mix the old and new values to produce the final colour
     * <li>Combine the old and new value using a bitwise operation
     * </ul>
     *
     * Two types of structs to configure colour blending:
     * <ul>
     * <li><code>VkPipelineColorBlendAttachmentState</code> contains per
     * attached framebuffer
     * <li><code>VkPipelineColorBlendStateCreateInfo</code> contains the <i>global</i>
     * color blending settings.
     * </ul>
     */
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    /**
     * <h2>Pipeline layout</h2>
     *
     * <p>You can use <code>uniform</code> values in shaders, which are globals
     * similar
     * to dynamic state variables that can be changed at drawing time to alter the
     * behavior of your shaders without having to recreate them.
     * They are commonly used to pass the <i>transformation matrix</i> to the
     * vertex shader, or to create <i>texture samplers</i> in the fragment shader.
     * </p>
     *
     * Uniform values needs to be specified during pipeline creation by creating
     * a <code>VkPipelineLayout</code> object.
     */
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        perror("failed to create pipeline layout!");
        exit(EXIT_FAILURE);
    }
    // endregion
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragShaderStageInfo};
    pipelineInfo.stageCount = sizeof(shaderStages) / sizeof(VkPipelineShaderStageCreateInfo);
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipeline graphicsPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        perror("failed to create graphics pipeline!");
        exit(EXIT_FAILURE);
    }
    // endregion
    // region framebuffers
    uint32_t swapChainFramebuffersCount = swapChainImageCount;
    VkFramebuffer *swapChainFramebuffers = calloc(swapChainImageCount, sizeof(swapChainFramebuffers));
    for (uint32_t i = 0; i < swapChainFramebuffersCount; ++i) {
        VkImageView attachments[] = {swapChainImageViews[i]};
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            perror("failed to create framebuffer!");
            exit(EXIT_FAILURE);
        }
    }
    // endregion framebuffers
    // endregion initVulkan

    // region mainLoop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    // endregion mainLoop

    // region cleanup
    // region cleanup globals
    for (int i = 0; i < validationLayersCount; ++i) {
        free(validationLayers[i]);
        validationLayers[i] = nullptr;
    }
    free(validationLayers);
    validationLayers = nullptr;
    for (int i = 0; i < extensionCount; ++i) {
        free(extensions[i]);
        extensions[i] = nullptr;
    }
    free(extensions);
    extensions = nullptr;
    for (int i = 0; i < deviceExtensionsCount; ++i) {
        free(deviceExtensions[i]);
    }
    free(deviceExtensions);
    // endregion cleanup globals
    for (int i = 0; i < swapChainFramebuffersCount; ++i) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }
    free(swapChainFramebuffers);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    free(dynamicStates);
    free(vertShaderCode);
    free(fragShaderCode);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    for (int i = 0; i < swapChainImageCount; ++i) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }
    free(swapChainImageViews);
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    free(swapChainImages);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    window = NULL;
    glfwTerminate();
    // endregion cleanup
    return 0;
}

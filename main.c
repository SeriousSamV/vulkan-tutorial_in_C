#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600

#define nullptr NULL

void printAvailableExtensions() {
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

const char *const *getRequiredExtensions(uint32_t *glfwExtensionCount) {
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(glfwExtensionCount);
    (*glfwExtensionCount) = (*glfwExtensionCount) + 1;
    if (glfwExtensions != nullptr) {
        glfwExtensions = realloc(glfwExtensions, *glfwExtensionCount); // NOLINT(*-suspicious-realloc-usage)
    } else {
        glfwExtensions = calloc(*glfwExtensionCount, sizeof(char *));
    }
    glfwExtensions[*glfwExtensionCount - 1] = strdup(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return glfwExtensions;
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

int main(void) {
    // region globals
    const size_t validationLayersCount = 1;
    char **validationLayers = calloc(validationLayersCount, sizeof(char *));
    validationLayers[0] = strdup("VK_LAYER_KHRONOS_validation");
    // endregion globals

    // region initWindow
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
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
    const char *const *extensions = getRequiredExtensions(&extensionCount);
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensions;
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
    // endregion initVulkan

    printAvailableExtensions();

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
    // endregion cleanup globals
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    window = NULL;
    glfwTerminate();
    // endregion cleanup
    return 0;
}

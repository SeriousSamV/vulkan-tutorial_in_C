#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

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

int main(void) {
    // region initWindow
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
    // endregion

    // region initVulkan
    // region createInstance
    VkInstance instance;
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    glfwExtensionCount += 1;
    glfwExtensions = realloc(glfwExtensions, glfwExtensionCount * sizeof(char *)); // NOLINT(*-suspicious-realloc-usage)
    if (glfwExtensions == NULL) {
        perror("cannot extend glfwExtensions");
        exit(EXIT_FAILURE);
    }
    glfwExtensions[glfwExtensionCount - 1] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
    instanceCreateInfo.enabledLayerCount = 0;
    if (vkCreateInstance(&instanceCreateInfo, NULL, &instance) != VK_SUCCESS) {
        perror("failed to create instance");
        exit(EXIT_FAILURE);
    }
    // endregion createInstance
    // endregion initVulkan

    printAvailableExtensions();

    // region mainLoop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    // endregion mainLoop

    // region cleanup
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    window = NULL;
    glfwTerminate();
    // endregion cleanup
    return 0;
}

cmake_minimum_required(VERSION 3.28)
project(vulkan_in_c C)

set(CMAKE_C_STANDARD 23)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(${PROJECT_NAME} main.c)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

if (VULKAN_FOUND)
    message(STATUS "Found Vulkan. Including and linking...")
    target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan glfw)
endif(VULKAN_FOUND)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} glm::glm)

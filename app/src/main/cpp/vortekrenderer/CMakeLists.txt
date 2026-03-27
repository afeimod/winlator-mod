cmake_minimum_required(VERSION 3.22.1)

project(VortekRenderer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wno-unused-function -Wimplicit-function-declaration -DVK_USE_PLATFORM_ANDROID_KHR")

include_directories(include)
include_directories(../winlator/include)
include_directories(../libadrenotools/include)

add_library(vortekrenderer SHARED
            src/main.c
            src/vk_object.c
            src/request_handler.c
            src/vulkan_helper.c
            src/vk_context.c
            src/resource_memory.c
            src/xwindow_swapchain.c
            src/texture_decoder.c
            src/shader_inspector.c
            src/async_pipeline_creator.c
            src/timeline_semaphore.c)

target_link_libraries(vortekrenderer
                      adrenotools
                      winlator
                      log
                      android)
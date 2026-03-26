cmake_minimum_required(VERSION 3.22.1)

project(GladioRenderer)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wno-unused-function -Wimplicit-function-declaration -Werror=incompatible-pointer-types")

include_directories(include)
include_directories(../winlator/include)
include_directories(../vortekrenderer/include)

add_library(gladiorenderer SHARED
            src/main.c
            src/gl_context.c
            src/gl_renderer.c
            src/request_handler.c
            src/shader_material.c
            src/shader_converter.c
            src/gl_formats.c
            src/gl_texture.c
            src/attrib_stack.c
            src/gl_query.c
            src/compressed_texture.c
            src/arb_program.c
            src/gl_buffer.c
            src/gl_vao.c
            src/gl_framebuffer.c)

target_link_libraries(gladiorenderer
                      winlator
                      log
                      android
                      EGL
                      GLESv2
                      GLESv3)
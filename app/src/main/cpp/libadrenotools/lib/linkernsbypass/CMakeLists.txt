cmake_minimum_required(VERSION 3.14)

if(NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL arm64-v8a)
	message(FATAL_ERROR "Unsupported target architecture: ${CMAKE_ANDROID_ARCH_ABI}. Please make an issue on the repo!")
endif()

project(linkernsbypass LANGUAGES CXX)

set(SOURCES android_linker_ns.cpp
            android_linker_ns.h
            elf_soname_patcher.cpp
            elf_soname_patcher.h)

add_library(linkernsbypass STATIC ${SOURCES})


target_compile_options(linkernsbypass PRIVATE -Wall -Wextra)
target_link_libraries(linkernsbypass android dl)
target_include_directories(linkernsbypass PUBLIC .)

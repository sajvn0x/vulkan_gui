cmake_minimum_required(VERSION 3.25)
project(gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(library/vulkan-headers)
if(NOT TARGET Vulkan::Headers)
    add_library(Vulkan::Headers INTERFACE IMPORTED)
    set_target_properties(Vulkan::Headers PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/library/vulkan-headers/include"
    )
endif()

find_package(X11 REQUIRED COMPONENTS Xinerama Xrandr Xi)
include_directories(${X11_INCLUDE_DIR})

set(SOURCES
	src/main.cc
	src/core/window_management.cc
	src/platform/linux/window_management_x11.cc
)

add_compile_options(-g)
add_compile_options(-Wall)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Headers
    X11::X11
    X11::Xinerama
    X11::Xrandr
)


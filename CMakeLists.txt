cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME cdh-v2-software)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Setup libcsp
set(CSP_SYSTEM_NAME freertos CACHE STRING "" FORCE)
set(CSP_FREERTOS 1 CACHE INTERNAL "")
set(CSP_POSIX 0 CACHE INTERNAL "")  # disables endian.h
set(CSP_USE_RTABLE 1 CACHE INTERNAL "")

add_subdirectory(libcsp)

# workaround for libcsp to link freertos correctly
set_target_properties(csp PROPERTIES
    LINK_LIBRARIES ""
    INTERFACE_LINK_LIBRARIES ""
)

# workaround to provide endian.h for csp
target_include_directories(csp PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/CDH          # where endian.h is located.
)

# workaround to get csp to link to FreeRTOS
target_link_libraries(csp PUBLIC
    stm32cubemx
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    CDH/cdh_main.c
    CDH/cdh_can_driver.c
    CDH/cdh_csp_server.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    CDH
    libcsp/include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
    csp
)

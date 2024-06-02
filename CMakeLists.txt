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
set(CMAKE_PROJECT_NAME stm32-ntru)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    libntru-stm32/pqNTRUSign/Gaussian-1024/ntru_api_gauss.c
    libntru-stm32/pqNTRUSign/common/crypto_hash_sha512.c
    libntru-stm32/pqNTRUSign/common/crypto_stream_salsa20.c
    libntru-stm32/pqNTRUSign/common/DGS.c
    libntru-stm32/pqNTRUSign/common/fastrandombytes.c
    libntru-stm32/pqNTRUSign/common/misc.c
    libntru-stm32/pqNTRUSign/common/NTT.c
    libntru-stm32/pqNTRUSign/common/packing.c
    libntru-stm32/pqNTRUSign/common/param.c
    libntru-stm32/pqNTRUSign/common/poly.c
    libntru-stm32/pqNTRUSign/common/pqNTRUSign.c
    libntru-stm32/pqNTRUSign/common/rng.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    libntru-stm32/pqNTRUSign/Gaussian-1024
    libntru-stm32/pqNTRUSign/common
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

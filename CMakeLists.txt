# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# Set the project name
project(AnimationEditor)

# Ensure that we are not building in our source directories
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Building in the source directory is not allowed.")
endif()

if(CMAKE_BINARY_DIR MATCHES "${CMAKE_SOURCE_DIR}/build.*")
  # Build directory is valid
else()
  message(FATAL_ERROR "You must run cmake from a directory that is not in your source tree, or that is in a special subdirectory of the tree whose name begins with 'build'.")
endif()

# Set the default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Add the cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Strip binary in release mode
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
endif()

# Set the output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Link and include directories
link_directories()
include_directories(
  ${CMAKE_SOURCE_DIR}/animationeditor
)

# Add the subdirectories
add_subdirectory(animationeditor)
add_subdirectory(sample)

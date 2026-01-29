cmake_minimum_required(VERSION 3.14)
project(EasyMRC VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Thread support
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# EasyMRC library (header-only)
add_library(easymrc INTERFACE)
target_include_directories(easymrc INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(easymrc INTERFACE cxx_std_17)
target_link_libraries(easymrc INTERFACE Threads::Threads)

# Main application
add_executable(easymrc_main src/main.cpp)
target_link_libraries(easymrc_main PRIVATE easymrc)

# Test executable
add_executable(easymrc_test test/easymrc_test.cpp)
target_link_libraries(easymrc_test PRIVATE easymrc)

# Test data generator
add_executable(generate_test_data test/generate_test_data.cpp)

# GDSII test
add_executable(test_gdsii test/test_gdsii.cpp)
target_link_libraries(test_gdsii PRIVATE easymrc)

# Installation
install(TARGETS easymrc_main DESTINATION bin)
install(DIRECTORY src/easymrc DESTINATION include)

# Print configuration
message(STATUS "EasyMRC Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

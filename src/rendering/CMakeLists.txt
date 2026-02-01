cmake_minimum_required(VERSION 3.16)

project(${RENDERING_TARGET})

# Set RHI source directories
set(RENDERING_SOURCE_DIRS 
    "rhi"
    "backend"
)

# Collect source files from all directories
file(GLOB_RECURSE RENDERING_SOURCES
    ./*.cpp
    ${RENDERING_SOURCE_DIRS}/*.cpp
)

add_library(${RENDERING_TARGET} STATIC ${RENDERING_SOURCES})

target_link_libraries(${RENDERING_TARGET} PRIVATE ${RENDERING_TARGET_DEPENDENCIES})

# Add RHI-specific include directories
target_include_directories(${RENDERING_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..  # Project root for inc/ directory
)

message(STATUS "Rendering Library Configuration Complete")
message(STATUS "  Target: ${RENDERING_TARGET}")
message(STATUS "  Source Files: ${RENDERING_SOURCES}")
message(STATUS "  Dependencies: ${RENDERING_TARGET_DEPENDENCIES}")
message(STATUS "  Source Directories: ${RENDERING_SOURCE_DIRS}")
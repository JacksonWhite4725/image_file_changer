cmake_minimum_required(VERSION 3.10)
project(media_processor C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Mac-specific settings
if(APPLE)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()

# Find required packages
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

# For Mac, we'll link libraries directly
find_library(HEIF_LIBRARY heif REQUIRED)
find_library(WEBP_LIBRARY webp REQUIRED)
find_library(AVIF_LIBRARY avif REQUIRED)

# Add definitions
add_definitions(${PNG_DEFINITIONS})

# Source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/converter.c
)

# Main executable
add_executable(media_processor ${SOURCES})

target_include_directories(media_processor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PNG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    /usr/local/include
)

target_link_libraries(media_processor PRIVATE
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${HEIF_LIBRARY}
    ${WEBP_LIBRARY}
    ${AVIF_LIBRARY}
)

# Set warning flags
if(MSVC)
    target_compile_options(media_processor PRIVATE /W4)
else()
    target_compile_options(media_processor PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -Wno-strict-prototypes  # Specifically target the libheif warning
    )
endif()
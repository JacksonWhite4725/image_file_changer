cmake_minimum_required(VERSION 3.10)
project(media_processor C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Mac-specific settings
if(APPLE)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
    
    # Add complete set of GTK3 and related includes for Mac
    include_directories(/usr/local/include/gtk-3.0)
    include_directories(/usr/local/include/glib-2.0)
    include_directories(/usr/local/lib/glib-2.0/include)
    include_directories(/usr/local/include/pango-1.0)
    include_directories(/usr/local/include/harfbuzz)
    include_directories(/usr/local/include/cairo)
    include_directories(/usr/local/include/gdk-pixbuf-2.0)
    include_directories(/usr/local/include/atk-1.0)
endif()

# Find required packages
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

# For Mac, we'll link libraries directly
find_library(HEIF_LIBRARY heif REQUIRED)
find_library(WEBP_LIBRARY webp REQUIRED)
find_library(AVIF_LIBRARY avif REQUIRED)

# Handle GTK3 and related libraries
if(APPLE)
    # Find all required libraries for GUI
    find_library(GTK3_LIBRARY gtk-3 REQUIRED)
    find_library(GDK3_LIBRARY gdk-3 REQUIRED)
    find_library(PANGOCAIRO_LIBRARY pangocairo-1.0 REQUIRED)
    find_library(PANGO_LIBRARY pango-1.0 REQUIRED)
    find_library(HARFBUZZ_LIBRARY harfbuzz REQUIRED)
    find_library(CAIRO_LIBRARY cairo REQUIRED)
    find_library(GLIB_LIBRARY glib-2.0 REQUIRED)
    find_library(GOBJECT_LIBRARY gobject-2.0 REQUIRED)
    find_library(GDK_PIXBUF_LIBRARY gdk_pixbuf-2.0 REQUIRED)
    find_library(INTL_LIBRARY intl REQUIRED)
    find_library(ATK_LIBRARY atk-1.0 REQUIRED)
    
    # Combine all GUI libraries
    set(GTK3_LIBRARIES
        ${GTK3_LIBRARY}
        ${GDK3_LIBRARY}
        ${PANGOCAIRO_LIBRARY}
        ${PANGO_LIBRARY}
        ${HARFBUZZ_LIBRARY}
        ${CAIRO_LIBRARY}
        ${GLIB_LIBRARY}
        ${GOBJECT_LIBRARY}
        ${GDK_PIXBUF_LIBRARY}
        ${INTL_LIBRARY}
        ${ATK_LIBRARY}
    )
else()
    # For other platforms, use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
endif()

# Add definitions
add_definitions(${PNG_DEFINITIONS})

# Source files
set(CLI_SOURCES
    src/main.c
    src/converter.c
    src/batch_processor.c
)

set(GUI_SOURCES
    src/main_gui.c
    src/gui.c
    src/converter.c
    src/batch_processor.c
)

# CLI executable
add_executable(media_processor ${CLI_SOURCES})

# GUI executable
add_executable(media_processor_gui ${GUI_SOURCES})

# Include directories for CLI
target_include_directories(media_processor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PNG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
)

# Include directories for GUI
target_include_directories(media_processor_gui PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PNG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
)

# Link libraries for CLI
target_link_libraries(media_processor PRIVATE
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${HEIF_LIBRARY}
    ${WEBP_LIBRARY}
    ${AVIF_LIBRARY}
)

# Link libraries for GUI
target_link_libraries(media_processor_gui PRIVATE
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${HEIF_LIBRARY}
    ${WEBP_LIBRARY}
    ${AVIF_LIBRARY}
    ${GTK3_LIBRARIES}
)

# Set warning flags
if(MSVC)
    target_compile_options(media_processor PRIVATE /W4)
    target_compile_options(media_processor_gui PRIVATE /W4)
else()
    target_compile_options(media_processor PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -Wno-strict-prototypes
    )
    target_compile_options(media_processor_gui PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -Wno-strict-prototypes
    )
endif()

# Install targets
install(TARGETS media_processor media_processor_gui
    RUNTIME DESTINATION bin
)
# Media Processor

A lightning-fast image format converter supporting HEIC, JPG, PNG, WEBP, and AVIF formats.

## Prerequisites

You'll need to install these dependencies:

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libheif-dev libjpeg-dev libpng-dev libwebp-dev libavif-dev
sudo apt install libgtk-3-dev
```

### macOS:
```bash
brew install cmake
brew install libheif libjpeg libpng webp libavif
brew install gtk+3
```

### Windows:
1. Install Visual Studio Community Edition with C++ support
2. Install vcpkg (package manager)
3. Run these commands:
```bash
vcpkg install libheif:x64-windows
vcpkg install libjpeg-turbo:x64-windows
vcpkg install libpng:x64-windows
vcpkg install libwebp:x64-windows
vcpkg install libavif:x64-windows
vcpkg install gtk:x64-windows
```

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .
```

## Usage

### CLI Version:
```bash
./media_processor_cli input.jpg --output png --quality 90
```

### GUI Version:
```bash
./media_processor_gui
```
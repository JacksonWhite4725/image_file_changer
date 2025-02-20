# 🖼️ Media Processor

A lightning-fast, versatile image format converter with both GUI and CLI interfaces. Convert between HEIC, JPG, PNG, WEBP, and AVIF with ease and precision.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## ✨ Features

- **Dual Interface**: Choose between GUI and CLI for your workflow
- **Multi-Format Support**: Convert between HEIC, JPG, PNG, WEBP, and AVIF formats
- **Batch Processing**: Convert entire directories of images at once
- **High Performance**: Multi-threaded processing for maximum speed
- **Quality Control**: Fine-tune compression and quality settings
- **Format Detection**: Automatic input format detection
- **Live Preview**: Preview images before conversion in GUI mode
- **Drag & Drop**: Easy file handling in GUI mode
- **Progress Tracking**: Real-time conversion progress

## 🚀 Quick Start

### Prerequisites

Choose your platform and install the required dependencies:

<details>
<summary>📦 Ubuntu/Debian</summary>

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libheif-dev libjpeg-dev libpng-dev libwebp-dev libavif-dev
sudo apt install libgtk-3-dev
```
</details>

<details>
<summary>🍎 macOS</summary>

```bash
brew install cmake pkg-config
brew install libheif libjpeg libpng webp libavif
brew install gtk+3 glib pango cairo gdk-pixbuf atk harfbuzz
```
</details>

<details>
<summary>🪟 Windows</summary>

1. Install Visual Studio Community Edition with C++ support
2. Install vcpkg (package manager)
3. Install GTK3 development files
4. Run:
```bash
vcpkg install libheif:x64-windows
vcpkg install libjpeg-turbo:x64-windows
vcpkg install libpng:x64-windows
vcpkg install libwebp:x64-windows
vcpkg install libavif:x64-windows
vcpkg install gtk3:x64-windows
```
</details>

### Building

```bash
# Clone the repository
git clone https://github.com/yourusername/media_processor.git
cd media_processor

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .
```

## 💻 Usage

### GUI Mode

Launch the GUI application:
```bash
./media_processor_gui
```

Features:
- **Single File Mode**: 
  - Select individual files
  - Preview before conversion
  - Real-time format switching
  - Quality adjustment slider

- **Batch Processing Mode**:
  - Select entire directories
  - Multi-threaded processing
  - Progress tracking
  - Success notifications

### CLI Mode

#### Single File Conversion
```bash
# Basic conversion
./media_processor input.heic output.jpg

# Set quality level (0-100)
./media_processor input.png output.webp -q 95
```

#### Batch Processing
```bash
# Convert all images in a directory to WEBP
./media_processor -b /path/to/directory webp

# Convert with specific quality
./media_processor -b -q 95 /path/to/directory png

# Convert and replace original files
./media_processor -b -r /path/to/directory jpg
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-b, --batch` | Enable batch processing mode |
| `-r, --replace` | Replace original files (batch mode) |
| `-q, --quality <0-100>` | Set output quality |
| `-h, --help` | Show help message |

## 🎯 Supported Formats

| Format | Read | Write | Quality Control | Preview |
|--------|------|-------|----------------|---------|
| HEIC | ✅ | ✅ | ✅ | ✅ |
| JPG | ✅ | ✅ | ✅ | ✅ |
| PNG | ✅ | ✅ | ✅ | ✅ |
| WEBP | ✅ | ✅ | ✅ | ✅ |
| AVIF | ✅ | ✅ | ✅ | ✅ |

## ⚡ Performance Tips

- Use the GUI's batch processing for multiple files
- Enable multi-threading for maximum performance
- Use WEBP for web-optimized images
- Use HEIC/AVIF for maximum compression
- Use PNG for lossless quality
- Quality settings of 85-95 offer the best quality/size balance

## 🛟 Troubleshooting

### Common Issues

1. **"Unsupported format" error**
   - Verify the file extension matches the actual format
   - Ensure the file isn't corrupted

2. **Build failures**
   - Ensure all dependencies (including GTK3) are installed
   - Check CMake version (3.10+ required)
   - Verify compiler supports C11

3. **GUI not starting**
   - Verify GTK3 installation
   - Check system theme compatibility
   - Ensure display server is running

4. **Slow batch processing**
   - Close other CPU-intensive applications
   - Ensure sufficient disk space
   - Check system memory usage

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with GTK3 for the graphical interface
- Powered by various open-source image processing libraries
- Special thanks to the open-source community

---

Made with ❤️ by Jackson @ Launch Turtle
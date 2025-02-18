# 🖼️ Media Processor

A lightning-fast, versatile image format converter supporting modern image formats. Convert between HEIC, JPG, PNG, WEBP, and AVIF with ease and precision.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## ✨ Features

- **Multi-Format Support**: Convert between HEIC, JPG, PNG, WEBP, and AVIF formats
- **Batch Processing**: Convert entire directories of images at once
- **High Performance**: Optimized for speed and quality
- **Quality Control**: Fine-tune compression and quality settings
- **Format Detection**: Automatic input format detection
- **Error Handling**: Robust error handling and reporting

## 🚀 Quick Start

### Prerequisites

Choose your platform and install the required dependencies:

<details>
<summary>📦 Ubuntu/Debian</summary>

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libheif-dev libjpeg-dev libpng-dev libwebp-dev libavif-dev
```
</details>

<details>
<summary>🍎 macOS</summary>

```bash
brew install cmake
brew install libheif libjpeg libpng webp libavif
```
</details>

<details>
<summary>🪟 Windows</summary>

1. Install Visual Studio Community Edition with C++ support
2. Install vcpkg (package manager)
3. Run:
```bash
vcpkg install libheif:x64-windows
vcpkg install libjpeg-turbo:x64-windows
vcpkg install libpng:x64-windows
vcpkg install libwebp:x64-windows
vcpkg install libavif:x64-windows
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

### Single File Conversion
Convert individual files with optional quality settings:

```bash
# Basic conversion
./media_processor input.heic output.jpg

# Set quality level (0-100)
./media_processor input.png output.webp -q 95
```

### Batch Processing
Convert all supported images in a directory:

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

| Format | Read | Write | Quality Control |
|--------|------|-------|----------------|
| HEIC | ✅ | ✅ | ✅ |
| JPG | ✅ | ✅ | ✅ |
| PNG | ✅ | ✅ | ✅ |
| WEBP | ✅ | ✅ | ✅ |
| AVIF | ✅ | ✅ | ✅ |

## ⚡ Performance Tips

- Use WEBP for web-optimized images
- Use HEIC/AVIF for maximum compression
- Use PNG for lossless quality
- Batch processing is faster than converting files individually
- Quality settings of 85-95 offer the best quality/size balance

## 🛟 Troubleshooting

### Common Issues

1. **"Unsupported format" error**
   - Verify the file extension matches the actual format
   - Ensure the file isn't corrupted

2. **Build failures**
   - Ensure all dependencies are installed
   - Check CMake version (3.10+ required)
   - Verify compiler supports C11

3. **Permission errors in batch mode**
   - Check directory permissions
   - Run with appropriate privileges

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with support from various open-source image processing libraries
- Special thanks to the open-source community

---

Made with ❤️ by Jackson @ Launch Turtle
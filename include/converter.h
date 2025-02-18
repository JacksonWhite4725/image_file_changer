#ifndef MEDIA_PROCESSOR_CONVERTER_H
#define MEDIA_PROCESSOR_CONVERTER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef enum {
    FORMAT_UNKNOWN,
    FORMAT_HEIC,
    FORMAT_JPG,
    FORMAT_PNG,
    FORMAT_WEBP,
    FORMAT_AVIF
} ImageFormat;

typedef struct {
    unsigned char* data;
    size_t width;
    size_t height;
    size_t channels;
    size_t size;
} ImageData;

typedef struct {
    int quality;        // 0-100
    bool maintain_exif; // whether to preserve EXIF data
    struct {
        bool progressive;  // For JPEG progressive encoding
        int optimization;  // For JPEG optimization level
    } jpeg_options;
    struct {
        bool lossless;    // For WebP lossless mode
        bool exact;       // For WebP exact preservation of RGB values
    } webp_options;
    struct {
        int speed;        // For AVIF encoding speed (0-10)
        bool lossless;    // For AVIF lossless mode
    } avif_options;
} ConversionOptions;

// Format-specific loading functions
ImageData* load_png(const char* filepath);
ImageData* load_jpeg(const char* filepath);
ImageData* load_webp(const char* filepath);
ImageData* load_avif(const char* filepath);
ImageData* load_heic(const char* filepath);

// Format-specific saving functions
bool save_png(const char* filepath, const ImageData* img, const ConversionOptions* options);
bool save_jpeg(const char* filepath, const ImageData* img, const ConversionOptions* options);
bool save_webp(const char* filepath, const ImageData* img, const ConversionOptions* options);
bool save_avif(const char* filepath, const ImageData* img, const ConversionOptions* options);
bool save_heic(const char* filepath, const ImageData* img, const ConversionOptions* options);

// Core functions
bool convert_image(const char* input_path, 
                  const char* output_path,
                  ImageFormat target_format,
                  const ConversionOptions* options);

ImageFormat detect_format(const char* filepath);

// Utility functions
const char* format_to_string(ImageFormat format);
ImageFormat string_to_format(const char* str);
void init_conversion_options(ConversionOptions* options);  // New utility function

// Memory management
void free_image_data(ImageData* img);

#endif // MEDIA_PROCESSOR_CONVERTER_H

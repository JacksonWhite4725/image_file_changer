#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "converter.h"

int main(int argc, char *argv[]) {
    printf("Media Processor Initializing...\n");
    
    if (argc < 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Detect input format
    ImageFormat input_format = detect_format(argv[1]);
    printf("Detected input format: %s\n", format_to_string(input_format));

    // Load image based on format
    ImageData* img = NULL;
    switch (input_format) {
        case FORMAT_PNG:
            printf("Loading PNG file...\n");
            img = load_png(argv[1]);
            break;
        case FORMAT_WEBP:
            printf("Loading WebP file...\n");
            img = load_webp(argv[1]);
            break;
        case FORMAT_JPG:
            printf("Loading JPEG file...\n");
            img = load_jpeg(argv[1]);
            break;
        case FORMAT_AVIF:
            printf("Loading AVIF file...\n");
            img = load_avif(argv[1]);
            break;
        case FORMAT_HEIC:
            printf("Loading HEIC file...\n");
            img = load_heic(argv[1]);
            break;
        default:
            printf("Unsupported input format\n");
            break;
    }

    if (img) {
        printf("Successfully loaded image: %zux%zu with %zu channels\n", 
               img->width, img->height, img->channels);
        
        // Create conversion options
        ConversionOptions options = {
            .quality = 90,
            .maintain_exif = true,
            .webp_options = {
                .lossless = false,
                .exact = false
            },
            .avif_options = {
                .speed = 6,      // Medium speed
                .lossless = false
            }
        };

        // Detect output format
        ImageFormat output_format = detect_format(argv[2]);
        printf("Detected output format: %s\n", format_to_string(output_format));
        
        // Save the image based on format
        bool save_success = false;
        switch (output_format) {
            case FORMAT_PNG:
                printf("Saving as PNG...\n");
                save_success = save_png(argv[2], img, &options);
                break;
            case FORMAT_WEBP:
                printf("Saving as WebP...\n");
                save_success = save_webp(argv[2], img, &options);
                break;
            case FORMAT_JPG:
                printf("Saving as JPEG...\n");
                save_success = save_jpeg(argv[2], img, &options);
                break;
            case FORMAT_AVIF:
                printf("Saving as AVIF...\n");
                save_success = save_avif(argv[2], img, &options);
                break;
            case FORMAT_HEIC:
                printf("Saving as HEIC...\n");
                save_success = save_heic(argv[2], img, &options);
                break;
            default:
                printf("Unsupported output format\n");
                break;
        }

        if (save_success) {
            printf("Successfully saved file to: %s\n", argv[2]);
        } else {
            printf("Failed to save file\n");
        }

        // Cleanup
        free_image_data(img);
        free(img);
    } else {
        printf("Failed to load input file\n");
    }
    
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "converter.h"
#include "batch_processor.h"

void print_usage(const char* program_name) {
    printf("Usage:\n");
    printf("Single file: %s <input_file> <output_file>\n", program_name);
    printf("Batch processing: %s -b <input_directory> <target_format>\n", program_name);
    printf("\nSupported formats: PNG, JPG, WEBP, AVIF, HEIC\n");
    printf("Options:\n");
    printf("  -b, --batch       Enable batch processing mode\n");
    printf("  -r, --replace     Replace original files (batch mode only)\n");
    printf("  -q, --quality     Set quality (0-100, default: 90)\n");
    printf("  -h, --help        Show this help message\n");
}

int main(int argc, char *argv[]) {
    printf("Media Processor Initializing...\n");

    if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 1;
    }

    // Check if we're in batch mode
    bool batch_mode = false;
    bool replace_originals = false;
    int quality = 90;
    
    // Parse command line options
    int arg_index = 1;
    while (arg_index < argc && argv[arg_index][0] == '-') {
        if (strcmp(argv[arg_index], "-b") == 0 || 
            strcmp(argv[arg_index], "--batch") == 0) {
            batch_mode = true;
        } else if (strcmp(argv[arg_index], "-r") == 0 || 
                   strcmp(argv[arg_index], "--replace") == 0) {
            replace_originals = true;
        } else if (strcmp(argv[arg_index], "-q") == 0 || 
                   strcmp(argv[arg_index], "--quality") == 0) {
            if (arg_index + 1 < argc) {
                quality = atoi(argv[arg_index + 1]);
                if (quality < 0) quality = 0;
                if (quality > 100) quality = 100;
                arg_index++;
            }
        }
        arg_index++;
    }

    // Create conversion options
    ConversionOptions options = {
        .quality = quality,
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

    if (batch_mode) {
        // Check remaining arguments for batch mode
        if (argc - arg_index < 2) {
            printf("Error: Batch mode requires input directory and target format\n");
            print_usage(argv[0]);
            return 1;
        }

        const char* input_dir = argv[arg_index];
        const char* target_format_str = argv[arg_index + 1];
        
        // Convert target format string to enum
        ImageFormat target_format = string_to_format(target_format_str);
        if (target_format == FORMAT_UNKNOWN) {
            printf("Error: Unsupported target format: %s\n", target_format_str);
            return 1;
        }

        // Set up batch processing options
        BatchProcessingOptions batch_options = {
            .input_dir = (char*)input_dir,
            .target_format = target_format,
            .options = options,
            .replace_originals = replace_originals,
            .extension_filter = NULL  // Process all supported images
        };

        // Process directory
        printf("Starting batch processing in directory: %s\n", input_dir);
        printf("Target format: %s\n", format_to_string(target_format));
        printf("Replace originals: %s\n", replace_originals ? "Yes" : "No");
        
        int result = process_directory(&batch_options);
        if (result < 0) {
            printf("Batch processing failed\n");
            return 1;
        }
    } else {
        // Original single-file conversion logic
        if (argc - arg_index < 2) {
            printf("Error: Single file mode requires input and output files\n");
            print_usage(argv[0]);
            return 1;
        }

        const char* input_file = argv[arg_index];
        const char* output_file = argv[arg_index + 1];

        // Original single-file conversion code here...
        ImageFormat input_format = detect_format(input_file);
        printf("Detected input format: %s\n", format_to_string(input_format));

        ImageData* img = NULL;
        switch (input_format) {
            case FORMAT_PNG:
                img = load_png(input_file);
                break;
            case FORMAT_WEBP:
                img = load_webp(input_file);
                break;
            case FORMAT_JPG:
                img = load_jpeg(input_file);
                break;
            case FORMAT_AVIF:
                img = load_avif(input_file);
                break;
            case FORMAT_HEIC:
                img = load_heic(input_file);
                break;
            default:
                printf("Unsupported input format\n");
                return 1;
        }

        if (img) {
            ImageFormat output_format = detect_format(output_file);
            bool save_success = false;
            
            switch (output_format) {
                case FORMAT_PNG:
                    save_success = save_png(output_file, img, &options);
                    break;
                case FORMAT_WEBP:
                    save_success = save_webp(output_file, img, &options);
                    break;
                case FORMAT_JPG:
                    save_success = save_jpeg(output_file, img, &options);
                    break;
                case FORMAT_AVIF:
                    save_success = save_avif(output_file, img, &options);
                    break;
                case FORMAT_HEIC:
                    save_success = save_heic(output_file, img, &options);
                    break;
                default:
                    printf("Unsupported output format\n");
                    break;
            }

            if (save_success) {
                printf("Successfully converted file to: %s\n", output_file);
            } else {
                printf("Failed to save file\n");
            }

            // Cleanup
            free_image_data(img);
            free(img);
        } else {
            printf("Failed to load input file\n");
            return 1;
        }
    }
    
    return 0;
}

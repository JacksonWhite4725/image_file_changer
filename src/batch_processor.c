#include "batch_processor.h"
#include <limits.h>  // For PATH_MAX
#include <unistd.h>  // For getcwd

#ifndef PATH_MAX
#define PATH_MAX 4096  // Common value for most UNIX systems
#endif

bool is_supported_image(const char* filename) {
    ImageFormat format = detect_format(filename);
    return format != FORMAT_UNKNOWN;
}

char* get_temp_filename(const char* original_filename) {
    const char* temp_suffix = ".tmp";
    size_t len = strlen(original_filename) + strlen(temp_suffix) + 1;
    char* temp_filename = (char*)malloc(len);
    if (!temp_filename) return NULL;
    
    snprintf(temp_filename, len, "%s%s", original_filename, temp_suffix);
    return temp_filename;
}

char* get_output_filename(const char* input_filename, ImageFormat target_format) {
    // Find the last dot in the filename
    const char* last_dot = strrchr(input_filename, '.');
    if (!last_dot) return NULL;

    // Calculate the base length (without extension)
    size_t base_len = last_dot - input_filename;
    
    // Get the new extension
    const char* new_ext = format_to_string(target_format);
    if (!new_ext) return NULL;

    // Allocate memory for new filename
    size_t new_len = base_len + strlen(new_ext) + 2; // +2 for dot and null terminator
    char* output_filename = (char*)malloc(new_len);
    if (!output_filename) return NULL;

    // Construct new filename
    strncpy(output_filename, input_filename, base_len);
    snprintf(output_filename + base_len, new_len - base_len, ".%s", new_ext);
    
    return output_filename;
}

int process_directory(const BatchProcessingOptions* options) {
    if (!options || !options->input_dir) {
        printf("Error: Invalid batch processing options\n");
        return -1;
    }

    DIR* dir = opendir(options->input_dir);
    if (!dir) {
        printf("Error: Could not open directory %s: %s\n", 
               options->input_dir, strerror(errno));
        return -1;
    }

    struct dirent* entry;
    int processed_count = 0;
    int error_count = 0;

    // Change to the input directory
    char original_dir[PATH_MAX];
    if (getcwd(original_dir, sizeof(original_dir)) == NULL) {
        printf("Error: Could not get current working directory\n");
        closedir(dir);
        return -1;
    }

    if (chdir(options->input_dir) != 0) {
        printf("Error: Could not change to input directory: %s\n", 
               strerror(errno));
        closedir(dir);
        return -1;
    }

    // Process each file in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG) continue; // Skip if not a regular file
        
        // Skip files that don't match extension filter if one is set
        if (options->extension_filter && 
            !strstr(entry->d_name, options->extension_filter)) {
            continue;
        }

        // Check if it's a supported image file
        if (!is_supported_image(entry->d_name)) {
            continue;
        }

        // Get the output filename
        char* output_filename = NULL;
        if (options->replace_originals) {
            output_filename = get_temp_filename(entry->d_name);
        } else {
            output_filename = get_output_filename(entry->d_name, 
                                                options->target_format);
        }

        if (!output_filename) {
            printf("Error: Could not create output filename for %s\n", 
                   entry->d_name);
            error_count++;
            continue;
        }

        // Load and convert the image
        ImageFormat input_format = detect_format(entry->d_name);
        ImageData* img = NULL;

        switch (input_format) {
            case FORMAT_PNG:
                img = load_png(entry->d_name);
                break;
            case FORMAT_WEBP:
                img = load_webp(entry->d_name);
                break;
            case FORMAT_JPG:
                img = load_jpeg(entry->d_name);
                break;
            case FORMAT_AVIF:
                img = load_avif(entry->d_name);
                break;
            case FORMAT_HEIC:
                img = load_heic(entry->d_name);
                break;
            default:
                printf("Skipping unsupported format: %s\n", entry->d_name);
                free(output_filename);
                continue;
        }

        if (!img) {
            printf("Error: Could not load image %s\n", entry->d_name);
            free(output_filename);
            error_count++;
            continue;
        }

        // Save in new format
        bool save_success = false;
        switch (options->target_format) {
            case FORMAT_PNG:
                save_success = save_png(output_filename, img, &options->options);
                break;
            case FORMAT_WEBP:
                save_success = save_webp(output_filename, img, &options->options);
                break;
            case FORMAT_JPG:
                save_success = save_jpeg(output_filename, img, &options->options);
                break;
            case FORMAT_AVIF:
                save_success = save_avif(output_filename, img, &options->options);
                break;
            case FORMAT_HEIC:
                save_success = save_heic(output_filename, img, &options->options);
                break;
            default:
                printf("Error: Unsupported output format\n");
                save_success = false;
                break;
        }

        // Cleanup image data
        free_image_data(img);
        free(img);

        if (save_success) {
            if (options->replace_originals) {
                // Remove original and rename temp file
                if (remove(entry->d_name) != 0) {
                    printf("Error: Could not remove original file %s\n", 
                           entry->d_name);
                    remove(output_filename); // Clean up temp file
                    free(output_filename);
                    error_count++;
                    continue;
                }
                if (rename(output_filename, entry->d_name) != 0) {
                    printf("Error: Could not rename temp file %s\n", 
                           output_filename);
                    free(output_filename);
                    error_count++;
                    continue;
                }
            } else {
                // If not replacing, but conversion succeeded, delete the original
                if (remove(entry->d_name) != 0) {
                    printf("Warning: Could not remove original file %s\n", 
                           entry->d_name);
                    // Don't count this as an error since conversion succeeded
                }
            }
            processed_count++;
            printf("Successfully converted: %s\n", entry->d_name);
        } else {
            printf("Error: Failed to convert %s\n", entry->d_name);
            if (options->replace_originals) {
                remove(output_filename); // Clean up temp file
            }
            error_count++;
        }

        free(output_filename);
    }

    // Change back to original directory
    if (chdir(original_dir) != 0) {
        printf("Warning: Could not change back to original directory\n");
    }

    closedir(dir);

    printf("\nBatch processing complete:\n");
    printf("Successfully processed: %d files\n", processed_count);
    printf("Errors encountered: %d files\n", error_count);

    return processed_count;
}

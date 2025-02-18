#ifndef BATCH_PROCESSOR_H
#define BATCH_PROCESSOR_H

#include "converter.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
    char* input_dir;
    ImageFormat target_format;
    ConversionOptions options;
    bool replace_originals;
    char* extension_filter;  // Optional: only process files with this extension
} BatchProcessingOptions;

// Main batch processing function
int process_directory(const BatchProcessingOptions* options);

// Helper function to get temporary filename
char* get_temp_filename(const char* original_filename);

// Helper function to check if file is supported image
bool is_supported_image(const char* filename);

#endif // BATCH_PROCESSOR_H

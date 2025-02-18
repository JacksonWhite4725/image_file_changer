#include "../include/converter.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <webp/decode.h>
#include <webp/encode.h>
#include <avif/avif.h>

// Removed 'static' keyword since this is now a public function
ImageData* load_png(const char* filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        printf("Error: Could not open file %s\n", filepath);
        return NULL;
    }

    // Read PNG signature
    unsigned char header[8];
    if (fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return NULL;
    }

    // Verify PNG signature
    if (png_sig_cmp(header, 0, 8)) {
        fclose(fp);
        return NULL;
    }

    // Initialize PNG structs
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return NULL;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp);
        return NULL;
    }

    // Error handling
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    png_init_io(png, fp);
    png_set_sig_bytes(png, 8);
    png_read_info(png, info);

    // Get image info
    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    // Convert to standard RGBA format
    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    // Allocate memory for image data
    ImageData* img = (ImageData*)malloc(sizeof(ImageData));
    if (!img) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->channels = 4; // RGBA
    img->size = width * height * 4;
    img->data = (unsigned char*)malloc(img->size);

    if (!img->data) {
        free(img);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    // Read image data
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = img->data + y * width * 4;
    }

    png_read_image(png, row_pointers);

    // Cleanup
    free(row_pointers);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);

    return img;
}

// The rest of your functions remain the same
ImageFormat detect_format(const char* filepath) {
    // Always check extension first for non-existent (output) files
    const char* ext = strrchr(filepath, '.');
    if (!ext) return FORMAT_UNKNOWN;
    ext++;

    // For output files, we can only rely on extension
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        // File doesn't exist (probably an output file), use extension
        if (strcasecmp(ext, "heic") == 0) return FORMAT_HEIC;
        if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) return FORMAT_JPG;
        if (strcasecmp(ext, "png") == 0) return FORMAT_PNG;
        if (strcasecmp(ext, "webp") == 0) return FORMAT_WEBP;
        if (strcasecmp(ext, "avif") == 0) return FORMAT_AVIF;
        return FORMAT_UNKNOWN;
    }

    // For existing files, try signature detection first
    unsigned char header[32];
    size_t bytes_read = fread(header, 1, 32, fp);
    fclose(fp);

    if (bytes_read >= 8) {
        // Check PNG signature
        if (png_sig_cmp(header, 0, 8) == 0) {
            return FORMAT_PNG;
        }

        // Check JPEG signature
        if (header[0] == 0xFF && header[1] == 0xD8 && header[2] == 0xFF) {
            return FORMAT_JPG;
        }

        // Check WebP signature (needs 12 bytes)
        if (bytes_read >= 12 && memcmp(header, "RIFF", 4) == 0 && 
            memcmp(header + 8, "WEBP", 4) == 0) {
            return FORMAT_WEBP;
        }

        // Check AVIF signature (look for 'ftyp' and 'avif' in the header)
        if (bytes_read >= 32) {
            for (size_t i = 0; i < bytes_read - 8; i++) {
                if (memcmp(header + i, "ftyp", 4) == 0) {
                    if (memcmp(header + i + 4, "avif", 4) == 0 ||
                        memcmp(header + i + 4, "avis", 4) == 0) {
                        return FORMAT_AVIF;
                    }
                }
            }
        }
    }

    // Fallback to extension-based detection for existing files
    if (strcasecmp(ext, "heic") == 0) return FORMAT_HEIC;
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) return FORMAT_JPG;
    if (strcasecmp(ext, "png") == 0) return FORMAT_PNG;
    if (strcasecmp(ext, "webp") == 0) return FORMAT_WEBP;
    if (strcasecmp(ext, "avif") == 0) return FORMAT_AVIF;
    
    return FORMAT_UNKNOWN;
}

const char* format_to_string(ImageFormat format) {
    switch (format) {
        case FORMAT_HEIC: return "HEIC";
        case FORMAT_JPG: return "JPG";
        case FORMAT_PNG: return "PNG";
        case FORMAT_WEBP: return "WEBP";
        case FORMAT_AVIF: return "AVIF";
        default: return "UNKNOWN";
    }
}

ImageFormat string_to_format(const char* str) {
    if (!str) return FORMAT_UNKNOWN;
    
    if (strcasecmp(str, "heic") == 0) return FORMAT_HEIC;
    if (strcasecmp(str, "jpg") == 0 || strcasecmp(str, "jpeg") == 0) return FORMAT_JPG;
    if (strcasecmp(str, "png") == 0) return FORMAT_PNG;
    if (strcasecmp(str, "webp") == 0) return FORMAT_WEBP;
    if (strcasecmp(str, "avif") == 0) return FORMAT_AVIF;
    
    return FORMAT_UNKNOWN;
}

void free_image_data(ImageData* img) {
    if (img && img->data) {
        free(img->data);
        img->data = NULL;
    }
}

bool save_png(const char* filepath, const ImageData* img, const ConversionOptions* options) {
    if (!img || !img->data || !filepath) {
        return false;
    }

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        printf("Error: Could not open file %s for writing\n", filepath);
        return false;
    }

    // Initialize PNG write structure
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return false;
    }

    // Initialize info structure
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        return false;
    }

    // Error handling
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return false;
    }

    png_init_io(png, fp);

    // Set compression level based on quality option
    int compression_level = PNG_COMPRESSION_TYPE_DEFAULT;
    if (options && options->quality >= 0 && options->quality <= 100) {
        compression_level = (options->quality * 9) / 100;  // Convert 0-100 to 0-9 range
        png_set_compression_level(png, compression_level);
    }

    // Write header
    png_set_IHDR(png, info, img->width, img->height,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);

    // Write image data
    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * img->height);
    for (size_t y = 0; y < img->height; y++) {
        row_pointers[y] = (png_bytep)(img->data + y * img->width * 4);
    }

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    // Cleanup
    free(row_pointers);
    png_destroy_write_struct(&png, &info);
    fclose(fp);

    return true;
}

// Custom error handler structure
typedef struct {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
    char error_message[JMSG_LENGTH_MAX];
} jpeg_error_mgr_wrapper;

// Custom error handler
static void jpeg_error_exit(j_common_ptr cinfo) {
    jpeg_error_mgr_wrapper* err = (jpeg_error_mgr_wrapper*)cinfo->err;
    (*cinfo->err->format_message)(cinfo, err->error_message);
    longjmp(err->setjmp_buffer, 1);
}

ImageData* load_jpeg(const char* filepath) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        printf("Error: Could not open JPEG file %s\n", filepath);
        return NULL;
    }

    // Initialize decompression objects
    struct jpeg_decompress_struct cinfo;
    jpeg_error_mgr_wrapper jerr;
    
    // Set up error handling
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_error_exit;
    
    if (setjmp(jerr.setjmp_buffer)) {
        printf("JPEG Error: %s\n", jerr.error_message);
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return NULL;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    
    // Set decompression parameters
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    // Allocate memory for the image
    ImageData* img = (ImageData*)malloc(sizeof(ImageData));
    if (!img) {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return NULL;
    }

    img->width = cinfo.output_width;
    img->height = cinfo.output_height;
    img->channels = 4;  // We'll convert to RGBA
    img->size = img->width * img->height * img->channels;
    img->data = (unsigned char*)malloc(img->size);

    if (!img->data) {
        free(img);
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return NULL;
    }

    // Allocate a one-row-high array of RGB pixels
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr)&cinfo, JPOOL_IMAGE, img->width * 3, 1);

    // Read scanlines and convert from RGB to RGBA
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char* row = img->data + (cinfo.output_scanline * img->width * 4);
        jpeg_read_scanlines(&cinfo, buffer, 1);
        
        // Convert RGB to RGBA
        for (size_t i = 0, j = 0; i < img->width * 3; i += 3, j += 4) {
            row[j] = buffer[0][i];     // R
            row[j + 1] = buffer[0][i + 1]; // G
            row[j + 2] = buffer[0][i + 2]; // B
            row[j + 3] = 255;          // A (fully opaque)
        }
    }

    // Cleanup
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return img;
}

bool save_jpeg(const char* filepath, const ImageData* img, const ConversionOptions* options) {
    if (!img || !img->data || !filepath) {
        return false;
    }

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        printf("Error: Could not open file %s for writing\n", filepath);
        return false;
    }

    // Initialize compression objects
    struct jpeg_compress_struct cinfo;
    jpeg_error_mgr_wrapper jerr;

    // Set up error handling
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        printf("JPEG Error: %s\n", jerr.error_message);
        jpeg_destroy_compress(&cinfo);
        fclose(fp);
        return false;
    }

    // Initialize compression
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);

    // Set image parameters
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    cinfo.input_components = 3;  // RGB
    cinfo.in_color_space = JCS_RGB;

    // Set defaults and compression parameters
    jpeg_set_defaults(&cinfo);
    
    // Set quality (0-100)
    int quality = options ? options->quality : 90;
    quality = quality < 0 ? 0 : (quality > 100 ? 100 : quality);
    jpeg_set_quality(&cinfo, quality, TRUE);

    // Set progressive mode if requested
    if (options && options->jpeg_options.progressive) {
        jpeg_simple_progression(&cinfo);
    }

    // Set optimization
    if (options && options->jpeg_options.optimization > 0) {
        cinfo.optimize_coding = TRUE;
    }

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Allocate temporary buffer for RGB data
    JSAMPROW row_buffer = (JSAMPROW)malloc(img->width * 3);
    if (!row_buffer) {
        jpeg_destroy_compress(&cinfo);
        fclose(fp);
        return false;
    }

    // Write scanlines, converting from RGBA to RGB
    while (cinfo.next_scanline < cinfo.image_height) {
        const unsigned char* rgba_row = img->data + (cinfo.next_scanline * img->width * 4);
        
        // Convert RGBA to RGB
        for (size_t i = 0, j = 0; i < img->width * 4; i += 4, j += 3) {
            row_buffer[j] = rgba_row[i];     // R
            row_buffer[j + 1] = rgba_row[i + 1]; // G
            row_buffer[j + 2] = rgba_row[i + 2]; // B
            // Alpha channel is discarded
        }

        jpeg_write_scanlines(&cinfo, &row_buffer, 1);
    }

    // Cleanup
    free(row_buffer);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);

    return true;
}

ImageData* load_webp(const char* filepath) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        printf("Error: Could not open WebP file %s\n", filepath);
        return NULL;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Read file data
    uint8_t* file_data = (uint8_t*)malloc(file_size);
    if (!file_data) {
        fclose(fp);
        return NULL;
    }
    if (fread(file_data, 1, file_size, fp) != file_size) {
        free(file_data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    // Get WebP image information
    WebPBitstreamFeatures features;
    if (WebPGetFeatures(file_data, file_size, &features) != VP8_STATUS_OK) {
        free(file_data);
        return NULL;
    }

    // Allocate image data structure
    ImageData* img = (ImageData*)malloc(sizeof(ImageData));
    if (!img) {
        free(file_data);
        return NULL;
    }

    img->width = features.width;
    img->height = features.height;
    img->channels = 4; // We'll decode to RGBA
    img->size = img->width * img->height * img->channels;
    img->data = (unsigned char*)malloc(img->size);

    if (!img->data) {
        free(file_data);
        free(img);
        return NULL;
    }

    // Decode WebP to RGBA
    if (!WebPDecodeRGBAInto(file_data, file_size, 
                           img->data, img->size,
                           img->width * img->channels)) {
        free(file_data);
        free(img->data);
        free(img);
        return NULL;
    }

    free(file_data);
    return img;
}

bool save_webp(const char* filepath, const ImageData* img, const ConversionOptions* options) {
    if (!img || !img->data || !filepath) {
        return false;
    }

    // Set up WebP config
    WebPConfig config;
    if (!WebPConfigInit(&config)) {
        return false;
    }

    // Configure based on options
    if (options) {
        // Quality setting (0-100)
        config.quality = options->quality;
        
        if (options->webp_options.lossless) {
            config.lossless = 1;
            config.quality = 100;
        }

        if (options->webp_options.exact) {
            config.exact = 1;
        }
    } else {
        config.quality = 90;
    }

    // Validate configuration
    if (!WebPValidateConfig(&config)) {
        return false;
    }

    // Set up WebP picture
    WebPPicture picture;
    if (!WebPPictureInit(&picture)) {
        return false;
    }

    picture.width = img->width;
    picture.height = img->height;
    picture.use_argb = 1;

    // Allocate memory for the picture
    if (!WebPPictureAlloc(&picture)) {
        return false;
    }

    // Import RGBA data
    if (!WebPPictureImportRGBA(&picture, img->data, img->width * 4)) {
        WebPPictureFree(&picture);
        return false;
    }

    // Set up WebP memory writer
    WebPMemoryWriter memory_writer;
    WebPMemoryWriterInit(&memory_writer);
    picture.writer = WebPMemoryWrite;
    picture.custom_ptr = &memory_writer;

    // Encode
    if (!WebPEncode(&config, &picture)) {
        WebPPictureFree(&picture);
        WebPMemoryWriterClear(&memory_writer);
        return false;
    }

    // Write to file
    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        WebPPictureFree(&picture);
        WebPMemoryWriterClear(&memory_writer);
        return false;
    }

    fwrite(memory_writer.mem, memory_writer.size, 1, fp);
    fclose(fp);

    // Cleanup
    WebPPictureFree(&picture);
    WebPMemoryWriterClear(&memory_writer);

    return true;
}

ImageData* load_avif(const char* filepath) {
    // Create decoder
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) {
        printf("Error: Could not create AVIF decoder\n");
        return NULL;
    }

    // Read file
    avifResult result = avifDecoderSetIOFile(decoder, filepath);
    if (result != AVIF_RESULT_OK) {
        printf("Error: Could not open AVIF file: %s\n", avifResultToString(result));
        avifDecoderDestroy(decoder);
        return NULL;
    }

    // Parse image
    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        printf("Error: Could not parse AVIF file: %s\n", avifResultToString(result));
        avifDecoderDestroy(decoder);
        return NULL;
    }

    // Read image
    result = avifDecoderNextImage(decoder);
    if (result != AVIF_RESULT_OK) {
        printf("Error: Could not decode AVIF image: %s\n", avifResultToString(result));
        avifDecoderDestroy(decoder);
        return NULL;
    }

    // Allocate our image structure
    ImageData* img = (ImageData*)malloc(sizeof(ImageData));
    if (!img) {
        avifDecoderDestroy(decoder);
        return NULL;
    }

    img->width = decoder->image->width;
    img->height = decoder->image->height;
    img->channels = 4; // RGBA
    img->size = img->width * img->height * img->channels;
    img->data = (unsigned char*)malloc(img->size);

    if (!img->data) {
        free(img);
        avifDecoderDestroy(decoder);
        return NULL;
    }

    // Convert AVIF to RGBA
    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, decoder->image);
    rgb.format = AVIF_RGB_FORMAT_RGBA;
    rgb.depth = 8;
    rgb.pixels = img->data;
    rgb.rowBytes = img->width * 4;

    result = avifImageYUVToRGB(decoder->image, &rgb);
    if (result != AVIF_RESULT_OK) {
        printf("Error: Could not convert AVIF to RGB: %s\n", avifResultToString(result));
        free(img->data);
        free(img);
        avifDecoderDestroy(decoder);
        return NULL;
    }

    // Cleanup decoder
    avifDecoderDestroy(decoder);

    return img;
}

bool save_avif(const char* filepath, const ImageData* img, const ConversionOptions* options) {
    if (!img || !img->data || !filepath) {
        return false;
    }

    // Create encoder
    avifEncoder* encoder = avifEncoderCreate();
    if (!encoder) {
        printf("Error: Could not create AVIF encoder\n");
        return false;
    }

    // Configure encoder based on options
    if (options) {
        encoder->speed = options->avif_options.speed;
        encoder->minQuantizer = encoder->maxQuantizer = 
            options->avif_options.lossless ? AVIF_QUANTIZER_LOSSLESS : 
            (63 - ((options->quality * 63) / 100));
    } else {
        encoder->speed = 6; // Default speed
        encoder->minQuantizer = encoder->maxQuantizer = 25; // ~90% quality
    }

    // Create image
    avifImage* avifImg = avifImageCreate(img->width, img->height, 8, AVIF_PIXEL_FORMAT_YUV444);
    if (!avifImg) {
        printf("Error: Could not create AVIF image\n");
        avifEncoderDestroy(encoder);
        return false;
    }

    // Convert RGBA to AVIF
    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, avifImg);
    rgb.format = AVIF_RGB_FORMAT_RGBA;
    rgb.depth = 8;
    rgb.pixels = img->data;
    rgb.rowBytes = img->width * 4;

    avifResult result = avifImageRGBToYUV(avifImg, &rgb);
    if (result != AVIF_RESULT_OK) {
        printf("Error: Could not convert RGB to AVIF: %s\n", avifResultToString(result));
        avifImageDestroy(avifImg);
        avifEncoderDestroy(encoder);
        return false;
    }

    // Encode image
    avifRWData output = AVIF_DATA_EMPTY;
    result = avifEncoderWrite(encoder, avifImg, &output);
    if (result != AVIF_RESULT_OK) {
        printf("Error: Could not encode AVIF: %s\n", avifResultToString(result));
        avifImageDestroy(avifImg);
        avifEncoderDestroy(encoder);
        return false;
    }

    // Write to file
    FILE* f = fopen(filepath, "wb");
    if (!f) {
        printf("Error: Could not open output file\n");
        avifRWDataFree(&output);
        avifImageDestroy(avifImg);
        avifEncoderDestroy(encoder);
        return false;
    }

    size_t bytesWritten = fwrite(output.data, 1, output.size, f);
    fclose(f);

    // Cleanup
    avifRWDataFree(&output);
    avifImageDestroy(avifImg);
    avifEncoderDestroy(encoder);

    return bytesWritten == output.size;
}

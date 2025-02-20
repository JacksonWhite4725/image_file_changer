#ifndef MEDIA_PROCESSOR_GUI_H
#define MEDIA_PROCESSOR_GUI_H

#include <gtk/gtk.h>
#include "converter.h"
#include "batch_processor.h"

// Struct for single file processing tab
typedef struct {
    GtkWidget *main_box;
    GtkWidget *preview_image;
    GtkWidget *file_chooser_button;
    GtkWidget *filename_label;
    char *current_filename;
} SingleFileTab;

// Struct for batch processing tab
typedef struct {
    GtkWidget *main_box;
    GtkWidget *file_list;
    GtkListStore *list_store;
    GtkWidget *dir_chooser_button;
    GtkWidget *files_found_label;
} BatchTab;

typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;       // For tabbed interface
    GtkWidget *format_combo;
    GtkWidget *quality_scale;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    SingleFileTab *single_tab;  // Single file processing widgets
    BatchTab *batch_tab;       // Batch processing widgets
    ImageFormat target_format;
    int quality;
} AppWindow;

void create_gui(int *argc, char ***argv);

// Helper function declarations
void scan_directory_for_images(const char* directory, GtkListStore* store);
gboolean update_image_preview(const char* filename, GtkWidget* preview_image);

#endif // MEDIA_PROCESSOR_GUI_H

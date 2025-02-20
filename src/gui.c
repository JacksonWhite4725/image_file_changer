#include <gtk/gtk.h>
#include "../include/gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

// CSS for styling
const char *CSS = "                                    \
    window {                                           \
        background-color: #f0f0f0;                     \
    }                                                  \
    button {                                          \
        background: linear-gradient(#fafafa, #e0e0e0); \
        border: 1px solid #999;                        \
        border-radius: 4px;                            \
        padding: 6px 12px;                             \
    }                                                  \
    button:hover {                                     \
        background: linear-gradient(#e0e0e0, #d0d0d0); \
    }                                                  \
    .preview-box {                                     \
        background-color: #ffffff;                     \
        border: 1px solid #ccc;                        \
        border-radius: 4px;                            \
        padding: 10px;                                 \
    }                                                  \
    .status-label {                                    \
        color: #333333;                               \
        font-weight: bold;                            \
        font-size: 14px;                              \
        padding: 8px;                                 \
    }                                                  \
    progress, trough {                                \
        min-height: 20px;                             \
    }                                                 \
    progress {                                        \
        background-color: #4CAF50;                    \
        border-radius: 3px;                           \
    }                                                 \
    trough {                                          \
        background-color: #e0e0e0;                    \
        border-radius: 3px;                           \
    }                                                 \
";

static void format_changed(GtkComboBox *combo, gpointer data) {
    AppWindow *app = (AppWindow *)data;
    gchar *text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    app->target_format = string_to_format(text);
    g_free(text);
}

static void quality_changed(GtkRange *range, gpointer data) {
    AppWindow *app = (AppWindow *)data;
    app->quality = (int)gtk_range_get_value(range);
}

static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static char *get_output_filename(const char *input_filename, ImageFormat target_format) {
    const char *last_dot = strrchr(input_filename, '.');
    if (!last_dot) return NULL;

    size_t base_len = last_dot - input_filename;
    const char *new_ext = format_to_string(target_format);
    if (!new_ext) return NULL;

    size_t new_len = base_len + strlen(new_ext) + 2;
    char *output_filename = (char *)malloc(new_len);
    if (!output_filename) return NULL;

    strncpy(output_filename, input_filename, base_len);
    snprintf(output_filename + base_len, new_len - base_len, ".%s", new_ext);

    return output_filename;
}

// Update preview image in single file mode
gboolean update_image_preview(const char* filename, GtkWidget* preview_image) {
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filename, 300, -1, TRUE, NULL);
    if (pixbuf) {
        gtk_image_set_from_pixbuf(GTK_IMAGE(preview_image), pixbuf);
        g_object_unref(pixbuf);
        return TRUE;
    }
    return FALSE;
}

// Single file selection handler
static void on_single_file_chosen(GtkFileChooserButton *button, gpointer data) {
    AppWindow *app = (AppWindow *)data;
    const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
    
    printf("Debug: File selected: %s\n", filename ? filename : "NULL"); // Debug print
    
    if (!filename) {
        gtk_label_set_markup(GTK_LABEL(app->status_label), 
                           "<span foreground='red'><b>No file selected</b></span>");
        return;
    }

    ImageFormat format = detect_format(filename);
    printf("Debug: Detected format: %s\n", format_to_string(format)); // Debug print

    if (format == FORMAT_UNKNOWN) {
        gtk_label_set_markup(GTK_LABEL(app->status_label), 
                           "<span foreground='red'><b>Unsupported file format</b></span>");
        return;
    }

    // Update preview and store filename
    if (app->single_tab->current_filename) {
        g_free(app->single_tab->current_filename);
    }
    app->single_tab->current_filename = g_strdup(filename);
    
    // Update the preview image
    if (!update_image_preview(filename, app->single_tab->preview_image)) {
        printf("Debug: Failed to update preview\n"); // Debug print
        gtk_label_set_markup(GTK_LABEL(app->status_label), 
                           "<span foreground='red'><b>Failed to load preview</b></span>");
        return;
    }

    // Update UI with success
    char *display_text = g_strdup_printf("<b>Selected: %s</b>", g_path_get_basename(filename));
    gtk_label_set_markup(GTK_LABEL(app->single_tab->filename_label), display_text);
    gtk_label_set_markup(GTK_LABEL(app->status_label), 
                       "<span foreground='green'><b>Ready to convert!</b></span>");
    g_free(display_text);
}

// Scan directory for supported images
void scan_directory_for_images(const char* directory, GtkListStore* store) {
    DIR *dir;
    struct dirent *entry;
    gtk_list_store_clear(store);
    
    dir = opendir(directory);
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {  // Regular file
                char *full_path = g_build_filename(directory, entry->d_name, NULL);
                if (detect_format(full_path) != FORMAT_UNKNOWN) {
                    GtkTreeIter iter;
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter, 0, full_path, -1);
                }
                g_free(full_path);
            }
        }
        closedir(dir);
    }
}

// Directory selection handler for batch processing
static void on_directory_chosen(GtkFileChooserButton *button, gpointer data) {
    AppWindow *app = (AppWindow *)data;
    const char *dirname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
    
    if (dirname) {
        scan_directory_for_images(dirname, app->batch_tab->list_store);
        
        // Count files found
        GtkTreeModel *model = GTK_TREE_MODEL(app->batch_tab->list_store);
        gint count = gtk_tree_model_iter_n_children(model, NULL);
        
        char *label_text = g_strdup_printf("Found %d supported images", count);
        gtk_label_set_text(GTK_LABEL(app->batch_tab->files_found_label), label_text);
        g_free(label_text);
    }
}

// Convert button handler
static void convert_clicked(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    AppWindow *app = (AppWindow *)data;
    int current_tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook));
    
    ConversionOptions options = {
        .quality = app->quality,
        .maintain_exif = true
    };

    if (current_tab == 0) {  // Single file mode
        if (!app->single_tab->current_filename) {
            gtk_label_set_text(GTK_LABEL(app->status_label), "No file selected");
            return;
        }

        char *output_filename = get_output_filename(app->single_tab->current_filename, 
                                                  app->target_format);
        if (!output_filename) {
            gtk_label_set_text(GTK_LABEL(app->status_label), "Error creating output filename");
            return;
        }

        gtk_label_set_text(GTK_LABEL(app->status_label), "Converting...");
        gtk_widget_set_sensitive(app->window, FALSE);
        
        if (convert_image(app->single_tab->current_filename, output_filename, 
                         app->target_format, &options)) {
            remove(app->single_tab->current_filename);  // Remove original after successful conversion
            g_free(app->single_tab->current_filename);
            app->single_tab->current_filename = g_strdup(output_filename);
            update_image_preview(output_filename, app->single_tab->preview_image);
            gtk_label_set_text(GTK_LABEL(app->status_label), "Conversion complete");
        } else {
            gtk_label_set_text(GTK_LABEL(app->status_label), "Conversion failed");
        }
        
        free(output_filename);
        gtk_widget_set_sensitive(app->window, TRUE);

    } else {  // Batch processing mode
        GtkTreeModel *model = GTK_TREE_MODEL(app->batch_tab->list_store);
        GtkTreeIter iter;
        gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
        
        if (!valid) {
            gtk_label_set_text(GTK_LABEL(app->status_label), "No files to convert");
            return;
        }

        int total_files = gtk_tree_model_iter_n_children(model, NULL);
        int processed_files = 0;

        gtk_label_set_text(GTK_LABEL(app->status_label), "Converting files...");
        gtk_widget_set_sensitive(app->window, FALSE);

        while (valid) {
            char *input_filename;
            gtk_tree_model_get(model, &iter, 0, &input_filename, -1);

            if (input_filename) {
                char *output_filename = get_output_filename(input_filename, app->target_format);
                if (output_filename) {
                    if (convert_image(input_filename, output_filename, app->target_format, &options)) {
                        remove(input_filename);  // Remove original after successful conversion
                        processed_files++;
                    }
                    free(output_filename);
                }
                g_free(input_filename);
            }

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar),
                                        (gdouble)processed_files / total_files);

            while (gtk_events_pending()) {
                gtk_main_iteration();
            }

            valid = gtk_tree_model_iter_next(model, &iter);
        }

        gtk_widget_set_sensitive(app->window, TRUE);
        
        // Show 100% completion briefly before resetting
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
        while (gtk_events_pending()) gtk_main_iteration();
        g_usleep(500000); // Show 100% for half a second
        
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);
        gtk_list_store_clear(app->batch_tab->list_store);
        
        // Create a more noticeable success message
        char *success_msg = g_strdup_printf(
            "<span foreground='green' size='large'><b>✓ Successfully converted %d files!</b></span>",
            processed_files);
        gtk_label_set_markup(GTK_LABEL(app->status_label), success_msg);
        g_free(success_msg);
    }
}

void create_gui(int *argc, char ***argv) {
    gtk_init(argc, argv);
    apply_css();
    
    AppWindow *app = g_new(AppWindow, 1);
    app->target_format = FORMAT_PNG;
    app->quality = 90;

    // Create main window
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Media Processor");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 800, 600);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create main vertical box
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);

    // Create notebook (tabbed interface)
    app->notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), app->notebook, TRUE, TRUE, 0);

    // Create single file tab
    app->single_tab = g_new(SingleFileTab, 1);
    app->single_tab->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(app->single_tab->main_box), 10);

    // Single file chooser
    app->single_tab->file_chooser_button = gtk_file_chooser_button_new(
        "Select File", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_box_pack_start(GTK_BOX(app->single_tab->main_box), 
                      app->single_tab->file_chooser_button, 
                      FALSE, FALSE, 0);

    // Create preview box
    GtkWidget *preview_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(preview_box, "preview-box");
    
    // Create preview image
    app->single_tab->preview_image = gtk_image_new();
    gtk_widget_set_size_request(app->single_tab->preview_image, 300, 200);
    gtk_box_pack_start(GTK_BOX(preview_box), app->single_tab->preview_image, TRUE, TRUE, 0);
    
    // Create filename label
    app->single_tab->filename_label = gtk_label_new("No file selected");
    gtk_box_pack_start(GTK_BOX(preview_box), app->single_tab->filename_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(app->single_tab->main_box), preview_box, TRUE, TRUE, 0);
    app->single_tab->current_filename = NULL;

    // Create batch processing tab
    app->batch_tab = g_new(BatchTab, 1);
    app->batch_tab->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(app->batch_tab->main_box), 10);

    // Directory chooser for batch processing
    app->batch_tab->dir_chooser_button = gtk_file_chooser_button_new(
        "Select Directory", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_box_pack_start(GTK_BOX(app->batch_tab->main_box), 
                      app->batch_tab->dir_chooser_button, FALSE, FALSE, 0);

    // Files found label
    app->batch_tab->files_found_label = gtk_label_new("No directory selected");
    gtk_box_pack_start(GTK_BOX(app->batch_tab->main_box), 
                      app->batch_tab->files_found_label, FALSE, FALSE, 0);

    // Create file list for batch processing
    app->batch_tab->list_store = gtk_list_store_new(1, G_TYPE_STRING);
    app->batch_tab->file_list = gtk_tree_view_new_with_model(
        GTK_TREE_MODEL(app->batch_tab->list_store));
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
        "Files", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app->batch_tab->file_list), column);

    // Create scrolled window for file list
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), app->batch_tab->file_list);
    gtk_box_pack_start(GTK_BOX(app->batch_tab->main_box), scroll, TRUE, TRUE, 0);

    // Add tabs to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook),
                           app->single_tab->main_box,
                           gtk_label_new("Single File"));
    gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook),
                           app->batch_tab->main_box,
                           gtk_label_new("Batch Processing"));

    // Create format selection combo box
    GtkWidget *format_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), format_box, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(format_box),
                      gtk_label_new("Target Format:"),
                      FALSE, FALSE, 0);
    
    app->format_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->format_combo), "PNG");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->format_combo), "JPG");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->format_combo), "WEBP");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->format_combo), "AVIF");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->format_combo), "HEIC");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->format_combo), 0);
    gtk_box_pack_start(GTK_BOX(format_box), app->format_combo, TRUE, TRUE, 0);

    // Create quality slider
    GtkWidget *quality_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), quality_box, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(quality_box),
                      gtk_label_new("Quality:"),
                      FALSE, FALSE, 0);
    
    app->quality_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(app->quality_scale), 90);
    gtk_box_pack_start(GTK_BOX(quality_box), app->quality_scale, TRUE, TRUE, 0);

    // Create convert button
    GtkWidget *convert_button = gtk_button_new_with_label("Convert");
    gtk_widget_set_name(convert_button, "convert-button");
    gtk_box_pack_start(GTK_BOX(main_box), convert_button, FALSE, FALSE, 0);

    // Create progress bar
    app->progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(main_box), app->progress_bar, FALSE, FALSE, 0);

    // Create status label
    app->status_label = gtk_label_new("Ready");
    gtk_widget_set_name(app->status_label, "status-label");
    gtk_box_pack_start(GTK_BOX(main_box), app->status_label, FALSE, FALSE, 0);

    // Connect signals
    g_signal_connect(app->single_tab->file_chooser_button, "file-set",
                    G_CALLBACK(on_single_file_chosen), app);
    g_signal_connect(app->batch_tab->dir_chooser_button, "file-set",
                    G_CALLBACK(on_directory_chosen), app);
    g_signal_connect(app->format_combo, "changed",
                    G_CALLBACK(format_changed), app);
    g_signal_connect(app->quality_scale, "value-changed",
                    G_CALLBACK(quality_changed), app);
    g_signal_connect(convert_button, "clicked",
                    G_CALLBACK(convert_clicked), app);

    // Set file filters
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.webp");
    gtk_file_filter_add_pattern(filter, "*.avif");
    gtk_file_filter_add_pattern(filter, "*.heic");
    
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(app->single_tab->file_chooser_button), filter);

    // Show all widgets
    gtk_widget_show_all(app->window);

    // Start main loop
    gtk_main();

    // Cleanup
    if (app->single_tab->current_filename) {
        g_free(app->single_tab->current_filename);
    }
    g_free(app->single_tab);
    g_free(app->batch_tab);
    g_free(app);
}

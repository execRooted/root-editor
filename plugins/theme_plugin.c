#include "../src/plugin.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define THEME_DEFAULT 0
#define THEME_BLUE 1
#define THEME_CYAN 2
#define THEME_YELLOW 3

static int current_theme = THEME_DEFAULT;

static void get_global_theme_path(char *out_path, size_t out_sz) {
    const char *home = getenv("HOME");
    if (!home) home = ".";
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/.config/root-editor", home);
    mkdir(dir, 0755);
    snprintf(out_path, out_sz, "%s/theme", dir);
}

static void set_theme_colors(int theme) {
    if (!has_colors()) return;

    int text_fg, syntax_fg;

    switch (theme) {
        case THEME_DEFAULT:
            text_fg = COLOR_WHITE;
            syntax_fg = COLOR_WHITE;
            break;
        case THEME_YELLOW:
            text_fg = COLOR_YELLOW;
            syntax_fg = COLOR_YELLOW;
            break;
        case THEME_BLUE:
            text_fg = COLOR_BLUE;
            syntax_fg = COLOR_BLUE;
            break;
        case THEME_CYAN:
            text_fg = COLOR_CYAN;
            syntax_fg = COLOR_CYAN;
            break;
        default:
            text_fg = COLOR_WHITE;
            syntax_fg = COLOR_WHITE;
            break;
    }

    
    init_pair(1, text_fg, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLUE);
    init_pair(13, text_fg, COLOR_BLACK);
    init_pair(28, text_fg, COLOR_BLACK);

    
    init_pair(7, COLOR_YELLOW, COLOR_BLACK);
    init_pair(8, COLOR_GREEN, COLOR_BLACK);
    init_pair(9, syntax_fg, COLOR_BLACK);
    init_pair(10, COLOR_RED, COLOR_BLACK);
    init_pair(11, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(12, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(14, COLOR_CYAN, COLOR_BLACK);
    init_pair(15, syntax_fg, COLOR_BLACK);
    init_pair(16, COLOR_BLUE, COLOR_BLACK);
    init_pair(17, COLOR_RED, COLOR_BLACK);
    init_pair(18, COLOR_RED, COLOR_BLACK);
    init_pair(19, COLOR_BLACK, COLOR_RED);
    init_pair(20, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(21, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(22, COLOR_YELLOW, COLOR_BLACK);
    init_pair(23, COLOR_RED, COLOR_BLACK);
    init_pair(24, COLOR_GREEN, COLOR_BLACK);
    init_pair(25, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(26, COLOR_BLUE, COLOR_BLACK);
    init_pair(27, COLOR_YELLOW, COLOR_BLACK);

    bkgdset(COLOR_PAIR(13) | ' ');
}

static void save_theme_for_file(const char* filename) {
    char path[512];
    get_global_theme_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "%d\n", current_theme);
        fclose(f);
    }
}

static void load_theme_for_file(const char* filename) {
    char path[512];
    get_global_theme_path(path, sizeof(path));
    FILE *f = fopen(path, "r");
    if (f) {
        int theme;
        if (fscanf(f, "%d", &theme) == 1) {
            if (theme >= 0 && theme <= 3) {
                current_theme = theme;
            } else {
                current_theme = THEME_DEFAULT;
            }
        } else {
            current_theme = THEME_DEFAULT;
        }
        fclose(f);
    } else {
        current_theme = THEME_DEFAULT;
    }
    set_theme_colors(current_theme);
}

static int theme_plugin_on_keypress(EditorState* state, int ch) {
    if (ch == 25) {
        current_theme = (current_theme + 1) % 4;
        set_theme_colors(current_theme);
        save_theme_for_file(state->filename);

        return 1;
    }
    return 0;
}

static void theme_plugin_on_load(EditorState* state) {
    load_theme_for_file(NULL);
}

static void theme_plugin_on_file_load(EditorState* state, const char* filename) {
    load_theme_for_file(filename);
}

static void theme_plugin_on_file_save(EditorState* state, const char* filename) {
    save_theme_for_file(filename);
}

static PluginInterface plugin_interface = {
    .name = "Theme Plugin",
    .version = "1.0.0",
    .description = "Provides theme switching with Ctrl+Y",
    .on_load = theme_plugin_on_load,
    .on_unload = NULL,
    .on_keypress = theme_plugin_on_keypress,
    .on_render = NULL,
    .on_file_load = theme_plugin_on_file_load,
    .on_file_save = theme_plugin_on_file_save,
    .on_quit = NULL
};
PluginInterface* get_plugin_interface(void) {
    return &plugin_interface;
}
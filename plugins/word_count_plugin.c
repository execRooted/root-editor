#include "../src/editor.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int has_selection(EditorState* state) {
    return state->select_mode && state->select_start_y >= 0 && state->select_end_y >= 0;
}

char* get_selected_text(EditorState* state) {
    if (!has_selection(state)) return NULL;

    int start_y = state->select_start_y;
    int end_y = state->select_end_y;
    int start_x = state->select_start_x;
    int end_x = state->select_end_x;

    size_t total_len = 0;
    for (int y = start_y; y <= end_y; y++) {
        char* line = state->lines[y];
        int len = strlen(line);
        int line_start = (y == start_y) ? start_x : 0;
        int line_end = (y == end_y) ? end_x : len;
        if (line_end > len) line_end = len;
        if (line_start < line_end) {
            total_len += line_end - line_start;
            if (y < end_y) total_len += 1; 
        }
    }

    char* result = (char*)malloc(total_len + 1);
    if (!result) return NULL;
    result[0] = '\0';

    for (int y = start_y; y <= end_y; y++) {
        char* line = state->lines[y];
        int len = strlen(line);
        int line_start = (y == start_y) ? start_x : 0;
        int line_end = (y == end_y) ? end_x : len;
        if (line_end > len) line_end = len;
        if (line_start < line_end) {
            strncat(result, line + line_start, line_end - line_start);
            if (y < end_y) strcat(result, "\n");
        }
    }
    return result;
}

static int count_words(const char* text) {
    int words = 0;
    int in_word = 0;
    for (const char* p = text; *p; p++) {
        if (isspace((unsigned char)*p) || *p == '\n' || *p == '\t') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }
    return words;
}

static int count_lines(const char* text) {
    int lines = 1;
    for (const char* p = text; *p; p++) {
        if (*p == '\n') lines++;
    }
    return lines;
}

static int count_chars(const char* text) {
    return strlen(text);
}

static void word_count_on_render(EditorState* state) {
    (void)state;
}

static PluginInterface plugin_interface = {
    .name = "Word Count Plugin",
    .version = "1.0",
    .description = "Provides word count functionality",
    .on_render = word_count_on_render,
};


PluginInterface* get_plugin_interface(void) {
    return &plugin_interface;
}
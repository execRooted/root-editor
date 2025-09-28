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
    char* text = NULL;
    if (state->select_mode && has_selection(state)) {
        text = get_selected_text(state);
    } else {
        
        size_t total_len = 0;
        for (int i = 0; i < state->line_count; i++) {
            total_len += strlen(state->lines[i]) + 1; 
        }
        text = (char*)malloc(total_len + 1);
        if (text) {
            text[0] = '\0';
            for (int i = 0; i < state->line_count; i++) {
                strcat(text, state->lines[i]);
                if (i < state->line_count - 1) strcat(text, "\n");
            }
        }
    }

    int words = 0;
    if (text) {
        words = count_words(text);
        free(text);
    }

    
    int cur_y, cur_x;
    getyx(stdscr, cur_y, cur_x); 
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    attron(COLOR_PAIR(1)); 
    const char* syntax_status = (state->syntax_enabled ? "ON" : "OFF");
    const char* brackets_status = (state->auto_complete_enabled ? "ON" : "OFF");
    const char* comment_status = (state->comment_complete_enabled ? "ON" : "OFF");
    mvprintw(max_y - 1, 0, "Line: %d, Col: %d | %s | Syntax HL: %s | Autocomplete: %s | Brackets and Quotes Autocomplete: %s | Autosave: %s | Auto Tabbing: %s | Words: %d",
             state->cursor_y + 1, state->cursor_x + 1,
             state->filename[0] ? state->filename : "[Untitled]",
             syntax_status,
             comment_status,
             brackets_status,
             state->autosave_enabled ? "ON" : "OFF",
             state->auto_tabbing_enabled ? "ON" : "OFF",
             words);
    attroff(COLOR_PAIR(1));
    move(cur_y, cur_x); 
}

static PluginInterface plugin_interface = {
    .name = "Word Count Plugin",
    .version = "1.0",
    .description = "Displays word count in bottom left corner",
    .on_render = word_count_on_render,
};


PluginInterface* get_plugin_interface(void) {
    return &plugin_interface;
}
#ifndef EDITOR_H
#define EDITOR_H

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <regex.h>
#include <signal.h>
#include <setjmp.h>

#define MAX_LINES 1000000
#define MAX_LINE_LENGTH 1024
#define TAB_SIZE 4
#define MAX_JSON_RULES 1000
#define MAX_SCOPE_LENGTH 256
#define MAX_COLOR_LENGTH 16
#define MAX_PLUGINS 32


typedef struct EditorState EditorState;
typedef void (*PluginOnLoad)(EditorState* state);
typedef void (*PluginOnUnload)(EditorState* state);
typedef int (*PluginOnKeypress)(EditorState* state, int ch);
typedef void (*PluginOnRender)(EditorState* state);
typedef void (*PluginOnFileLoad)(EditorState* state, const char* filename);
typedef void (*PluginOnFileSave)(EditorState* state, const char* filename);
typedef void (*PluginOnQuit)(EditorState* state);
typedef void (*PluginOnHighlightLine)(EditorState* state, int line_num, int screen_row, int line_num_width, int horizontal_scroll_offset);


typedef struct {
    const char* name;
    const char* version;
    const char* description;

    
    PluginOnLoad on_load;
    PluginOnUnload on_unload;
    PluginOnKeypress on_keypress;
    PluginOnRender on_render;
    PluginOnFileLoad on_file_load;
    PluginOnFileSave on_file_save;
    PluginOnQuit on_quit;
    PluginOnHighlightLine on_highlight_line;
} PluginInterface;


typedef struct Plugin {
    void* handle;
    PluginInterface* interface;
    char path[256];
    int loaded;
} Plugin;

typedef struct EditorState {
    char** lines;
    int line_count;
    int cursor_x, cursor_y;
    int scroll_offset;
    int horizontal_scroll_offset;
    char filename[256];
    int dirty;
    int line_numbers;
    int word_wrap;
    int tab_size;
    int select_mode;
    int select_start_x, select_start_y;
    int select_end_x, select_end_y;
    int show_help;
    int needs_sudo;
    int key_states[256];
    time_t key_timestamps[256];
    int syntax_enabled;
    int syntax_display_enabled;
    int editor_mode; // 0 = text, 1 = selecting
    int auto_tabbing_enabled;
    int file_type;
    char** keywords;
    int keyword_count;

    int theme_id;
    char theme_name[32];

    int autosave_enabled;
    int autosave_interval_sec;
    time_t last_autosave_time;

    int auto_complete_enabled;
    int comment_complete_enabled;

    int json_rules_count;
    char json_scopes[MAX_JSON_RULES][MAX_SCOPE_LENGTH];
    char json_colors[MAX_JSON_RULES][MAX_COLOR_LENGTH];
    char json_font_styles[MAX_JSON_RULES][32];
    int json_loaded;

    char** original_lines;
    int original_line_count;
    time_t last_input_time;
    int rapid_input_mode;
    int char_select_mode;

    
    Plugin plugins[MAX_PLUGINS];
    int plugin_count;

} EditorState;

void init_editor(EditorState* state);
void load_file(EditorState* state, const char* filename);
void save_file(EditorState* state);
void save_file_with_sudo(EditorState* state);
void prompt_filename(EditorState* state);

void prompt_open_file(EditorState* state);
void toggle_syntax_highlighting(EditorState* state);
void toggle_syntax_display(EditorState* state);
void enter_selecting_mode(EditorState* state);
void exit_selecting_mode(EditorState* state);
void toggle_auto_tabbing(EditorState* state);
void toggle_auto_complete(EditorState* state);
void toggle_comment_complete(EditorState* state);

void autosave_check(EditorState* state);
void toggle_autosave(EditorState* state);
void load_config(EditorState* state);
void save_config(EditorState* state);
void safe_quit(EditorState* state);

void init_theme_colors(EditorState* state);
void cycle_theme(EditorState* state);

int can_process_key(EditorState* state, int key_code);
void mark_key_processed(EditorState* state, int key_code);
void reset_key_states(EditorState* state);
void handle_input(EditorState* state, int ch);
void handle_ctrl_keys(EditorState* state, int ch);
void insert_char(EditorState* state, char c);
void delete_char(EditorState* state);
void new_line(EditorState* state);
void move_cursor(EditorState* state, int dx, int dy);
void find_text(EditorState* state);
void replace_text(EditorState* state);
void prompt_root_password(EditorState* state);
void render_screen(EditorState* state);
void show_status(EditorState* state, const char* message);
void show_status_left(EditorState* state, const char* message);
void count_stats(EditorState* state);
void toggle_line_numbers(EditorState* state);
void toggle_word_wrap(EditorState* state);
void cut_text(EditorState* state);
void copy_text(EditorState* state);
void paste_text(EditorState* state);
void select_all(EditorState* state);
void select_current_word(EditorState* state);
void select_current_line(EditorState* state);
void copy_selected_text(EditorState* state);
void clear_selection(EditorState* state);
void start_selection(EditorState* state);
void extend_selection(EditorState* state);
int has_selection(EditorState* state);
char* get_selected_text(EditorState* state);
void delete_selected_text(EditorState* state);
void copy_to_system_clipboard(const char* text);
void toggle_help(EditorState* state);
void render_help_screen(EditorState* state);
void jump_to_line(EditorState* state);

void init_syntax_highlighting(EditorState* state);
void update_syntax_highlighting(EditorState* state);
void detect_file_type(EditorState* state);
void load_c_keywords(EditorState* state);
void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width, int horizontal_scroll_offset);
void highlight_line_segment(EditorState* state, int line_num, int screen_row, int line_num_width, int start_col, int max_len);
int is_keyword(const char* word, EditorState* state);
int is_number(const char* token);
void free_syntax_data(EditorState* state);
int load_syntax_json(EditorState* state);
int parse_json_token_colors(const char* json_content, EditorState* state);
int hex_to_color_pair(const char* hex_color);
int match_scope(const char* token_scope, const char* rule_scope);
int get_dynamic_color(EditorState* state, const char* scope);
const char* get_dynamic_font_style(EditorState* state, const char* scope);
void save_original_content(EditorState* state);
void free_original_content(EditorState* state);
int content_matches_original(EditorState* state);
void update_dirty_status(EditorState* state);


int load_plugin(EditorState* state, const char* plugin_path);
void unload_plugin(EditorState* state, int plugin_index);
void unload_all_plugins(EditorState* state);
void list_plugins(EditorState* state);
int find_plugin_by_name(EditorState* state, const char* name);




#endif
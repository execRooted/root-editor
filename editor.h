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
#define MAX_UNDO_STATES 100


typedef struct {
    char** lines;
    int line_count;
    int cursor_x, cursor_y;
    int scroll_offset;
} UndoState;

typedef struct {
    char** lines;
    int line_count;
    int cursor_x, cursor_y;
    int scroll_offset;
    int undo_count;
    int redo_count;
    char filename[256];
    int dirty;
    int line_numbers;
    int word_wrap;
    int tab_size;
    char* clipboard;
    int select_mode;
    int select_start_x, select_start_y;
    int select_end_x, select_end_y;
    UndoState undo_buffer[MAX_UNDO_STATES];
    UndoState redo_buffer[MAX_UNDO_STATES];
    int show_help;
    int needs_sudo;
    int autosave_enabled;
    int autosave_interval;
    time_t last_save_time;
    int key_states[256];
    time_t key_timestamps[256];
    int syntax_enabled;
    int file_type;
    char** keywords;
    int keyword_count;
} EditorState;


void init_editor(EditorState* state);


void load_file(EditorState* state, const char* filename);
void save_file(EditorState* state);
void save_file_with_sudo(EditorState* state);
void prompt_filename(EditorState* state);
void autosave_check(EditorState* state);
void toggle_autosave(EditorState* state);
void toggle_syntax_highlighting(EditorState* state);

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
void copy_selected_text(EditorState* state);
void clear_selection(EditorState* state);
int has_selection(EditorState* state);
char* get_selected_text(EditorState* state);
void delete_selected_text(EditorState* state);


void undo(EditorState* state);
void redo(EditorState* state);

void save_undo_state(EditorState* state);
void copy_to_system_clipboard(const char* text);
void toggle_help(EditorState* state);
void render_help_screen(EditorState* state);


void jump_to_line(EditorState* state);

void init_syntax_highlighting(EditorState* state);
void detect_file_type(EditorState* state);
void load_c_keywords(EditorState* state);
void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width);
int is_keyword(const char* word, EditorState* state);
int is_number(const char* token);
void free_syntax_data(EditorState* state);
void detect_syntax_errors(EditorState* state);
void highlight_syntax_errors(EditorState* state, int line_num, int screen_row, int line_num_width);

#endif
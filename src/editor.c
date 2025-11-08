#include "editor.h"
#include <string.h>

#define FILE_TYPE_PLAIN   0

void init_editor(EditorState* state)
{
    state -> lines = (char ** ) malloc(MAX_LINES * sizeof(char * ));
    state -> lines[0] = (char * ) malloc(MAX_LINE_LENGTH);
    state -> lines[0][0] = '\0';
    state -> line_count = 1;
    state -> cursor_x = 0;
    state -> cursor_y = 0;
    state -> scroll_offset = 0;
    state -> horizontal_scroll_offset = 0;
    state -> filename[0] = '\0';
    state -> dirty = 0;
    state -> line_numbers = 1;
    state -> word_wrap = 1;
    state -> tab_size = TAB_SIZE;
    state -> select_mode = 0;
    state -> select_start_x = 0;
    state -> select_start_y = 0;
    state -> select_end_x = 0;
    state -> select_end_y = 0;
    state -> show_help = 0;
    memset(state -> key_states, 0, sizeof(state -> key_states));
    memset(state -> key_timestamps, 0, sizeof(state -> key_timestamps));
    state -> syntax_enabled = 1;  
    state -> syntax_display_enabled = 1;
    state -> auto_tabbing_enabled = 1;    
    state -> file_type = 0;
    state -> keywords = NULL;
    state -> keyword_count = 0;

    
    state -> theme_id = 3;
    strncpy(state -> theme_name, "White Selection", sizeof(state -> theme_name) - 1);
    state -> theme_name[sizeof(state -> theme_name) - 1] = '\0';

    

    
    state -> auto_complete_enabled = 1;
    state -> sticky_cursor_enabled = 1;

    
    state -> json_rules_count = 0;
    state -> json_loaded = 0;
    memset(state -> json_scopes, 0, sizeof(state -> json_scopes));
    memset(state -> json_colors, 0, sizeof(state -> json_colors));

    
    state -> original_lines = NULL;
    state -> original_line_count = 0;

    
    state -> last_input_time = time(NULL);
    state -> rapid_input_mode = 0;
    state -> char_select_mode = 0;
    state -> editor_mode = 0; 

    
    state -> plugin_count = 0;
    memset(state -> plugins, 0, sizeof(state -> plugins));

    FILE *f = fopen("/tmp/editor_debug.log", "w");
    if (f) 
    {
        fprintf(f, "Editor initialization\n");
        fprintf(f, "Loading syntax configuration...\n");
        int result = load_syntax_json(state);
        fprintf(f, "Syntax config loaded: %d\n", result);
        fclose(f);
    }

    init_syntax_highlighting(state);
}
void toggle_dark_mode(EditorState* state)
{
    cycle_theme(state);
}
void insert_char(EditorState* state, char c)
{
    if (!state || !state -> lines || state -> cursor_y >= state -> line_count ||
        state -> cursor_y < 0 || !state -> lines[state -> cursor_y]) {
        return;
    }
    char * line = state -> lines[state -> cursor_y];
    int len = strlen(line);
    if (state -> cursor_x < 0) state -> cursor_x = 0;

    

    

    
    if (len < MAX_LINE_LENGTH - 1) {
        if (state -> cursor_x > len) {

            for (int i = len; i < state -> cursor_x; i++) {
                line[i] = ' ';
            }
            line[state -> cursor_x] = c;
            line[state -> cursor_x + 1] = '\0';
        } else {
            memmove( & line[state -> cursor_x + 1], & line[state -> cursor_x], len - state -> cursor_x + 1);
            line[state -> cursor_x] = c;
        }
        state -> cursor_x++;
        update_dirty_status(state);
    }

    // Auto-complete logic
    if (state->auto_complete_enabled) {
        if (c == '(' && len < MAX_LINE_LENGTH - 1) {
            if (state -> cursor_x > len) {
                for (int i = len; i < state -> cursor_x; i++) {
                    line[i] = ' ';
                }
                line[state -> cursor_x] = '(';
                line[state -> cursor_x + 1] = ')';
                line[state -> cursor_x + 2] = '\0';
            } else {
                memmove( & line[state -> cursor_x + 2], & line[state -> cursor_x], len - state -> cursor_x + 1);
                line[state -> cursor_x] = '(';
                line[state -> cursor_x + 1] = ')';
            }
            state -> cursor_x++;
            update_dirty_status(state);
        } else if (c == '{' && len < MAX_LINE_LENGTH - 1) {
            if (state -> cursor_x > len) {
                for (int i = len; i < state -> cursor_x; i++) {
                    line[i] = ' ';
                }
                line[state -> cursor_x] = '{';
                line[state -> cursor_x + 1] = '}';
                line[state -> cursor_x + 2] = '\0';
            } else {
                memmove( & line[state -> cursor_x + 2], & line[state -> cursor_x], len - state -> cursor_x + 1);
                line[state -> cursor_x] = '{';
                line[state -> cursor_x + 1] = '}';
            }
            state -> cursor_x++;
            update_dirty_status(state);
        } else if (c == '[' && len < MAX_LINE_LENGTH - 1) {
            if (state -> cursor_x > len) {
                for (int i = len; i < state -> cursor_x; i++) {
                    line[i] = ' ';
                }
                line[state -> cursor_x] = '[';
                line[state -> cursor_x + 1] = ']';
                line[state -> cursor_x + 2] = '\0';
            } else {
                memmove( & line[state -> cursor_x + 2], & line[state -> cursor_x], len - state -> cursor_x + 1);
                line[state -> cursor_x] = '[';
                line[state -> cursor_x + 1] = ']';
            }
            state -> cursor_x++;
            update_dirty_status(state);
        } else if (c == '"' && len < MAX_LINE_LENGTH - 1) {
            if (state -> cursor_x > len) {
                for (int i = len; i < state -> cursor_x; i++) {
                    line[i] = ' ';
                }
                line[state -> cursor_x] = '"';
                line[state -> cursor_x + 1] = '"';
                line[state -> cursor_x + 2] = '\0';
            } else {
                memmove( & line[state -> cursor_x + 2], & line[state -> cursor_x], len - state -> cursor_x + 1);
                line[state -> cursor_x] = '"';
                line[state -> cursor_x + 1] = '"';
            }
            state -> cursor_x++;
            update_dirty_status(state);
        } else if (c == '\'' && len < MAX_LINE_LENGTH - 1) {
            if (state -> cursor_x > len) {
                for (int i = len; i < state -> cursor_x; i++) {
                    line[i] = ' ';
                }
                line[state -> cursor_x] = '\'';
                line[state -> cursor_x + 1] = '\'';
                line[state -> cursor_x + 2] = '\0';
            } else {
                memmove( & line[state -> cursor_x + 2], & line[state -> cursor_x], len - state -> cursor_x + 1);
                line[state -> cursor_x] = '\'';
                line[state -> cursor_x + 1] = '\'';
            }
            state -> cursor_x++;
            update_dirty_status(state);
        } else if (c == ')' && state -> cursor_x < len && line[state -> cursor_x] == ')') {
            state -> cursor_x++;
        } else if (c == '}' && state -> cursor_x < len && line[state -> cursor_x] == '}') {
            state -> cursor_x++;
        } else if (c == ']' && state -> cursor_x < len && line[state -> cursor_x] == ']') {
            state -> cursor_x++;
        } else if (c == '"' && state -> cursor_x < len && line[state -> cursor_x] == '"') {
            state -> cursor_x++;
        } else if (c == '\'' && state -> cursor_x < len && line[state -> cursor_x] == '\'') {
            state -> cursor_x++;
        }
    }
}
void delete_char(EditorState* state)
{
    if (!state || !state -> lines || state -> cursor_y >= state -> line_count ||
        state -> cursor_y < 0 || !state -> lines[state -> cursor_y]) {
        return;
    }

    char * line = state -> lines[state -> cursor_y];
    int len = strlen(line);

    
    
    if (state -> cursor_x > 0) {
        
        int is_deleting_indentation = 1;
        for (int i = 0; i < state->cursor_x; i++) {
            if (line[i] != ' ') {
                is_deleting_indentation = 0;
                break;
            }
        }

        if (is_deleting_indentation && state->cursor_x >= state->tab_size) {
            
            int spaces_to_delete = state->tab_size;
            if (state->cursor_x < spaces_to_delete) {
                spaces_to_delete = state->cursor_x;
            }
            memmove(&line[state->cursor_x - spaces_to_delete], &line[state->cursor_x], len - state->cursor_x + 1);
            state->cursor_x -= spaces_to_delete;
        } else {
            
            if (state -> cursor_x > len) state -> cursor_x = len;
            memmove(&line[state -> cursor_x - 1], &line[state -> cursor_x], len - state -> cursor_x + 1);
            state -> cursor_x--;
        }
        update_dirty_status(state);
        return;
    }

    
    if (state -> cursor_x == 0 && state -> cursor_y > 0) {
        
        int is_empty_or_whitespace = 1;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] != ' ' && line[i] != '\t') {
                is_empty_or_whitespace = 0;
                break;
            }
        }

        char * prev = state -> lines[state -> cursor_y - 1];
        int prev_len = strlen(prev);

        
        if (is_empty_or_whitespace) {
            
            free(state -> lines[state -> cursor_y]);
            for (int i = state -> cursor_y; i < state -> line_count - 1; i++) {
                state -> lines[i] = state -> lines[i + 1];
            }
            state -> line_count--;

            
            state -> cursor_y--;
            state -> cursor_x = prev_len;

            move_cursor(state, 0, 0);
            update_dirty_status(state);
            return;
        }

        
        if (prev_len + len >= MAX_LINE_LENGTH) {
            show_status(state, "Join aborted: maximum line length exceeded");
            return;
        }

        
        memcpy(&prev[prev_len], line, len + 1); 

        
        free(state -> lines[state -> cursor_y]);
        for (int i = state -> cursor_y; i < state -> line_count - 1; i++) {
            state -> lines[i] = state -> lines[i + 1];
        }
        state -> line_count--;

        
        state -> cursor_y--;
        state -> cursor_x = prev_len;

        move_cursor(state, 0, 0);
        update_dirty_status(state);
    }
}
void new_line(EditorState* state)
{
    if (state -> line_count >= MAX_LINES) {
        show_status(state, "Error: Maximum line count reached (1,000,000 lines)");
        return;
    }
    char * line = state -> lines[state -> cursor_y];

    // Check if cursor is between a pair and split it
    int split_pair = 0;
    char closing_char = 0;
    int after_pos = 0;
    if (state->cursor_x > 0 && state->cursor_x < (int)strlen(line)) {
        char before = line[state->cursor_x - 1];
        after_pos = state->cursor_x;
        while (after_pos < (int)strlen(line) && (line[after_pos] == ' ' || line[after_pos] == '\t')) after_pos++;
        char after = (after_pos < (int)strlen(line)) ? line[after_pos] : 0;
        if ((before == '{' && after == '}') ||
            (before == '(' && after == ')') ||
            (before == '[' && after == ']')) {
            split_pair = 1;
            closing_char = after;
            // Remove from cursor_x to after_pos + 1
            int remove_start = state->cursor_x;
            int remove_end = after_pos + 1;
            memmove(&line[remove_start], &line[remove_end], strlen(line) - remove_end + 1);
        }
    }

    int base_indent = 0;

    if (state->auto_tabbing_enabled) {

        while (base_indent < strlen(line) && (line[base_indent] == ' ' || line[base_indent] == '\t')) {
            base_indent++;
        }
    }


    int extra_indent = 0;
    if (state->cursor_x > 0) {
        char before = line[state->cursor_x - 1];
        if (before == '}' && base_indent >= state->tab_size) {
            base_indent -= state->tab_size;
        }
    }

    
    int brace_level = 0;
    int inside_function_braces = 0;

    
    int line_has_closing_brace = 0;
    for (int i = 0; i < state->cursor_x && line[i] != '\0'; i++) {
        if (line[i] == '}') {
            line_has_closing_brace = 1;
            break;
        }
    }

    
    for (int l = 0; l < state->cursor_y; l++) {
        const char* check_line = state->lines[l];
        if (!check_line) continue;

        for (int i = 0; check_line[i] != '\0'; i++) {
            if (check_line[i] == '{') {
                brace_level++;
                inside_function_braces = 1;
            } else if (check_line[i] == '}') {
                brace_level--;
                if (brace_level <= 0) {
                    inside_function_braces = 0;
                }
            }
        }
    }

    
    for (int i = 0; i < state->cursor_x && line[i] != '\0'; i++) {
        if (line[i] == '{') {
            brace_level++;
            inside_function_braces = 1;
        } else if (line[i] == '}') {
            brace_level--;
            if (brace_level <= 0) {
                inside_function_braces = 0;
            }
        }
    }

    
    
    
    

    if (split_pair) {
        // Create two new lines for pair splitting
        char * empty_line = (char * ) malloc(MAX_LINE_LENGTH);
        if (!empty_line) {
            show_status(state, "Memory allocation failed");
            return;
        }
        char * closing_line = (char * ) malloc(MAX_LINE_LENGTH);
        if (!closing_line) {
            free(empty_line);
            show_status(state, "Memory allocation failed");
            return;
        }

        // Empty line with indentation
        int indent_len = 0;
        for (int i = 0; i < base_indent && indent_len < MAX_LINE_LENGTH - 1; i++) {
            empty_line[indent_len++] = line[i];
        }
        for (int i = 0; i < extra_indent && indent_len < MAX_LINE_LENGTH - 1; i++) {
            empty_line[indent_len++] = ' ';
        }
        empty_line[indent_len] = '\0';

        // Closing line with same indentation + closing char
        indent_len = 0;
        for (int i = 0; i < base_indent && indent_len < MAX_LINE_LENGTH - 1; i++) {
            closing_line[indent_len++] = line[i];
        }
        for (int i = 0; i < extra_indent && indent_len < MAX_LINE_LENGTH - 1; i++) {
            closing_line[indent_len++] = ' ';
        }
        closing_line[indent_len++] = closing_char;
        closing_line[indent_len] = '\0';

        line[state -> cursor_x] = '\0';

        // Insert two lines
        for (int i = state -> line_count + 1; i > state -> cursor_y + 2; i--) {
            state -> lines[i] = state -> lines[i - 2];
        }
        state -> lines[state -> cursor_y + 1] = empty_line;
        state -> lines[state -> cursor_y + 2] = closing_line;
        state -> line_count += 2;
        state -> cursor_y++;
        state -> cursor_x = indent_len - 1; // Position at end of indentation on empty line
        move_cursor(state, 0, 0);
        napms(10);
        update_dirty_status(state);
    } else {
        // Original logic for non-pair splitting
        char * new_line = (char * ) malloc(MAX_LINE_LENGTH);
        if (!new_line) {
            show_status(state, "Memory allocation failed");
            return;
        }

        int indent_len = 0;
        for (int i = 0; i < base_indent && indent_len < MAX_LINE_LENGTH - 1; i++) {
            new_line[indent_len++] = line[i];
        }
        for (int i = 0; i < extra_indent && indent_len < MAX_LINE_LENGTH - 1; i++) {
            new_line[indent_len++] = ' ';
        }

        const char *tail = &line[state->cursor_x];
        int tail_len = strlen(tail);
        if (indent_len + tail_len >= MAX_LINE_LENGTH) {
            tail_len = MAX_LINE_LENGTH - 1 - indent_len;
        }
        memcpy(new_line + indent_len, tail, tail_len);
        new_line[indent_len + tail_len] = '\0';

        line[state -> cursor_x] = '\0';

        for (int i = state -> line_count; i > state -> cursor_y + 1; i--) {
            state -> lines[i] = state -> lines[i - 1];
        }
        state -> lines[state -> cursor_y + 1] = new_line;
        state -> line_count++;
        state -> cursor_y++;
        state -> cursor_x = indent_len;
        move_cursor(state, 0, 0);
        napms(10);
        update_dirty_status(state);
    }
}
void move_cursor(EditorState* state, int dx, int dy)
{
    if (!state || !state -> lines || state -> line_count <= 0) {
        return;
    }

    int max_y_screen, max_x_screen;
    getmaxyx(stdscr, max_y_screen, max_x_screen);
    int avail_w = max_x_screen - 6 - 1;
    if (avail_w < 1) avail_w = 1;

    int new_x = state -> cursor_x + dx;
    int new_y = state -> cursor_y + dy;
    if (new_y < 0) new_y = 0;
    if (new_y >= state -> line_count) new_y = state -> line_count - 1;
    if (!state -> lines[new_y]) return;
    int max_x = strlen(state -> lines[new_y]);

    
    if (dy != 0 && !state->select_mode) {

        if (state->sticky_cursor_enabled) {
            
            if (new_x >= max_x) new_x = max_x;
            if (new_x < 0) new_x = 0;
        } else {
            
            if (dy < 0 && new_y >= 0 && new_y < state -> line_count && strlen(state -> lines[new_y]) > 0) {
                new_x = strlen(state -> lines[new_y]);
            }

            else if (dy > 0 && new_y >= 0 && new_y < state -> line_count && strlen(state -> lines[new_y]) > 0) {
                new_x = strlen(state -> lines[new_y]);
            }

            else if (dy > 0 && state -> cursor_y >= 0 && state -> cursor_y < state -> line_count && strlen(state -> lines[state -> cursor_y]) == 0) {
                new_x = 0;
            }

            else {
                if (new_x >= max_x) new_x = max_x;
                if (new_x < 0) new_x = 0;
            }
        }

        if (dy != 0 && new_y >= 0 && new_y < state -> line_count) {
            int target_line_len = strlen(state -> lines[new_y]);
            if (target_line_len < state -> horizontal_scroll_offset) {
                state -> horizontal_scroll_offset = 0;
            }
        }
    }

    if (new_x < 0) {
        if (state -> cursor_x == 0) {
            state -> horizontal_scroll_offset--;
            state -> cursor_x = 0;
            state -> cursor_y = new_y;
        } else if (new_y > 0 && state -> lines[new_y - 1]) {
            state -> cursor_y = new_y - 1;
            state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
            if (state -> cursor_x >= max_x && max_x > 0) state -> cursor_x = max_x - 1;
        } else {
            state -> cursor_x = 0;
            state -> cursor_y = new_y;
        }
    } else if (new_x >= max_x) {
        new_x = max_x;
        if (!state -> select_mode || state -> char_select_mode) {
            state -> horizontal_scroll_offset = new_x - avail_w + 1;
            if (state->horizontal_scroll_offset < 0) state->horizontal_scroll_offset = 0;
            int line_len_cur = 0;
            if (new_y >= 0 && new_y < state->line_count && state->lines[new_y]) {
                line_len_cur = (int)strlen(state->lines[new_y]);
            }
            int max_off = (line_len_cur > avail_w) ? (line_len_cur - avail_w) : 0;
            if (state->horizontal_scroll_offset > max_off) state->horizontal_scroll_offset = max_off;
        }
        state -> cursor_x = new_x;
        state -> cursor_y = new_y;
    } else {
        state -> cursor_x = new_x;
        state -> cursor_y = new_y;
    }

    int max_y = max_y_screen;
    if (state -> cursor_y <= state -> scroll_offset - 1) {
        state -> scroll_offset = state -> cursor_y - 1;
        if (state -> scroll_offset < 0) state -> scroll_offset = 0;
        state -> cursor_y = state -> scroll_offset;
    } else if (state -> cursor_y >= state -> scroll_offset + max_y - 5) {
        state -> scroll_offset = state -> cursor_y - max_y + 6;
    }
    if (state -> scroll_offset < 0) state -> scroll_offset = 0;
    if (state -> scroll_offset >= state -> line_count) state -> scroll_offset = state -> line_count - 1;
    if (state -> cursor_y < 0) state -> cursor_y = 0;
    if (state -> cursor_y >= state -> line_count) state -> cursor_y = state -> line_count - 1;

    
    if (state->char_select_mode || !state->select_mode) {
        if (state -> cursor_x < state -> horizontal_scroll_offset) {
            state -> horizontal_scroll_offset = state -> cursor_x;
        } else if (state -> cursor_x >= state -> horizontal_scroll_offset + avail_w) {
            state -> horizontal_scroll_offset = state -> cursor_x - avail_w + 1;
        }
        if (state->horizontal_scroll_offset < 0) state->horizontal_scroll_offset = 0;
        int line_len_cur2 = 0;
        if (state->cursor_y >= 0 && state->cursor_y < state->line_count && state->lines[state->cursor_y]) {
            line_len_cur2 = (int)strlen(state->lines[state->cursor_y]);
        }
        int max_off2 = (line_len_cur2 > avail_w) ? (line_len_cur2 - avail_w) : 0;
        if (state->horizontal_scroll_offset > max_off2) state->horizontal_scroll_offset = max_off2;
    }
}
void show_status(EditorState* state,
    const char* message)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(max_y - 1, 0, "%*s", max_x, "");
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(max_y - 1, 0, "%s", message);
    attroff(COLOR_PAIR(1) | A_BOLD);
    refresh();
    napms(500);
}
void show_status_left(EditorState* state,
    const char* message)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    mvprintw(max_y - 1, 0, "%*s", max_x, "");
    attron(COLOR_PAIR(1));
    mvprintw(max_y - 1, 0, "%s", message);
    attroff(COLOR_PAIR(1));
    refresh();
    napms(500);
}
void count_stats(EditorState* state)
{
    int words = 0;
    int characters = 0;
    for (int i = 0; i < state -> line_count; i++) {
        characters += strlen(state -> lines[i]);
        char * line = state -> lines[i];
        int in_word = 0;
        for (int j = 0; line[j] != '\0'; j++) {
            if (isspace(line[j])) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;
            }
        }
    }
    char msg[256];
    snprintf(msg, sizeof(msg), "Words: %d, Characters: %d", words, characters);
}

static char* internal_clipboard = NULL;
static char* my_strdup(const char* s)
{
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* dup = (char*)malloc(len);
    if (dup) memcpy(dup, s, len);
    return dup;
}
void set_internal_clipboard(const char* text)
{
    if (internal_clipboard) {
        free(internal_clipboard);
    }
    internal_clipboard = text ? my_strdup(text) : NULL;
}

void copy_to_system_clipboard(const char* text)
{
    if (!text || strlen(text) == 0) return;

    
    set_internal_clipboard(text);

    
    char temp_file[] = "/tmp/kilo_editor_clipboard_temp.txt";
    FILE * fp = fopen(temp_file, "w");
    if (!fp) return;
    fprintf(fp, "%s", text);
    fclose(fp);

    
    int is_wayland = getenv("WAYLAND_DISPLAY") != NULL;
    if (is_wayland) {
        system("cat '/tmp/kilo_editor_clipboard_temp.txt' | wl-copy 2>/dev/null &");
    } else {
        system("cat '/tmp/kilo_editor_clipboard_temp.txt' | xclip -selection clipboard 2>/dev/null &");
        system("cat '/tmp/kilo_editor_clipboard_temp.txt' | xclip -selection primary 2>/dev/null &");
    }

    
    system("rm -f '/tmp/kilo_editor_clipboard_temp.txt' 2>/dev/null &");
}
void toggle_help(EditorState* state)
{
    state -> show_help = !state -> show_help;
}
void jump_to_line(EditorState* state)
{
    echo();
    curs_set(1);
    attron(COLOR_PAIR(1));
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    char prompt[64];
    int prompt_len = snprintf(prompt, sizeof(prompt), "Jump to line (1-%d): ", state -> line_count);
    mvprintw(max_y - 2, 0, "%s", prompt);
    clrtoeol();
    refresh();
    char input[32] = {
        0
    };
    int input_pos = 0;
    int cursor_x = prompt_len;
    move(max_y - 2, cursor_x);
    refresh();
    while (1) {
        int ch = getch();
        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        } else if (ch == 27) {
            input[0] = '\0';
            break;
        } else if (ch == KEY_LEFT) {
            if (cursor_x > prompt_len) {
                cursor_x--;
                input_pos = cursor_x - prompt_len;
            }
            move(max_y - 2, cursor_x);
            refresh();
        } else if (ch == KEY_RIGHT) {
            if (cursor_x < prompt_len + (int) strlen(input)) {
                cursor_x++;
                input_pos = cursor_x - prompt_len;
            }
            move(max_y - 2, cursor_x);
            refresh();
        } else if (ch == KEY_HOME) {
            cursor_x = prompt_len;
            input_pos = 0;
            move(max_y - 2, cursor_x);
            refresh();
        } else if (ch == KEY_END) {
            cursor_x = prompt_len + input_pos;
            move(max_y - 2, cursor_x);
            refresh();
        } else if (ch == KEY_DC) {
            if (input_pos < strlen(input)) {
                memmove( & input[input_pos], & input[input_pos + 1], strlen(input) - input_pos);
                mvprintw(max_y - 2, cursor_x, "%s ", & input[input_pos]);
                refresh();
            }
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt_len) {
            int delete_pos = cursor_x - prompt_len - 1;
            if (delete_pos >= 0) {
                memmove( & input[delete_pos], & input[delete_pos + 1], strlen(input) - delete_pos + 1);
                cursor_x--;
                input_pos = cursor_x - prompt_len;
                mvprintw(max_y - 2, 0, "%s%s ", prompt, input);
                move(max_y - 2, cursor_x);
                refresh();
            }
        } else if (isprint(ch) && input_pos < sizeof(input) - 1 && cursor_x < max_x - 1) {
            input[input_pos++] = ch;
            mvprintw(max_y - 2, cursor_x++, "%c", ch);
            refresh();
        }
    }
    noecho();
    curs_set(1);
    attroff(COLOR_PAIR(1));
    if (strlen(input) == 0) {
        return;
    }
    int line_num = atoi(input);
    if (line_num >= 1 && line_num <= state -> line_count) {
        state -> cursor_y = line_num - 1;
        state -> cursor_x = 0;
        if (state -> cursor_y < state -> scroll_offset) {
            state -> scroll_offset = state -> cursor_y - 1;
            if (state -> scroll_offset < 0) state -> scroll_offset = 0;
        } else if (state -> cursor_y >= state -> scroll_offset + max_y - 3) {
            state -> scroll_offset = state -> cursor_y - max_y + 4;
        }
    } else {
        show_status(state, "Line not found or not created");
    }
}
void render_help_screen(EditorState* state)
{
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    
    int popup_width = 42;
    int popup_height = 24;
    int popup_x = max_x - popup_width - 1;
    int popup_y = 1;

    
    if (popup_x < 0) popup_x = 0;
    if (popup_y + popup_height > max_y) popup_height = max_y - popup_y - 1;
    if (popup_width > max_x) popup_width = max_x - 2;

    
    for (int y = popup_y; y < popup_y + popup_height; y++) {
        for (int x = popup_x; x < popup_x + popup_width; x++) {
            mvaddch(y, x, ' ');
        }
    }

    
    attron(COLOR_PAIR(1));
    for (int i = 0; i < popup_width; i++) {
        mvprintw(popup_y, popup_x + i, "-");
        mvprintw(popup_y + popup_height - 1, popup_x + i, "-");
    }
    for (int i = 0; i < popup_height; i++) {
        mvprintw(popup_y + i, popup_x, "|");
        mvprintw(popup_y + i, popup_x + popup_width - 1, "|");
    }
    mvprintw(popup_y, popup_x, "+");
    mvprintw(popup_y, popup_x + popup_width - 1, "+");
    mvprintw(popup_y + popup_height - 1, popup_x, "+");
    mvprintw(popup_y + popup_height - 1, popup_x + popup_width - 1, "+");

    
    mvprintw(popup_y + 1, popup_x + 2, "HELP (F1)");
    attroff(COLOR_PAIR(1));

    
    int line = popup_y + 3;
    int col1 = popup_x + 2;
    int col2 = popup_x + 24; 

    mvprintw(line++, col1, "Ctrl+Q  Quit");
    mvprintw(line++, col1, "Ctrl+S  Save");
    mvprintw(line++, col1, "Ctrl+O  Open");
    mvprintw(line++, col1, "Ctrl+V  Paste");
    mvprintw(line++, col1, "Ctrl+F  Find");
    mvprintw(line++, col1, "Ctrl+R  Replace");
    mvprintw(line++, col1, "Ctrl+L  Jump to line");
    mvprintw(line++, col1, "Ctrl+T  Auto Tab");
    mvprintw(line++, col1, "Ctrl+K  Auto Complete");
    mvprintw(line++, col1, "Ctrl+H  Help");
    mvprintw(line++, col1, "Ctrl+W Select mode");
    mvprintw(line++, col1, "Ctrl+A  Select all");
    mvprintw(line++, col1, "Ctrl+X  Cut");
    mvprintw(line++, col1, "Ctrl+C  Copy");
    mvprintw(line++, col1, "Esc then Enter Exit select mode");

    line = popup_y + 3;
    mvprintw(line++, col2, "F1   Help");
    mvprintw(line++, col2, "F2   Find");
    mvprintw(line++, col2, "F3   Replace");
    mvprintw(line++, col2, "F4   Cut");
    mvprintw(line++, col2, "F5   Copy");
    mvprintw(line++, col2, "F6   Paste");
    mvprintw(line++, col2, "F7   Syntax HL");
    mvprintw(line++, col2, "F8   Sticky Cursor");
    mvprintw(line++, col2, "F9   Autocomplete");



    
    attron(COLOR_PAIR(1));
    mvprintw(popup_y + popup_height - 2, popup_x + 2, "Press any key to close");
    attroff(COLOR_PAIR(1));

    refresh();
    int ch = getch();
    state -> show_help = 0;

    
    render_screen(state);
}
void toggle_syntax_highlighting(EditorState* state)
{
    state -> syntax_enabled = !state -> syntax_enabled;
    if (state -> syntax_enabled) {
        init_syntax_highlighting(state);
    } else {
        free_syntax_data(state);
    }
    save_config(state);
}

void toggle_word_wrap(EditorState* state)
{
    state -> word_wrap = !state -> word_wrap;
    save_config(state);
}

void toggle_syntax_display(EditorState* state)
{
    state -> syntax_display_enabled = !state -> syntax_display_enabled;
    save_config(state);
}

void enter_selecting_mode(EditorState* state)
{
    state->editor_mode = 1; 
    state->syntax_display_enabled = 0; 
    clear_selection(state);
    start_selection(state);
}

void exit_selecting_mode(EditorState* state)
{
    state->editor_mode = 0; 
    state->syntax_display_enabled = 1; 
    clear_selection(state);
}

void toggle_auto_tabbing(EditorState* state)
{
    state -> auto_tabbing_enabled = !state -> auto_tabbing_enabled;
    save_config(state);
}

void toggle_auto_complete(EditorState* state)
{
    state -> auto_complete_enabled = !state -> auto_complete_enabled;
    save_config(state);
}

void toggle_comment_complete(EditorState* state)
{
    state -> auto_complete_enabled = !state -> auto_complete_enabled;
    save_config(state);
}

void toggle_sticky_cursor(EditorState* state)
{
    state -> sticky_cursor_enabled = !state -> sticky_cursor_enabled;
    save_config(state);
}
int can_process_key(EditorState* state, int key_code)
{
    if (key_code < 0 || key_code >= 256) {
        return 1;
    }
    return (state -> key_states[key_code] == 0 || state -> key_states[key_code] == 2);
}
void mark_key_processed(EditorState* state, int key_code)
{
    if (key_code >= 0 && key_code < 256) {
        state -> key_states[key_code] = 2;
        state -> key_timestamps[key_code] = time(NULL);
    }
}
void reset_key_states(EditorState* state)
{
    memset(state -> key_states, 0, sizeof(state -> key_states));
    memset(state -> key_timestamps, 0, sizeof(state -> key_timestamps));
}
void save_original_content(EditorState* state)
{
    
    free_original_content(state);

    
    state->original_lines = (char**)malloc(state->line_count * sizeof(char*));
    if (!state->original_lines) return;

    state->original_line_count = state->line_count;

    
    for (int i = 0; i < state->line_count; i++) {
        state->original_lines[i] = (char*)malloc(MAX_LINE_LENGTH);
        if (!state->original_lines[i]) {
            free_original_content(state);
            return;
        }
        strcpy(state->original_lines[i], state->lines[i]);
    }
}
void free_original_content(EditorState* state)
{
    if (state->original_lines) {
        for (int i = 0; i < state->original_line_count; i++) {
            if (state->original_lines[i]) {
                free(state->original_lines[i]);
            }
        }
        free(state->original_lines);
        state->original_lines = NULL;
    }
    state->original_line_count = 0;
}
int content_matches_original(EditorState* state)
{
    
    if (!state->original_lines || state->original_line_count == 0) {
        return 0;
    }

    
    if (state->line_count != state->original_line_count) {
        return 0;
    }

    
    for (int i = 0; i < state->line_count; i++) {
        if (strcmp(state->lines[i], state->original_lines[i]) != 0) {
            return 0; 
        }
    }

    return 1; 
}
void update_dirty_status(EditorState* state)
{
    
    if (state->filename[0] == '\0') {
        
        int has_content = 0;
        for (int i = 0; i < state->line_count; i++) {
            if (strlen(state->lines[i]) > 0) {
                has_content = 1;
                break;
            }
        }
        state->dirty = has_content;
        return;
    }

    state->dirty = !content_matches_original(state);
}
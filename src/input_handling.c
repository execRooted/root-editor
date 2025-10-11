#define _POSIX_C_SOURCE 200809L
#include "editor.h"
#include "plugin.h"
#include <stdlib.h>
#include <stdio.h>

void render_screen(EditorState * state);

static int try_handle_bracketed_paste(EditorState * state, int first_ch);
static void paste_from_string(EditorState * state, const char * clipboard_content);

void delete_current_line(EditorState * state);
void handle_mouse_event(EditorState * state);
static char* internal_clipboard = NULL;
extern char* internal_clipboard;

static int dragging = 0;
static int selection_started_with_shift = 0;

char * get_system_clipboard() {
    
    if (internal_clipboard && *internal_clipboard) {
        return strdup(internal_clipboard);
    }

    
    FILE * fp;
    int is_wayland = getenv("WAYLAND_DISPLAY") != NULL;

    if (is_wayland) {
        fp = popen("wl-paste 2>/dev/null", "r");
        if (fp) {
            char buffer[1024 * 1024];
            size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
            pclose(fp);
            if (len > 0) {
                buffer[len] = '\0';
                return strdup(buffer);
            }
        }
    }

    fp = popen("xclip -selection clipboard -o 2>/dev/null", "r");
    if (fp) {
        char buffer[1024 * 1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        pclose(fp);
        if (len > 0) {
            buffer[len] = '\0';
            return strdup(buffer);
        }
    }

    fp = popen("xclip -selection primary -o 2>/dev/null", "r");
    if (fp) {
        char buffer[1024 * 1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        pclose(fp);
        if (len > 0) {
            buffer[len] = '\0';
            return strdup(buffer);
        }
    }

    if (!is_wayland) {
        fp = popen("wl-paste 2>/dev/null", "r");
        if (fp) {
            char buffer[1024 * 1024];
            size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
            pclose(fp);
            if (len > 0) {
                buffer[len] = '\0';
                return strdup(buffer);
            }
        }
    }

    fp = popen("xsel --clipboard --output 2>/dev/null", "r");
    if (fp) {
        char buffer[1024 * 1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        pclose(fp);
        if (len > 0) {
            buffer[len] = '\0';
            return strdup(buffer);
        }
    }

    fp = popen("xsel --primary --output 2>/dev/null", "r");
    if (fp) {
        char buffer[1024 * 1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        pclose(fp);
        if (len > 0) {
            buffer[len] = '\0';
            return strdup(buffer);
        }
    }

    fp = popen("termux-clipboard-get 2>/dev/null", "r");
    if (fp) {
        char buffer[1024 * 1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        pclose(fp);
        if (len > 0) {
            buffer[len] = '\0';
            return strdup(buffer);
        }
    }

    fp = popen("pbpaste 2>/dev/null", "r");
    if (fp) {
        char buffer[1024 * 1024];
        size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
        pclose(fp);
        if (len > 0) {
            buffer[len] = '\0';
            return strdup(buffer);
        }
    }

    return NULL;
}
void handle_input(EditorState * state, int ch) {

        if (ch >= 32 && ch <= 126) {
                reset_key_states(state);
                insert_char(state, ch);
                move_cursor(state, 0, 0);
                return;
        }

        
        if (ch == 27) {
                if (try_handle_bracketed_paste(state, ch)) {
                        return;
                }
        }

        if (ch < 32 && ch != 27 && ch != '\n' && ch != '\t' && ch != KEY_BACKSPACE) {
                handle_ctrl_keys(state, ch);
                return;
        }

        switch (ch) {
        case KEY_UP:
                if (state->char_select_mode) {
                        move_cursor(state, 0, -1);
                        extend_selection(state);
                } else if (selection_started_with_shift) {
                        move_cursor(state, 0, -1);
                        extend_selection(state);
                } else {
                        move_cursor(state, 0, -1);
                        if (state->select_mode) {
                                extend_selection(state);
                        }
                }
                break;
        case KEY_DOWN:
                if (state->char_select_mode) {
                        move_cursor(state, 0, 1);
                        extend_selection(state);
                } else if (selection_started_with_shift) {
                        move_cursor(state, 0, 1);
                        extend_selection(state);
                } else {
                        move_cursor(state, 0, 1);
                        if (state->select_mode) {
                                extend_selection(state);
                        }
                }
                break;
        case KEY_LEFT:
                if (state->char_select_mode) {
                        move_cursor(state, -1, 0);
                        extend_selection(state);
                } else if (selection_started_with_shift) {
                        move_cursor(state, -1, 0);
                        extend_selection(state);
                } else {
                        move_cursor(state, -1, 0);
                        if (state->select_mode) {
                                extend_selection(state);
                        }
                }
                break;
        case KEY_RIGHT:
                if (state->char_select_mode) {
                        move_cursor(state, 1, 0);
                        extend_selection(state);
                } else if (selection_started_with_shift) {
                        move_cursor(state, 1, 0);
                        extend_selection(state);
                } else {
                        move_cursor(state, 1, 0);
                        if (state->select_mode) {
                                extend_selection(state);
                        }
                }
                break;
        case KEY_SLEFT:
                selection_started_with_shift = 1;
                if (!state->select_mode) start_selection(state);
                move_cursor(state, -1, 0);
                extend_selection(state);
                break;
        case KEY_SRIGHT:
                selection_started_with_shift = 1;
                if (!state->select_mode) start_selection(state);
                move_cursor(state, 1, 0);
                extend_selection(state);
                break;
        case KEY_HOME:

                state -> cursor_x = 0;
                state -> horizontal_scroll_offset = 0;
                if (state->select_mode || state->char_select_mode) {
                        extend_selection(state);
                }
                break;
        case KEY_END:

                if (state -> cursor_y < state -> line_count && state -> lines[state -> cursor_y]) {
                        state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                }
                move_cursor(state, 0, 0);
                if (state->select_mode || state->char_select_mode) {
                        extend_selection(state);
                }
                break;
        case KEY_BACKSPACE:
        case KEY_DC:
        case 127:
                if (state -> select_mode && has_selection(state)) {

                        delete_selected_text(state);
                } else {
                        delete_char(state);
                }
                break;
        case KEY_ENTER:
        case '\n':
                if (state->select_mode) {
                        clear_selection(state);
                } else {
                        new_line(state);
                }
                break;
        case KEY_F(1):
                toggle_help(state);
                break;
        case KEY_F(2):
                if (can_process_key(state, KEY_F(2))) {
                        find_text(state);
                        mark_key_processed(state, KEY_F(2));
                }
                break;
        case KEY_F(3):
                if (can_process_key(state, KEY_F(3))) {
                        replace_text(state);
                        mark_key_processed(state, KEY_F(3));
                }
                break;
        case KEY_F(4):
                if (can_process_key(state, KEY_F(4))) {
                        cut_text(state);
                        mark_key_processed(state, KEY_F(4));
                }
                break;
        case KEY_F(5):
                if (can_process_key(state, KEY_F(5))) {
                        copy_text(state);
                        mark_key_processed(state, KEY_F(5));
                }
                break;
        case KEY_F(6):
                if (can_process_key(state, KEY_F(6))) {
                        paste_text(state);
                        mark_key_processed(state, KEY_F(6));
                }
                break;
        case KEY_F(7):
                toggle_word_wrap(state);
                break;
        case KEY_F(8):
                toggle_syntax_highlighting(state);
                break;
        case KEY_F(9):
                toggle_autosave(state);
                save_config(state);
                break;
        case KEY_F(10):
                toggle_syntax_display(state);
                break;
        case KEY_IC:
                paste_text(state);
                break;
        case KEY_MOUSE:
                handle_mouse_event(state);
                break;
        case '\t':
                for (int i = 0; i < state -> tab_size; i++) {
                        insert_char(state, ' ');
                }
                move_cursor(state, 0, 0);
                break;
        case 27:
                if (state->char_select_mode) {
                        state->char_select_mode = 0;
                } else if (state -> select_mode) {
                        clear_selection(state);
                } else if (state -> show_help) {
                        state -> show_help = 0;
                } else {
                        safe_quit(state);
                }
                break;
        case 23:  
                
                printf("\033[?2004l"); fflush(stdout);
                endwin();
                exit(0);
                break;
        default:
                if (isprint(ch)) {
                        if (selection_started_with_shift && state->select_mode) {
                                copy_selected_text(state);
                                clear_selection(state);
                                selection_started_with_shift = 0;
                        }
                        reset_key_states(state);
                        insert_char(state, ch);
                        move_cursor(state, 0, 0);
                }
                break;
        }
}

void handle_ctrl_keys(EditorState * state, int ch) {

        int needs_tracking = 0;
        switch (ch) {
        case 19:
        case 3:
        case 22:
        case 24:
        case 6:
        case 8:
        case 15:
        case 18:
        case 20:
        case 11:
        case 21:
        case 23:  
                needs_tracking = 1;
                break;
        }

        if (needs_tracking && !can_process_key(state, ch)) {
                return;
        }

        switch (ch) {
        case 17:
                safe_quit(state);
                break;
        case 19:
                save_file(state);
                mark_key_processed(state, ch);
                break;
        case 15:
                prompt_open_file(state);
                mark_key_processed(state, ch);
                break;
        case 8:


                toggle_help(state);
                mark_key_processed(state, ch);
                break;
        case 1:
                select_all(state);
                break;
        case 2:  
                select_current_line(state);
                break;
        case 3:
                if (state -> select_mode) {
                        copy_selected_text(state);
                } else {
                        copy_text(state);
                }
                mark_key_processed(state, ch);
                break;
        case 22:
                paste_text(state);
                mark_key_processed(state, ch);
                break;
        case 6:
                find_text(state);
                mark_key_processed(state, ch);
                break;
        case 18:
                replace_text(state);
                mark_key_processed(state, ch);
                break;
        case 24:
                if (state -> select_mode) {
                        cut_text(state);
                        mark_key_processed(state, ch);
                } else {
                }
                break;

        case 12:
                jump_to_line(state);
                break;
        case 20:
                toggle_auto_tabbing(state);
                mark_key_processed(state, ch);
                break;
        case 11:
                toggle_auto_complete(state);
                mark_key_processed(state, ch);
                break;
        case 21:
                toggle_comment_complete(state);
                mark_key_processed(state, ch);
                break;
        case 23:
                state->char_select_mode = 1;
                clear_selection(state);

                start_selection(state);
                mark_key_processed(state, ch);
                break;
        case 5:
                state -> cursor_y = state -> line_count - 1;
                state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                move_cursor(state, 0, 0);
                break;
        case 4:
                delete_current_line(state);
                break;
        case 14:  
                load_plugin_interactive(state);
                break;
        default:
                break;
        }
}

void cut_text(EditorState * state) {
        if (state -> select_mode && has_selection(state)) {

                char * selected = get_selected_text(state);
                if (selected) {
                        copy_to_system_clipboard(selected);
                        delete_selected_text(state);
                        free(selected);
                }
        } else if (strlen(state -> lines[state -> cursor_y]) > 0) {

                copy_to_system_clipboard(state -> lines[state -> cursor_y]);
                state -> lines[state -> cursor_y][0] = '\0';
                state -> cursor_x = 0;
                state -> dirty = 1;
        } else {
        }
}

void copy_text(EditorState * state) {
        if (strlen(state -> lines[state -> cursor_y]) > 0) {
                copy_to_system_clipboard(state -> lines[state -> cursor_y]);
        }
}

void delete_current_line(EditorState * state) {
        if (state -> line_count <= 1) {
                show_status(state, "Cannot delete the last line");
                return;
        }

        free(state -> lines[state -> cursor_y]);

        for (int i = state -> cursor_y; i < state -> line_count - 1; i++) {
                state -> lines[i] = state -> lines[i + 1];
        }

        state -> line_count--;

        if (state -> cursor_y >= state -> line_count) {
                state -> cursor_y = state -> line_count - 1;
        }

        if (state -> cursor_y < 0) state -> cursor_y = 0;

        state -> cursor_x = 0;

        move_cursor(state, 0, 0);

        state -> dirty = 1;
}

static void paste_from_string(EditorState * state, const char *clipboard_content) {
        
        if (!state || !state->lines || state->cursor_y >= state->line_count) {
                show_status(state, "Invalid editor state for paste operation");
                return;
        }
        if (!clipboard_content || clipboard_content[0] == '\0') {
                show_status(state, "No content to paste");
                return;
        }

        
        if (state->cursor_x > (int)strlen(state->lines[state->cursor_y])) {
                state->cursor_x = strlen(state->lines[state->cursor_y]);
        }

        
        size_t total_len = strlen(clipboard_content);
        char *content_copy = (char*)malloc(total_len + 1);
        if (!content_copy) {
                show_status(state, "Memory allocation failed for paste");
                return;
        }
        memcpy(content_copy, clipboard_content, total_len + 1);

        int total_chars_pasted = 0;
        int line_index = 0;

        
        size_t start = 0;
        while (1) {
                
                size_t i = start;
                while (i < total_len && content_copy[i] != '\n') i++;

                size_t seg_len = i - start;

                if (line_index == 0) {
                        
                        char *current_line = state->lines[state->cursor_y];
                        int current_len = strlen(current_line);

                        if (current_len + (int)seg_len >= MAX_LINE_LENGTH) {
                                show_status(state, "Content too large for current line");
                                free(content_copy);
                                return;
                        }

                        
                        memmove(&current_line[state->cursor_x + seg_len],
                                &current_line[state->cursor_x],
                                current_len - state->cursor_x + 1);
                        if (seg_len > 0) {
                                memcpy(&current_line[state->cursor_x], &content_copy[start], seg_len);
                        }

                        state->cursor_x += seg_len;
                        total_chars_pasted += (int)seg_len;
                } else {
                        
                        if (state->line_count >= MAX_LINES) {
                                show_status(state, "Maximum line count reached during paste (1,000,000 lines)");
                                free(content_copy);
                                return;
                        }

                        char *new_line = (char*)malloc(MAX_LINE_LENGTH);
                        if (!new_line) {
                                free(content_copy);
                                return;
                        }

                        if (seg_len > 0) {
                                if (seg_len >= MAX_LINE_LENGTH) {
                                        free(new_line);
                                        show_status(state, "Pasted line exceeds maximum length");
                                        free(content_copy);
                                        return;
                                }
                                memcpy(new_line, &content_copy[start], seg_len);
                                new_line[seg_len] = '\0';
                        } else {
                                
                                new_line[0] = '\0';
                        }

                        
                        for (int k = state->line_count; k > state->cursor_y + line_index; k--) {
                                state->lines[k] = state->lines[k - 1];
                        }
                        state->lines[state->cursor_y + line_index] = new_line;
                        state->line_count++;

                        total_chars_pasted += (int)seg_len;
                }

                line_index++;

                
                if (i >= total_len) break;

                
                start = i + 1;
                if (start > total_len) break;
        }

        
        if (state->line_count > 0 && strlen(state->lines[state->line_count - 1]) > 0 && state->line_count < MAX_LINES) {
                state->lines[state->line_count] = (char*)malloc(MAX_LINE_LENGTH);
                if (state->lines[state->line_count]) {
                        state->lines[state->line_count][0] = '\0';
                        state->line_count++;
                }
        }

        state->cursor_x = strlen(state->lines[state->cursor_y]);

        state->dirty = 1;


        free(content_copy);
}

void paste_text(EditorState * state) {
        
        char *clipboard_content = get_system_clipboard();
        if (!clipboard_content || clipboard_content[0] == '\0') {
                if (clipboard_content) free(clipboard_content);
                show_status(state, "No content to paste");
                return;
        }

        paste_from_string(state, clipboard_content);
        free(clipboard_content);
        move_cursor(state, 0, 0);
}

static int try_handle_bracketed_paste(EditorState * state, int first_ch) {
        
        if (first_ch != 27) return 0;

        int c1 = getch();
        if (c1 != '[') { if (c1 != ERR) ungetch(c1); return 0; }

        int c2 = getch();
        int c3 = getch();
        int c4 = getch();
        if (!(c2=='2' && c3=='0' && c4=='0')) {
                if (c4 != ERR) ungetch(c4);
                if (c3 != ERR) ungetch(c3);
                if (c2 != ERR) ungetch(c2);
                
                return 0;
        }
        int c5 = getch();
        if (c5 != '~') {
                if (c5 != ERR) ungetch(c5);
                if (c4 != ERR) ungetch(c4);
                if (c3 != ERR) ungetch(c3);
                if (c2 != ERR) ungetch(c2);
                return 0;
        }

        size_t cap = 4096, len = 0;
        char *buf = (char*)malloc(cap);
        if (!buf) return 1; 

        for (;;) {
                int ch = getch();
                if (ch == 27) {
                        int d1 = getch();
                        if (d1 == '[') {
                                int d2 = getch();
                                int d3 = getch();
                                int d4 = getch();
                                if (d2=='2' && d3=='0' && d4=='1') {
                                        int d5 = getch();
                                        if (d5 == '~') {
                                                
                                                break;
                                        } else {
                                                
                                                if (len + 5 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; }
                                                buf[len++] = 27; buf[len++] = '['; buf[len++] = '2'; buf[len++] = '0'; buf[len++] = '1';
                                                
                                                if (d5 != ERR) ungetch(d5);
                                                continue;
                                        }
                                } else {
                                        
                                        if (len + 2 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; }
                                        buf[len++] = 27; buf[len++] = '[';
                                        if (d2 != ERR) { if (len + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; } buf[len++] = (char)d2; }
                                        if (d3 != ERR) { if (len + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; } buf[len++] = (char)d3; }
                                        if (d4 != ERR) { if (len + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; } buf[len++] = (char)d4; }
                                        continue;
                                }
                        } else {
                                
                                if (len + 2 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; }
                                buf[len++] = 27;
                                if (d1 != ERR) buf[len++] = (char)d1;
                                continue;
                        }
                } else if (ch == ERR) {
                        
                        continue;
                } else {
                        
                        if (ch == '\r') ch = '\n';
                        if (len + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); if (!buf) return 1; }
                        buf[len++] = (char)ch;
                }
        }

        
        if (len + 1 >= cap) { cap += 1; buf = (char*)realloc(buf, cap); if (!buf) return 1; }
        buf[len] = '\0';

        paste_from_string(state, buf);
        free(buf);
        return 1;
}

void select_current_word(EditorState *state) {
    if (!state->lines[state->cursor_y]) return;
    char *line = state->lines[state->cursor_y];
    int len = strlen(line);
    if (state->cursor_x >= len || (!isalnum(line[state->cursor_x]) && line[state->cursor_x] != '_')) {
        return;
    }
    int start = state->cursor_x;
    int end = state->cursor_x;
    
    while (start > 0 && (isalnum(line[start-1]) || line[start-1] == '_')) start--;
    
    while (end < len && (isalnum(line[end]) || line[end] == '_')) end++;
    
    if (start < end) {
        state->select_start_x = start;
        state->select_start_y = state->cursor_y;
        state->select_end_x = end;
        state->select_end_y = state->cursor_y;
        state->select_mode = 1;

        
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int avail_w = max_x - 6 - 1;
        if (avail_w < 1) avail_w = 1;
        if (state->cursor_x >= state->horizontal_scroll_offset + avail_w) {
            state->horizontal_scroll_offset = state->cursor_x - avail_w + 1;
        }
        if (state->cursor_x < state->horizontal_scroll_offset) {
            state->horizontal_scroll_offset = state->cursor_x;
        }
    }
}

void handle_mouse_event(EditorState * state) {
        MEVENT event;

        if (getmouse(&event) == OK) {
                
                int doc_y = event.y - 3;
                int screen_x = event.x - 6;
                doc_y += state->scroll_offset;

                if (doc_y < 0 || doc_y >= state->line_count) {
                        return; 
                }

                if (screen_x < 0) screen_x = 0;

                int doc_x = screen_x + state->horizontal_scroll_offset;
                int line_len = 0;
                if (state->lines[doc_y]) {
                        line_len = (int)strlen(state->lines[doc_y]);
                }

                
                if (doc_x < 0) doc_x = 0;
                if (doc_x > line_len) doc_x = line_len;

                if (event.bstate & BUTTON1_PRESSED) {
                        dragging = 1;
                        
                        if (state->lines[doc_y]) {
                                if (doc_x < line_len) {
                                        if (state->lines[doc_y][doc_x] != ' ') {
                                                if (has_selection(state)) {
                                                        clear_selection(state);
                                                }
                                                state->cursor_y = doc_y;
                                                state->cursor_x = doc_x;
                                                move_cursor(state, 0, 0);
                                        }
                                } else {
                                        
                                        if (has_selection(state)) {
                                                clear_selection(state);
                                        }
                                        state->cursor_y = doc_y;
                                        state->cursor_x = line_len;
                                        move_cursor(state, 0, 0);
                                }
                        }
                } else if (event.bstate & REPORT_MOUSE_POSITION) {
                        if (dragging) {
                                if (!state->select_mode) {
                                        start_selection(state);
                                }
                                state->cursor_y = doc_y;
                                state->cursor_x = doc_x;
                                move_cursor(state, 0, 0);
                                extend_selection(state);
                        }
                } else if (event.bstate & BUTTON1_CLICKED) {
                    dragging = 0;
                    
                } else if (event.bstate & BUTTON1_DOUBLE_CLICKED) {
                    
                    clear_selection(state);
                    state->cursor_y = doc_y;
                    state->cursor_x = doc_x;
                    move_cursor(state, 0, 0);
                    select_current_word(state);
                    if (has_selection(state)) {
                        copy_selected_text(state);
                    }
                } else if (event.bstate & BUTTON3_CLICKED) {
                        
                        paste_text(state);
                } else if (event.bstate & BUTTON4_PRESSED) {
                        dragging = 0;
                        move_cursor(state, 0, -1);
                } else if (event.bstate & BUTTON5_PRESSED) {
                        dragging = 0;
                        move_cursor(state, 0, 1);
                }
        }
}
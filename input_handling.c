#define _POSIX_C_SOURCE 200809L
#include "editor.h"
#include <stdlib.h>
#include <stdio.h>

void delete_current_line(EditorState * state);

char * get_system_clipboard() {
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

        if (ch < 32 && ch != 27 && ch != '\n' && ch != '\t' && ch != KEY_BACKSPACE) {
                handle_ctrl_keys(state, ch);
                return;
        }

        switch (ch) {
        case KEY_UP:
                move_cursor(state, 0, -1);
                break;
        case KEY_DOWN:
                move_cursor(state, 0, 1);
                break;
        case KEY_LEFT:
                move_cursor(state, -1, 0);
                break;
        case KEY_RIGHT:
                move_cursor(state, 1, 0);
                break;
        case KEY_HOME:

                state -> cursor_x = 0;
                break;
        case KEY_END:

                if (state -> cursor_y < state -> line_count && state -> lines[state -> cursor_y]) {
                        state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                }
                break;
        case KEY_BACKSPACE:
        case 127:
                if (state -> select_mode && has_selection(state)) {

                        delete_selected_text(state);
                } else {
                        delete_char(state);
                }
                break;
        case KEY_ENTER:
        case '\n':
                new_line(state);
                break;
        case KEY_F(1):
                toggle_help(state);
                break;
        case KEY_F(2):
                toggle_word_wrap(state);
                break;
        case KEY_F(3):
                if (can_process_key(state, KEY_F(3))) {
                        find_text(state);
                        mark_key_processed(state, KEY_F(3));
                }
                break;
        case KEY_F(4):
                if (can_process_key(state, KEY_F(4))) {
                        replace_text(state);
                        mark_key_processed(state, KEY_F(4));
                }
                break;
        case KEY_F(5):
                count_stats(state);
                break;
        case KEY_F(6):
                if (can_process_key(state, KEY_F(6))) {
                        cut_text(state);
                        mark_key_processed(state, KEY_F(6));
                }
                break;
        case KEY_F(7):
                if (can_process_key(state, KEY_F(7))) {
                        copy_text(state);
                        mark_key_processed(state, KEY_F(7));
                }
                break;
        case KEY_F(8):
                if (can_process_key(state, KEY_F(8))) {
                        paste_text(state);
                        mark_key_processed(state, KEY_F(8));
                }
                break;
        case KEY_F(9):
                if (can_process_key(state, KEY_F(9))) {
                        undo(state);
                        mark_key_processed(state, KEY_F(9));
                }
                break;
        case KEY_F(10):
                if (can_process_key(state, KEY_F(10))) {
                        redo(state);
                        mark_key_processed(state, KEY_F(10));
                }
                break;
        case KEY_F(12):
                toggle_syntax_highlighting(state);
                break;
        case '\t':
                for (int i = 0; i < state -> tab_size; i++) {
                        insert_char(state, ' ');
                }
                break;
        case 27:
                if (state -> show_help) {

                        state -> show_help = 0;
                } else if (state -> select_mode) {
                        clear_selection(state);
                } else {
                        endwin();
                        exit(0);
                }
                break;
        default:
                if (isprint(ch)) {

                        reset_key_states(state);
                        insert_char(state, ch);
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
        case 26:
        case 25:
        case 24:
        case 6:
        case 4:
                needs_tracking = 1;
                break;
        }

        if (needs_tracking && !can_process_key(state, ch)) {
                return;
        }

        switch (ch) {
        case 17:
                endwin();
                exit(0);
                break;
        case 19:
                save_file(state);
                mark_key_processed(state, ch);
                break;
        case 1:
                select_all(state);
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
        case 26:
                undo(state);
                mark_key_processed(state, ch);
                break;
        case 25:
                redo(state);
                mark_key_processed(state, ch);
                break;
        case 6:
                find_text(state);
                mark_key_processed(state, ch);
                break;
        case 24:
                if (state -> select_mode) {
                        cut_text(state);
                        mark_key_processed(state, ch);
                } else {
                        show_status(state, "Select text first (Ctrl+A for all, or use mouse)");
                }
                break;
        case 8:
                toggle_help(state);
                break;
        case 12:
                jump_to_line(state);
                break;
        case 18:
                state -> cursor_y = 0;
                state -> cursor_x = 0;
                move_cursor(state, 0, 0);
                break;
        case 5:
                state -> cursor_y = state -> line_count - 1;
                state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                move_cursor(state, 0, 0);
                break;
        case 4:
                delete_current_line(state);
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
                        show_status(state, "Selected text cut to clipboard");
                }
        } else if (strlen(state -> lines[state -> cursor_y]) > 0) {

                save_undo_state(state);

                copy_to_system_clipboard(state -> lines[state -> cursor_y]);
                state -> lines[state -> cursor_y][0] = '\0';
                state -> cursor_x = 0;
                state -> dirty = 1;
                show_status(state, "Line cut to clipboard");
        } else {
                show_status(state, "No text to cut");
        }
}

void copy_text(EditorState * state) {
        if (strlen(state -> lines[state -> cursor_y]) > 0) {
                copy_to_system_clipboard(state -> lines[state -> cursor_y]);
                show_status(state, "Line copied to clipboard");
        }
}

void delete_current_line(EditorState * state) {
        if (state -> line_count <= 1) {
                show_status(state, "Cannot delete the last line");
                return;
        }

        save_undo_state(state);

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

        show_status(state, "Line deleted");
}

void paste_text(EditorState * state) {

        if (!state || !state -> lines || state -> cursor_y >= state -> line_count) {
                show_status(state, "Invalid editor state for paste operation");
                return;
        }

        char * clipboard_content = get_system_clipboard();
        if (!clipboard_content || strlen(clipboard_content) == 0) {
                if (clipboard_content) free(clipboard_content);
                show_status(state, "No content to paste");
                return;
        }

        size_t len = strlen(clipboard_content);
        while (len > 0 && (clipboard_content[len - 1] == '\n' || clipboard_content[len - 1] == '\r')) {
                clipboard_content[len - 1] = '\0';
                len--;
        }

        if (clipboard_content && strlen(clipboard_content) > 0) {
                save_undo_state(state);

                if (state -> cursor_x > (int) strlen(state -> lines[state -> cursor_y])) {
                        state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                }

                char * content_copy = (char * ) malloc(strlen(clipboard_content) + 1);
                if (!content_copy) {
                        show_status(state, "Memory allocation failed for paste");
                        free(clipboard_content);
                        return;
                }
                strcpy(content_copy, clipboard_content);

                char * line = strtok(content_copy, "\n");
                int total_chars_pasted = 0;
                int line_index = 0;

                while (line != NULL) {
                        int line_len = strlen(line);

                        if (line_index == 0) {

                                char * current_line = state -> lines[state -> cursor_y];
                                int current_len = strlen(current_line);

                                if (current_len + line_len >= MAX_LINE_LENGTH) {
                                        show_status(state, "Content too large for current line");
                                        free(content_copy);
                                        free(clipboard_content);
                                        return;
                                }

                                memmove( & current_line[state -> cursor_x + line_len], &
                                        current_line[state -> cursor_x],
                                        current_len - state -> cursor_x + 1);
                                memcpy( & current_line[state -> cursor_x], line, line_len);

                                state -> cursor_x += line_len;
                                total_chars_pasted += line_len;
                        } else {

                                if (state -> line_count >= MAX_LINES) {
                                        show_status(state, "Maximum line count reached during paste (1,000,000 lines)");
                                        free(content_copy);
                                        free(clipboard_content);
                                        return;
                                }

                                char * new_line = (char * ) malloc(MAX_LINE_LENGTH);
                                if (!new_line) {
                                        free(content_copy);
                                        free(clipboard_content);
                                        return;
                                }
                                strcpy(new_line, line);

                                for (int i = state -> line_count; i > state -> cursor_y + line_index; i--) {
                                        state -> lines[i] = state -> lines[i - 1];
                                }

                                state -> lines[state -> cursor_y + line_index] = new_line;
                                state -> line_count++;
                                total_chars_pasted += line_len;
                        }

                        line = strtok(NULL, "\n");
                        line_index++;
                }

                state -> cursor_y += (line_index - 1);
                if (line_index > 1) {
                        state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                }

                if (state -> cursor_x > (int) strlen(state -> lines[state -> cursor_y])) {
                        state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                }

                state -> dirty = 1;

                char status_msg[256];
                snprintf(status_msg, sizeof(status_msg), "Pasted %d characters", total_chars_pasted);
                show_status(state, status_msg);

                free(content_copy);
        } else {
                show_status(state, "No content to paste");
        }

        if (clipboard_content) free(clipboard_content);
}
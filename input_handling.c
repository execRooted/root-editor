#include "editor.h"

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
        case KEY_F(11):
                toggle_autosave(state);
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
        default:
                break;
        }
}

void cut_text(EditorState * state) {
        if (state -> select_mode && has_selection(state)) {

                char * selected = get_selected_text(state);
                if (selected) {
                        strcpy(state -> clipboard, selected);
                        copy_to_system_clipboard(selected);
                        delete_selected_text(state);
                        free(selected);
                        show_status(state, "Selected text cut to clipboard");
                }
        } else if (strlen(state -> lines[state -> cursor_y]) > 0) {

                save_undo_state(state);

                strcpy(state -> clipboard, state -> lines[state -> cursor_y]);
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
                strcpy(state -> clipboard, state -> lines[state -> cursor_y]);
                copy_to_system_clipboard(state -> lines[state -> cursor_y]);
                show_status(state, "Line copied to clipboard");
        }
}

void paste_text(EditorState * state) {

        if (!state || !state -> lines || state -> cursor_y >= state -> line_count) {
                show_status(state, "Invalid editor state for paste operation");
                return;
        }

        char * clipboard_content = NULL;
        size_t content_size = 0;

        FILE * sys_clip = fopen("/tmp/kilo_editor_clipboard.txt", "r");
        if (sys_clip) {
                fseek(sys_clip, 0, SEEK_END);
                content_size = ftell(sys_clip);
                fseek(sys_clip, 0, SEEK_SET);

                if (content_size > 0 && content_size < 1024 * 1024) {
                        clipboard_content = (char * ) malloc(content_size + 1);
                        if (clipboard_content) {
                                size_t bytes_read = fread(clipboard_content, 1, content_size, sys_clip);
                                if (bytes_read == content_size) {
                                        clipboard_content[content_size] = '\0';
                                        if (content_size > 0 && clipboard_content[content_size - 1] == '\n') {
                                                clipboard_content[content_size - 1] = '\0';
                                        }
                                } else {
                                        free(clipboard_content);
                                        clipboard_content = NULL;
                                }
                        }
                }
                fclose(sys_clip);
        }

        if (!clipboard_content || strlen(clipboard_content) == 0) {
                if (clipboard_content) free(clipboard_content);
                size_t internal_len = strlen(state -> clipboard);
                if (internal_len > 0) {
                        clipboard_content = (char * ) malloc(internal_len + 1);
                        if (clipboard_content) {
                                strcpy(clipboard_content, state -> clipboard);
                        }
                }
        }

        if (!clipboard_content || strlen(clipboard_content) == 0) {
                if (clipboard_content) free(clipboard_content);
                clipboard_content = NULL;
        }

        if (clipboard_content) {
                size_t len = strlen(clipboard_content);
                while (len > 0 && (clipboard_content[len - 1] == '\n' || clipboard_content[len - 1] == '\r')) {
                        clipboard_content[len - 1] = '\0';
                        len--;
                }
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
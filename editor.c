#include "editor.h"

void init_editor(EditorState * state) {
        state -> lines = (char ** ) malloc(MAX_LINES * sizeof(char * ));

        state -> lines[0] = (char * ) malloc(MAX_LINE_LENGTH);
        state -> lines[0][0] = '\0';

        state -> line_count = 1;
        state -> cursor_x = 0;
        state -> cursor_y = 0;
        state -> scroll_offset = 0;
        state -> undo_count = 0;
        state -> redo_count = 0;
        state -> filename[0] = '\0';
        state -> dirty = 0;
        state -> line_numbers = 1;
        state -> word_wrap = 0;
        state -> tab_size = TAB_SIZE;
        state -> select_mode = 0;
        state -> select_start_x = 0;
        state -> select_start_y = 0;
        state -> select_end_x = 0;
        state -> select_end_y = 0;
        state -> show_help = 0;
        state -> needs_sudo = 0;

        memset(state -> key_states, 0, sizeof(state -> key_states));
        memset(state -> key_timestamps, 0, sizeof(state -> key_timestamps));

        state -> syntax_enabled = 1;
        state -> file_type = 0;
        state -> keywords = NULL;
        state -> keyword_count = 0;
}

void toggle_dark_mode(EditorState * state) {

        show_status(state, "Dark mode not available");
}

void insert_char(EditorState * state, char c) {

        if (!state || !state -> lines || state -> cursor_y >= state -> line_count ||
                state -> cursor_y < 0 || !state -> lines[state -> cursor_y]) {
                return;
        }

        char * line = state -> lines[state -> cursor_y];
        int len = strlen(line);

        if (state -> cursor_x < 0) state -> cursor_x = 0;
        if (state -> cursor_x > len) state -> cursor_x = len;

        if (len < MAX_LINE_LENGTH - 1) {
                memmove( & line[state -> cursor_x + 1], & line[state -> cursor_x], len - state -> cursor_x + 1);
                line[state -> cursor_x] = c;
                state -> cursor_x++;
                state -> dirty = 1;
        }
}

void delete_char(EditorState * state) {

        if (!state || !state -> lines || state -> cursor_y >= state -> line_count ||
                state -> cursor_y < 0 || !state -> lines[state -> cursor_y]) {
                return;
        }

        if (state -> cursor_x > 0) {
                char * line = state -> lines[state -> cursor_y];
                int len = strlen(line);

                if (state -> cursor_x > len) state -> cursor_x = len;

                memmove( & line[state -> cursor_x - 1], & line[state -> cursor_x],
                        len - state -> cursor_x + 1);
                state -> cursor_x--;
                state -> dirty = 1;
        } else if (state -> cursor_y > 0) {

                if (!state -> lines[state -> cursor_y - 1]) return;

                char * prev_line = state -> lines[state -> cursor_y - 1];
                char * curr_line = state -> lines[state -> cursor_y];
                int prev_len = strlen(prev_line);
                int curr_len = strlen(curr_line);

                if (prev_len + curr_len < MAX_LINE_LENGTH) {
                        strcat(prev_line, curr_line);
                        free(curr_line);

                        for (int i = state -> cursor_y; i < state -> line_count - 1; i++) {
                                state -> lines[i] = state -> lines[i + 1];
                        }

                        state -> line_count--;
                        state -> cursor_y--;
                        state -> cursor_x = prev_len;
                        state -> dirty = 1;
                }
        }
}

void new_line(EditorState * state) {
        save_undo_state(state);
        if (state -> line_count >= MAX_LINES) {
                show_status(state, "Error: Maximum line count reached (1,000,000 lines)");
                return;
        }

        int old_cursor_y = state -> cursor_y;
        int old_line_count = state -> line_count;

        char * line = state -> lines[state -> cursor_y];

        char * new_line = (char * ) malloc(MAX_LINE_LENGTH);
        if (!new_line) {
                show_status(state, "Memory allocation failed");
                return;
        }
        strcpy(new_line, & line[state -> cursor_x]);
        line[state -> cursor_x] = '\0';

        for (int i = state -> line_count; i > state -> cursor_y + 1; i--) {
                state -> lines[i] = state -> lines[i - 1];
        }

        state -> lines[state -> cursor_y + 1] = new_line;
        state -> line_count++;
        state -> cursor_y++;
        state -> cursor_x = 0;

        move_cursor(state, 0, 0);
        napms(10);
        state -> dirty = 1;
}

void move_cursor(EditorState * state, int dx, int dy) {

        if (!state || !state -> lines || state -> line_count <= 0) {
                return;
        }

        int new_x = state -> cursor_x + dx;
        int new_y = state -> cursor_y + dy;

        if (new_y < 0) new_y = 0;
        if (new_y >= state -> line_count) new_y = state -> line_count - 1;

        if (!state -> lines[new_y]) return;

        int max_x = strlen(state -> lines[new_y]);

        if (new_x < 0) {

                if (new_y > 0 && state -> lines[new_y - 1]) {
                        state -> cursor_y = new_y - 1;
                        state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
                }
        } else if (new_x > max_x) {

                if (new_y < state -> line_count - 1 && state -> lines[new_y + 1]) {
                        state -> cursor_y = new_y + 1;
                        state -> cursor_x = 0;
                } else {
                        state -> cursor_x = max_x;
                }
        } else {
                state -> cursor_x = new_x;
                state -> cursor_y = new_y;
        }

        int max_y, max_x_screen;
        getmaxyx(stdscr, max_y, max_x_screen);

        if (state -> cursor_y <= state -> scroll_offset + 2) {
                state -> scroll_offset = state -> cursor_y - 3;
                if (state -> scroll_offset < 0) state -> scroll_offset = 0;
        } else if (state -> cursor_y >= state -> scroll_offset + max_y - 5) {
                state -> scroll_offset = state -> cursor_y - max_y + 6;
        }

        if (state -> scroll_offset < 0) state -> scroll_offset = 0;
        if (state -> scroll_offset >= state -> line_count) state -> scroll_offset = state -> line_count - 1;
}

void show_status(EditorState * state,
        const char * message) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        attron(COLOR_PAIR(1));
        mvprintw(max_y - 1, max_x / 2, " %s ", message);
        attroff(COLOR_PAIR(1));
        refresh();

        napms(500);
}

void show_status_left(EditorState * state,
        const char * message) {
        int old_cursor_y = state -> cursor_y;
        int old_line_count = state -> line_count;

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        attron(COLOR_PAIR(1));
        mvprintw(max_y - 1, 0, "%s", message);
        attroff(COLOR_PAIR(1));
        refresh();

        napms(500);
}

void count_stats(EditorState * state) {
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
        show_status(state, msg);
}

void toggle_word_wrap(EditorState * state) {
        state -> word_wrap = !state -> word_wrap;
        show_status(state, state -> word_wrap ? "Word wrap enabled" : "Word wrap disabled");
}

void save_undo_state(EditorState * state) {

        if (state -> undo_count >= MAX_UNDO_STATES) {

                free(state -> undo_buffer[0].lines);
                for (int i = 0; i < state -> undo_buffer[0].line_count; i++) {
                        free(state -> undo_buffer[0].lines[i]);
                }
                for (int i = 1; i < MAX_UNDO_STATES; i++) {
                        state -> undo_buffer[i - 1] = state -> undo_buffer[i];
                }
                state -> undo_count--;
        }

        UndoState * undo_state = & state -> undo_buffer[state -> undo_count];
        undo_state -> line_count = state -> line_count;
        undo_state -> cursor_x = state -> cursor_x;
        undo_state -> cursor_y = state -> cursor_y;
        undo_state -> scroll_offset = state -> scroll_offset;

        undo_state -> lines = (char ** ) malloc(state -> line_count * sizeof(char * ));
        for (int i = 0; i < state -> line_count; i++) {
                undo_state -> lines[i] = (char * ) malloc(MAX_LINE_LENGTH);
                strcpy(undo_state -> lines[i], state -> lines[i]);
        }

        state -> undo_count++;

}

void restore_state(EditorState * state, UndoState * restore_state) {

        for (int i = 0; i < state -> line_count; i++) {
                free(state -> lines[i]);
        }

        state -> line_count = restore_state -> line_count;
        state -> cursor_x = restore_state -> cursor_x;
        state -> cursor_y = restore_state -> cursor_y;
        state -> scroll_offset = restore_state -> scroll_offset;

        for (int i = 0; i < state -> line_count; i++) {
                state -> lines[i] = (char * ) malloc(MAX_LINE_LENGTH);
                strcpy(state -> lines[i], restore_state -> lines[i]);
        }

        state -> dirty = 1;
}

void undo(EditorState * state) {
        if (state -> undo_count == 0) {
                show_status(state, "Nothing to undo");
                return;
        }

        if (state -> redo_count >= MAX_UNDO_STATES) {

                free(state -> redo_buffer[0].lines);
                for (int i = 0; i < state -> redo_buffer[0].line_count; i++) {
                        free(state -> redo_buffer[0].lines[i]);
                }
                for (int i = 1; i < MAX_UNDO_STATES; i++) {
                        state -> redo_buffer[i - 1] = state -> redo_buffer[i];
                }
                state -> redo_count--;
        }

        UndoState * redo_state = & state -> redo_buffer[state -> redo_count];
        redo_state -> line_count = state -> line_count;
        redo_state -> cursor_x = state -> cursor_x;
        redo_state -> cursor_y = state -> cursor_y;
        redo_state -> scroll_offset = state -> scroll_offset;

        redo_state -> lines = (char ** ) malloc(state -> line_count * sizeof(char * ));
        for (int i = 0; i < state -> line_count; i++) {
                redo_state -> lines[i] = (char * ) malloc(MAX_LINE_LENGTH);
                strcpy(redo_state -> lines[i], state -> lines[i]);
        }
        state -> redo_count++;

        state -> undo_count--;
        restore_state(state, & state -> undo_buffer[state -> undo_count]);
        show_status(state, "Undid last action");
}

void redo(EditorState * state) {
        if (state -> redo_count == 0) {
                show_status(state, "Nothing to redo");
                return;
        }

        if (state -> undo_count >= MAX_UNDO_STATES) {

                free(state -> undo_buffer[0].lines);
                for (int i = 0; i < state -> undo_buffer[0].line_count; i++) {
                        free(state -> undo_buffer[0].lines[i]);
                }
                for (int i = 1; i < MAX_UNDO_STATES; i++) {
                        state -> undo_buffer[i - 1] = state -> undo_buffer[i];
                }
                state -> undo_count--;
        }

        UndoState * undo_state = & state -> undo_buffer[state -> undo_count];
        undo_state -> line_count = state -> line_count;
        undo_state -> cursor_x = state -> cursor_x;
        undo_state -> cursor_y = state -> cursor_y;
        undo_state -> scroll_offset = state -> scroll_offset;

        undo_state -> lines = (char ** ) malloc(state -> line_count * sizeof(char * ));
        for (int i = 0; i < state -> line_count; i++) {
                undo_state -> lines[i] = (char * ) malloc(MAX_LINE_LENGTH);
                strcpy(undo_state -> lines[i], state -> lines[i]);
        }
        state -> undo_count++;

        state -> redo_count--;
        restore_state(state, & state -> redo_buffer[state -> redo_count]);
        show_status(state, "Redid last action");
}

void copy_to_system_clipboard(const char * text) {
        if (!text || strlen(text) == 0) return;

        char temp_file[] = "/tmp/kilo_editor_clipboard_temp.txt";
        FILE * fp = fopen(temp_file, "w");
        if (!fp) return;
        fprintf(fp, "%s", text);
        fclose(fp);

        int success = 0;
        int is_wayland = getenv("WAYLAND_DISPLAY") != NULL;

        if (is_wayland && system("which wl-copy >/dev/null 2>&1") == 0) {
                char command[1024];
                snprintf(command, sizeof(command), "cat '%s' | wl-copy", temp_file);
                if (system(command) == 0) {
                        success = 1;
                }
        }

        if (!success && system("which xclip >/dev/null 2>&1") == 0) {
                char command[1024];
                snprintf(command, sizeof(command), "cat '%s' | xclip -selection clipboard", temp_file);
                if (system(command) == 0) {
                        success = 1;
                }
                snprintf(command, sizeof(command), "cat '%s' | xclip -selection primary", temp_file);
                system(command);
        }

        if (!success && system("which xsel >/dev/null 2>&1") == 0) {
                char command[1024];
                snprintf(command, sizeof(command), "cat '%s' | xsel --clipboard", temp_file);
                if (system(command) == 0) {
                        success = 1;
                }
        }

        if (!success && system("which termux-clipboard-set >/dev/null 2>&1") == 0) {
                char command[1024];
                snprintf(command, sizeof(command), "cat '%s' | termux-clipboard-set", temp_file);
                if (system(command) == 0) {
                        success = 1;
                }
        }

        if (!success && system("which pbcopy >/dev/null 2>&1") == 0) {
                char command[1024];
                snprintf(command, sizeof(command), "cat '%s' | pbcopy", temp_file);
                if (system(command) == 0) {
                        success = 1;
                }
        }

        unlink(temp_file);

        if (!success) {
        }
}

void toggle_help(EditorState * state) {
        state -> show_help = !state -> show_help;
}

void jump_to_line(EditorState * state) {
        echo();
        curs_set(1);

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        mvprintw(max_y - 2, 0, "Jump to line (1-%d): ", state -> line_count);
        refresh();

        char input[32];
        getnstr(input, sizeof(input) - 1);

        noecho();
        curs_set(1);

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

                char msg[256];
                snprintf(msg, sizeof(msg), "Jumped to line %d", line_num);
                show_status(state, msg);
        } else {

                show_status(state, "Line not found or not created");
        }
}

void render_help_screen(EditorState * state) {

        clear();
        refresh();

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        if (max_y < 10 || max_x < 40) {
                mvprintw(0, 0, "Terminal too small for help display");
                refresh();
                napms(2000);
                state -> show_help = 0;
                return;
        }

        attron(COLOR_PAIR(1));
        for (int i = 0; i < max_x; i++) {
                mvprintw(0, i, "=");
                mvprintw(max_y - 1, i, "=");
        }
        for (int i = 0; i < max_y; i++) {
                mvprintw(i, 0, "|");
                mvprintw(i, max_x - 1, "|");
        }
        mvprintw(1, max_x / 2 - 10, " TEXT EDITOR HELP ");
        attroff(COLOR_PAIR(1));

        int line = 3;
        int max_help_line = max_y - 4;

        if (line < max_help_line) mvprintw(line++, 2, "Keyboard Shortcuts:");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+Q        - Quit");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+S        - Save file");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+A        - Select all");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+X        - Cut selected text");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+C        - Copy to system clipboard");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+V        - Paste from clipboard");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+Z        - Undo last line addition");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+Y        - Redo");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+R        - Go to start of file");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+E        - Go to end of file");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+F        - Find");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+L        - Jump to line");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+M        - Make a new line");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+D        - Delete current line");
        if (line < max_help_line) mvprintw(line++, 2, "Ctrl+H        - Toggle this help");

        line += 2;
        if (line < max_help_line) mvprintw(line++, 2, "Function Keys:");
        if (line < max_help_line) mvprintw(line++, 2, "F1            - Toggle help");
        if (line < max_help_line) mvprintw(line++, 2, "F2            - Toggle word wrap");
        if (line < max_help_line) mvprintw(line++, 2, "F3            - Find all occurrences (shows line numbers)");
        if (line < max_help_line) mvprintw(line++, 2, "F4            - Replace text");
        if (line < max_help_line) mvprintw(line++, 2, "F5            - Show statistics");
        if (line < max_help_line) mvprintw(line++, 2, "F6            - Cut current line");
        if (line < max_help_line) mvprintw(line++, 2, "F7            - Copy editor cliboard current line");
        if (line < max_help_line) mvprintw(line++, 2, "F8            - Paste editor clipboard line");
        if (line < max_help_line) mvprintw(line++, 2, "F9            - Undo");
        if (line < max_help_line) mvprintw(line++, 2, "F10           - Redo");
        if (line < max_help_line) mvprintw(line++, 2, "F12           - Toggle enhanced syntax highlighting");

        line += 2;
        if (line < max_help_line) mvprintw(line++, 2, "Navigation:");
        if (line < max_help_line) mvprintw(line++, 2, "Home/End      - Move to start or end of the line");
        if (line < max_help_line) mvprintw(line++, 2, "Arrow Keys    - Move cursor");
        if (line < max_help_line) mvprintw(line++, 2, "Backspace     - Delete character");
        if (line < max_help_line) mvprintw(line++, 2, "Enter         - New line");
        if (line < max_help_line) mvprintw(line++, 2, "Tab           - Insert spaces");

        line += 2;
        if (line < max_help_line) mvprintw(line++, 2, "File Operations:");
        if (line < max_help_line) mvprintw(line++, 2, "./editor <filename> - Open/create file");
        if (line < max_help_line) mvprintw(line++, 2, "Files are created automatically if they don't exist");

        attron(COLOR_PAIR(1));
        mvprintw(max_y - 2, max_x / 2 - 20, " Press any key to return to editor ");
        attroff(COLOR_PAIR(1));

        refresh();

        timeout(10000);
        int ch = getch();
        timeout(-1);

        if (ch == ERR) {

                show_status(state, "Help timeout - press Ctrl+H to show help again");
        }

        state -> show_help = 0;
}



void toggle_syntax_highlighting(EditorState * state) {
        state -> syntax_enabled = !state -> syntax_enabled;
        if (state -> syntax_enabled) {
                init_syntax_highlighting(state);
                show_status(state, "Syntax highlighting enabled");
        } else {
                free_syntax_data(state);
                show_status(state, "Syntax highlighting disabled");
        }
}

int can_process_key(EditorState * state, int key_code) {
        time_t current_time = time(NULL);

        if (key_code < 0 || key_code >= 256) {
                return 1;
        }

        if (state -> key_states[key_code] == 2) {
                if ((current_time - state -> key_timestamps[key_code]) < 1) {
                        return 0;
                }
        }

        return (state -> key_states[key_code] == 0 || state -> key_states[key_code] == 2);
}

void mark_key_processed(EditorState * state, int key_code) {
        if (key_code >= 0 && key_code < 256) {
                state -> key_states[key_code] = 2;
                state -> key_timestamps[key_code] = time(NULL);
        }
}

void reset_key_states(EditorState * state) {
        memset(state -> key_states, 0, sizeof(state -> key_states));
        memset(state -> key_timestamps, 0, sizeof(state -> key_timestamps));
}

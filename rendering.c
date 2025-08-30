#include "editor.h"

void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width);

void find_all_occurrences(EditorState * state,
        const char * search_term);
void navigate_matches(EditorState * state,
        const char * search_term, int * match_lines, int * match_positions, int match_count);
void jump_to_match(EditorState * state, int line_num, int position);
void replace_text_simple(EditorState * state,
        const char * search_term,
                const char * replace_term);

void render_screen(EditorState * state) {
        erase();

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                strcpy(cwd, "[Unable to get path]");
        }

        if (strlen(state -> filename) > 0) {
                mvprintw(0, 0, "Current File: %s", state -> filename);
                mvprintw(1, 0, "Path: %s", cwd);
        } else {
                mvprintw(0, 0, "Current File: [Untitled]");
                mvprintw(1, 0, "Path: %s", cwd);
        }

        const int line_num_width = 6;
        const int start_line = state -> scroll_offset;
        const int end_line = (start_line + max_y - 4 > state -> line_count) ?
                state -> line_count : start_line + max_y - 4;


        char ** lines = state -> lines;

        for (int i = start_line; i < end_line; i++) {
                const int screen_row = i - start_line + 3;

                attron(COLOR_PAIR(15));
                mvprintw(screen_row, 0, "%5d ", i + 1);
                attroff(COLOR_PAIR(15));

                char * line = lines[i];
                const int line_len = strlen(line);

                if (state -> select_mode && i >= state -> select_start_y && i <= state -> select_end_y) {
                        const int start_x = (i == state -> select_start_y) ? state -> select_start_x : 0;
                        const int end_x = (i == state -> select_end_y) ? state -> select_end_x : line_len;

                        if (start_x > 0) {
                                mvprintw(screen_row, line_num_width, "%.*s", start_x, line);
                        }

                        if (end_x > start_x) {
                                attron(COLOR_PAIR(6));
                                mvprintw(screen_row, line_num_width + start_x, "%.*s",
                                        end_x - start_x, line + start_x);
                                attroff(COLOR_PAIR(6));
                        }

                        if (end_x < line_len) {
                                mvprintw(screen_row, line_num_width + end_x, "%.*s",
                                        line_len - end_x, line + end_x);
                        }
                } else {
                        highlight_line(state, i, screen_row, line_num_width);
                }
        }

        attron(COLOR_PAIR(1));
        mvprintw(max_y - 1, 0, "Line: %d, Col: %d | %s%s",
                state -> cursor_y + 1, state -> cursor_x + 1,
                state -> filename, state -> dirty ? " [Modified]" : "");
        attroff(COLOR_PAIR(1));

        move(state -> cursor_y - start_line + 3, state -> cursor_x + 6);
        refresh();
}

void find_text(EditorState * state) {
        echo();
        curs_set(1);

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        mvprintw(max_y - 2, 0, "Find (letter/word): ");
        refresh();

        char search_term[256];
        getnstr(search_term, sizeof(search_term) - 1);

        noecho();
        curs_set(1);

        if (strlen(search_term) == 0) {
                return;
        }

        find_all_occurrences(state, search_term);
}

void find_all_occurrences(EditorState * state,
        const char * search_term) {

        const int initial_capacity = 64;
        int capacity = initial_capacity;
        int match_count = 0;

        int * match_lines = (int * ) malloc(capacity * sizeof(int));
        int * match_positions = (int * ) malloc(capacity * sizeof(int));

        if (!match_lines || !match_positions) {
                show_status(state, "Memory allocation failed");
                free(match_lines);
                free(match_positions);
                return;
        }

        const int term_len = strlen(search_term);
        char ** lines = state -> lines;
        const int line_count = state -> line_count;

        for (int i = 0; i < line_count; i++) {
                const char * line = lines[i];
                const char * pos = line;

                while ((pos = strstr(pos, search_term)) != NULL) {

                        if (match_count >= capacity) {
                                capacity *= 2;
                                int * new_lines = (int * ) realloc(match_lines, capacity * sizeof(int));
                                int * new_positions = (int * ) realloc(match_positions, capacity * sizeof(int));

                                if (!new_lines || !new_positions) {
                                        show_status(state, "Memory reallocation failed");
                                        free(match_lines);
                                        free(match_positions);
                                        return;
                                }

                                match_lines = new_lines;
                                match_positions = new_positions;
                        }

                        match_lines[match_count] = i;
                        match_positions[match_count] = pos - line;
                        match_count++;

                        pos += term_len;

                        if (term_len == 0) break;
                }
        }

        if (match_count == 0) {
                char msg[256];
                snprintf(msg, sizeof(msg), "No matches found for '%s'", search_term);
                show_status(state, msg);
                free(match_lines);
                free(match_positions);
                return;
        }

        navigate_matches(state, search_term, match_lines, match_positions, match_count);

        free(match_lines);
        free(match_positions);
}

void navigate_matches(EditorState * state,
        const char * search_term, int * match_lines, int * match_positions, int match_count) {
        static int current_match = 0;

        if (match_count == 0) return;

        char msg[256];
        snprintf(msg, sizeof(msg), "Found %d matches for '%s'. Press Enter to cycle through matches", match_count, search_term);
        show_status(state, msg);

        current_match = 0;
        jump_to_match(state, match_lines[current_match], match_positions[current_match]);

        char nav_msg[256];
        snprintf(nav_msg, sizeof(nav_msg), "Match 1/%d - Jumped to line %d", match_count, match_lines[current_match] + 1);
        show_status(state, nav_msg);

        timeout(10000);

        while (1) {
                int ch = getch();

                if (ch == '\n' || ch == KEY_ENTER) {

                        current_match = (current_match + 1) % match_count;
                        jump_to_match(state, match_lines[current_match], match_positions[current_match]);

                        char nav_msg[256];
                        snprintf(nav_msg, sizeof(nav_msg), "Match %d/%d - Jumped to line %d", current_match + 1, match_count, match_lines[current_match] + 1);
                        show_status(state, nav_msg);
                } else if (ch == 27) {

                        show_status(state, "Find navigation exited");
                        break;
                } else if (ch == ERR) {

                        show_status(state, "Find navigation timed out");
                        break;
                }
        }

        timeout(-1);
}

void jump_to_match(EditorState * state, int line_num, int position) {

        state -> cursor_y = line_num;
        state -> cursor_x = position;

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        if (state -> cursor_y < state -> scroll_offset) {
                state -> scroll_offset = state -> cursor_y;
        } else if (state -> cursor_y >= state -> scroll_offset + max_y - 2) {
                state -> scroll_offset = state -> cursor_y - max_y + 3;
        }
}

void replace_text(EditorState * state) {
        echo();
        curs_set(1);

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        mvprintw(max_y - 2, 0, "Replace: ");
        char search_term[256];
        getnstr(search_term, sizeof(search_term) - 1);

        mvprintw(max_y - 2, 0, "With: ");
        char replace_term[256];
        getnstr(replace_term, sizeof(replace_term) - 1);

        noecho();
        curs_set(1);

        if (strlen(search_term) == 0) {
                return;
        }

        replace_text_simple(state, search_term, replace_term);
}

void replace_text_simple(EditorState * state,
        const char * search_term,
                const char * replace_term) {
        save_undo_state(state);

        int replacements = 0;
        for (int i = 0; i < state -> line_count; i++) {
                char * pos = strstr(state -> lines[i], search_term);
                while (pos) {
                        int old_len = strlen(state -> lines[i]);
                        int search_len = strlen(search_term);
                        int replace_len = strlen(replace_term);
                        int new_len = old_len - search_len + replace_len;

                        if (new_len < MAX_LINE_LENGTH) {
                                int offset = pos - state -> lines[i];
                                memmove( & state -> lines[i][offset + replace_len], &
                                        state -> lines[i][offset + search_len],
                                        old_len - offset - search_len + 1);
                                memcpy( & state -> lines[i][offset], replace_term, replace_len);

                                replacements++;
                                pos = strstr( & state -> lines[i][offset + replace_len], search_term);
                        } else {
                                break;
                        }
                }
        }

        if (replacements > 0) {
                state -> dirty = 1;
                char msg[256];
                snprintf(msg, sizeof(msg), "Replaced %d occurrences", replacements);
                show_status(state, msg);
        } else {
                show_status(state, "No occurrences found");
        }
}

void prompt_root_password(EditorState * state) {
        echo();
        curs_set(1);

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        clear();
        attron(COLOR_PAIR(1));
        mvprintw(max_y / 2 - 3, max_x / 2 - 15, " Root Password Required ");
        attroff(COLOR_PAIR(1));

        mvprintw(max_y / 2 - 1, max_x / 2 - 25, "Enter root password (or press Esc to cancel): ");
        int password_x = max_x / 2 - 10;
        mvprintw(max_y / 2 + 2, password_x, "                    ");
        move(max_y / 2 + 2, password_x);
        refresh();

        char password[256] = {
                0
        };
        int ch;
        int pos = 0;

        while (pos < sizeof(password) - 1) {
                ch = getch();

                if (ch == '\n' || ch == KEY_ENTER) {
                        password[pos] = '\0';
                        if (strlen(password) > 0) {
                                char command[512];
                                char temp_path[256];
                                snprintf(temp_path, sizeof(temp_path), "/tmp/root_edit_temp_%d.txt", getpid());

                                FILE * temp_file = fopen(temp_path, "w");
                                if (!temp_file) {
                                        show_status(state, "Error: Could not create temporary file");
                                        noecho();
                                        curs_set(0);
                                        return;
                                }

                                for (int i = 0; i < state -> line_count; i++) {
                                        fprintf(temp_file, "%s\n", state -> lines[i]);
                                }
                                fclose(temp_file);

                                snprintf(command, sizeof(command), "echo '%s' | sudo -S cp %s %s 2>/dev/null",
                                        password, temp_path, state -> filename);

                                int result = system(command);
                                unlink(temp_path);

                                if (result == 0) {
                                        state -> dirty = 0;
                                        state -> needs_sudo = 0;
                                        show_status_left(state, "File saved with root privileges");
                                } else {
                                        show_status(state, "Incorrect password or save failed");
                                }
                        } else {
                                show_status(state, "Save cancelled - no password entered");
                        }
                        break;
                } else if (ch == 27) {
                        show_status(state, "Save cancelled");
                        break;
                } else if (ch == KEY_BACKSPACE || ch == 127) {
                        if (pos > 0) {
                                pos--;
                                mvprintw(max_y / 2 + 2, password_x, "                    ");
                                for (int i = 0; i < pos; i++) {
                                        mvprintw(max_y / 2 + 2, password_x + i, "*");
                                }
                                move(max_y / 2 + 2, password_x + pos);
                        }
                } else if (isprint(ch)) {
                        password[pos++] = ch;
                        mvprintw(max_y / 2 + 2, password_x, "                    ");
                        for (int i = 0; i < pos; i++) {
                                mvprintw(max_y / 2 + 2, password_x + i, "*");
                        }
                        move(max_y / 2 + 2, password_x + pos);
                }
        }

        noecho();
        curs_set(0);
}
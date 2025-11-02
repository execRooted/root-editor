#include "editor.h"

#define FILE_TYPE_PLAIN   0
#define COLOR_DEFAULT     13

char * get_system_clipboard();

void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width, int horizontal_scroll_offset);

void find_all_occurrences(EditorState* state,
        const char* search_term);
void navigate_matches(EditorState* state,
        const char* search_term, int* match_lines, int* match_positions, int match_count);
void jump_to_match(EditorState* state, int line_num, int position);
void replace_text_simple(EditorState* state,
        const char* search_term,
                const char* replace_term);

void render_screen(EditorState* state)
{
        erase();

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
                strcpy(cwd, "[Unable to get path]");
        }

        attron(COLOR_PAIR(1) | A_BOLD);
        if (strlen(state -> filename) > 0) {
                mvprintw(0, 0, "Current File: %s", state -> filename);
                mvprintw(1, 0, "Path: %s", cwd);
        } else {
                mvprintw(0, 0, "Current File: [Untitled]");
                mvprintw(1, 0, "Path: %s", cwd);
        }
        attroff(COLOR_PAIR(1) | A_BOLD);

        const int line_num_width = 8;
        const int start_line = state -> scroll_offset;
        

        const int show_line_numbers = 1;
        const int text_start_col = show_line_numbers ? line_num_width - 2 : 0;
        const int avail_width = max_x - text_start_col - 1;

        char ** lines = state -> lines;

        int screen_row = 3;
        int logical_line = state->scroll_offset;
        int offset_in_line = 0;
        while (screen_row < max_y - 2 && logical_line < state->line_count) {
                char *line = state->lines[logical_line];
                int line_len = strlen(line);
                // print line number or ->
                if (show_line_numbers) {
                        attron(COLOR_PAIR(28) | A_BOLD);
                        if (offset_in_line == 0) {
                                mvprintw(screen_row, 0, "%5d ", logical_line + 1);
                        } else {
                                mvprintw(screen_row, 0, "  ->  ");
                        }
                        attroff(COLOR_PAIR(28) | A_BOLD);
                }
                // print text if not blank or not at end
                if (line_len > 0 && offset_in_line < line_len) {
                        int start = offset_in_line;
                        int end = start + avail_width;
                        if (end > line_len) end = line_len;
                        mvprintw(screen_row, text_start_col, "%.*s", end - start, line + start);
                        // update
                        if (end == line_len) {
                                logical_line++;
                                offset_in_line = 0;
                        } else {
                                offset_in_line = end;
                        }
                } else {
                        // blank line or end of line, move to next logical line
                        logical_line++;
                        offset_in_line = 0;
                }
                screen_row++;
        }


        
        // Calculate cursor visual position
        int cursor_visual_row = 3;
        int temp_logical = state->scroll_offset;
        while (temp_logical < state->cursor_y) {
                char *line = state->lines[temp_logical];
                int line_len = strlen(line);
                if (line_len == 0) {
                        cursor_visual_row += 1; // blank lines take 1 visual row
                } else {
                        int visual_rows = (line_len + avail_width - 1) / avail_width;
                        cursor_visual_row += visual_rows;
                }
                temp_logical++;
        }
        // add position in current line
        char *cursor_line = state->lines[state->cursor_y];
        int cursor_line_len = strlen(cursor_line);
        if (cursor_line_len == 0) {
                cursor_visual_row += 0; // cursor at start of blank line
        } else {
                int cursor_visual_in_line = state->cursor_x / avail_width;
                cursor_visual_row += cursor_visual_in_line;
        }
        int screen_cursor_col = text_start_col + (state->cursor_x % avail_width);

        int words = 0;
        for (int i = 0; i < state->line_count; i++) {
            char* line = state->lines[i];
            int in_word = 0;
            for (int j = 0; line[j] != '\0'; j++) {
                if (isspace((unsigned char)line[j])) {
                    in_word = 0;
                } else if (!in_word) {
                    in_word = 1;
                    words++;
                }
            }
        }

        attron(COLOR_PAIR(1) | A_BOLD);
        const char* syntax_status = (state->syntax_enabled ? "ON" : "OFF");
        const char* brackets_status = (state->auto_complete_enabled ? "ON" : "OFF");
        const char* comment_status = (state->comment_complete_enabled ? "ON" : "OFF");
        const char* edited_indicator = (state->dirty ? " [edited]" : "");
        mvprintw(max_y - 1, 0, "Line: %d, Col: %d | %s%s | Syntax HL: %s | Autocomplete: %s | Brackets and Quotes Autocomplete: %s | Auto Tabbing: %s | Words: %d",
                  state->cursor_y + 1, state->cursor_x + 1,
                  state->filename[0] ? state->filename : "[Untitled]",
                  edited_indicator,
                  syntax_status,
                  comment_status,
                  brackets_status,
                  state->auto_tabbing_enabled ? "ON" : "OFF",
                  words);
        attroff(COLOR_PAIR(1) | A_BOLD);

        move(cursor_visual_row, screen_cursor_col);
        refresh();
}

void find_text(EditorState* state)
{
         echo();
         curs_set(1);

         attron(COLOR_PAIR(1));

         int max_y, max_x;
         getmaxyx(stdscr, max_y, max_x);

         const char* prompt = "Find (letter/word): ";
         int prompt_len = strlen(prompt);
         mvprintw(max_y - 2, 0, "%s", prompt);
         clrtoeol();
         refresh();

         char search_term[256] = {0};
         int input_pos = 0;
         int cursor_x = prompt_len;
         move(max_y - 2, cursor_x);
         refresh();

         while (1) {
                 int ch = getch();
                 if (ch == '\n' || ch == KEY_ENTER) {
                         break;
                 } else if (ch == 27) {
                         search_term[0] = '\0';
                         break;
                 } else if (ch == KEY_LEFT) {
                         if (cursor_x > prompt_len) {
                                 cursor_x--;
                                 input_pos = cursor_x - prompt_len;
                         }
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_RIGHT) {
                         if (cursor_x < prompt_len + (int)strlen(search_term)) {
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
                         cursor_x = prompt_len + (int)strlen(search_term);
                         input_pos = cursor_x - prompt_len;
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_DC) {
                         if (input_pos < (int)strlen(search_term)) {
                                 memmove(&search_term[input_pos], &search_term[input_pos + 1], strlen(search_term) - input_pos);
                                 mvprintw(max_y - 2, 0, "%s%s ", prompt, search_term);
                                 move(max_y - 2, cursor_x);
                                 refresh();
                         }
                 } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt_len) {
                         int delete_pos = cursor_x - prompt_len - 1;
                         if (delete_pos >= 0) {
                                 memmove(&search_term[delete_pos], &search_term[delete_pos + 1], strlen(search_term) - delete_pos + 1);
                                 cursor_x--;
                                 input_pos = cursor_x - prompt_len;
                                 mvprintw(max_y - 2, 0, "%s%s ", prompt, search_term);
                                 move(max_y - 2, cursor_x);
                                 refresh();
                         }
                 } else if (ch == 22) {  
                         char* clipboard = get_system_clipboard();
                         if (clipboard) {
                                 int len = strlen(clipboard);
                                 for (int i = 0; i < len && input_pos < (int)sizeof(search_term) - 1; i++) {
                                         if (clipboard[i] != '\n') {
                                                 search_term[input_pos++] = clipboard[i];
                                                 mvprintw(max_y - 2, cursor_x++, "%c", clipboard[i]);
                                         }
                                 }
                                 free(clipboard);
                                 refresh();
                         }
                 } else if (isprint(ch) && input_pos < (int)sizeof(search_term) - 1 && cursor_x < max_x - 1) {
                         search_term[input_pos++] = ch;
                         mvprintw(max_y - 2, cursor_x++, "%c", ch);
                         refresh();
                 }
         }

         noecho();
         curs_set(0);

         attroff(COLOR_PAIR(1));

         if (strlen(search_term) == 0) {
                 return;
         }

         find_all_occurrences(state, search_term);
}

void find_all_occurrences(EditorState* state,
        const char* search_term)
{

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
                show_status(state, "The word is not present in this file");
                free(match_lines);
                free(match_positions);
                return;
        }

        int unique_lines[1024];
        int unique_count = 0;
        for(int j=0; j<match_count && unique_count < 1024; j++){
            int line = match_lines[j];
            int found = 0;
            for(int k=0; k<unique_count; k++){
                if(unique_lines[k] == line){ found=1; break; }
            }
            if(!found) unique_lines[unique_count++] = line;
        }
        char msg[512] = "Found ";
        strncat(msg, search_term, sizeof(msg) - strlen(msg) - 1);
        strncat(msg, " on the lines ", sizeof(msg) - strlen(msg) - 1);
        int len = strlen(msg);
        for(int j=0; j<unique_count; j++){
            char num[16];
            snprintf(num, sizeof(num), "%d", unique_lines[j] + 1);
            if(j>0) {
                if(len + 1 < sizeof(msg)) { msg[len++] = ';'; }
            }
            if(len + strlen(num) < sizeof(msg)) {
                strcpy(msg + len, num);
                len += strlen(num);
            }
        }
        show_status(state, msg);

        navigate_matches(state, search_term, match_lines, match_positions, match_count);

        free(match_lines);
        free(match_positions);
}

void navigate_matches(EditorState* state,
        const char* search_term, int* match_lines, int* match_positions, int match_count)
{
        static int current_match = 0;

        if (match_count == 0) return;

        current_match = 0;
        jump_to_match(state, match_lines[current_match], match_positions[current_match]);

        timeout(2000);

        while (1) {
                int ch = getch();

                if (ch == '\n' || ch == KEY_ENTER) {

                        current_match = (current_match + 1) % match_count;
                        jump_to_match(state, match_lines[current_match], match_positions[current_match]);
                } else if (ch == 27) {

                        break;
                } else if (ch == ERR) {

                        break;
                }
        }

        timeout(-1);
}

void jump_to_match(EditorState* state, int line_num, int position)
{

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

void replace_text(EditorState* state)
{
         echo();
         curs_set(1);

         attron(COLOR_PAIR(1));

         int max_y, max_x;
         getmaxyx(stdscr, max_y, max_x);


         const char* prompt1 = "Replace: ";
         int prompt1_len = strlen(prompt1);
         mvprintw(max_y - 2, 0, "%s", prompt1);
         clrtoeol();
         refresh();

         char search_term[256] = {0};
         int input_pos = 0;
         int cursor_x = prompt1_len;
         move(max_y - 2, cursor_x);
         refresh();

         while (1) {
                 int ch = getch();
                 if (ch == '\n' || ch == KEY_ENTER) {
                         break;
                 } else if (ch == 27) {
                         search_term[0] = '\0';
                         break;
                 } else if (ch == KEY_LEFT) {
                         if (cursor_x > prompt1_len) {
                                 cursor_x--;
                                 input_pos = cursor_x - prompt1_len;
                         }
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_RIGHT) {
                         if (cursor_x < prompt1_len + (int)strlen(search_term)) {
                                 cursor_x++;
                                 input_pos = cursor_x - prompt1_len;
                         }
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_HOME) {
                         cursor_x = prompt1_len;
                         input_pos = 0;
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_END) {
                         cursor_x = prompt1_len + (int)strlen(search_term);
                         input_pos = cursor_x - prompt1_len;
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_DC) {
                         if (input_pos < (int)strlen(search_term)) {
                                 memmove(&search_term[input_pos], &search_term[input_pos + 1], strlen(search_term) - input_pos);
                                 mvprintw(max_y - 2, 0, "%s%s ", prompt1, search_term);
                                 move(max_y - 2, cursor_x);
                                 refresh();
                         }
                 } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt1_len) {
                         int delete_pos = cursor_x - prompt1_len - 1;
                         if (delete_pos >= 0) {
                                 memmove(&search_term[delete_pos], &search_term[delete_pos + 1], strlen(search_term) - delete_pos + 1);
                                 cursor_x--;
                                 input_pos = cursor_x - prompt1_len;
                                 mvprintw(max_y - 2, 0, "%s%s ", prompt1, search_term);
                                 move(max_y - 2, cursor_x);
                                 refresh();
                         }
                 } else if (ch == 22) {  
                         char* clipboard = get_system_clipboard();
                         if (clipboard) {
                                 int len = strlen(clipboard);
                                 for (int i = 0; i < len && input_pos < (int)sizeof(search_term) - 1; i++) {
                                         if (clipboard[i] != '\n') {
                                                 search_term[input_pos++] = clipboard[i];
                                                 mvprintw(max_y - 2, cursor_x++, "%c", clipboard[i]);
                                         }
                                 }
                                 free(clipboard);
                                 refresh();
                         }
                 } else if (isprint(ch) && input_pos < (int)sizeof(search_term) - 1 && cursor_x < max_x - 1) {
                         search_term[input_pos++] = ch;
                         mvprintw(max_y - 2, cursor_x++, "%c", ch);
                         refresh();
                 }
         }

         if (strlen(search_term) == 0) {
                 noecho();
                 curs_set(0);
                 return;
         }

         
         int exists = 0;
         for (int i = 0; i < state->line_count; i++) {
                 if (strstr(state->lines[i], search_term)) {
                         exists = 1;
                         break;
                 }
         }

         if (!exists) {
                 noecho();
                 curs_set(0);
                 show_status(state, "Text to replace not found in document");
                 return;
         }

         
         const char* prompt2 = "With: ";
         int prompt2_len = strlen(prompt2);
         mvprintw(max_y - 2, 0, "%*s", max_x, "");
         mvprintw(max_y - 2, 0, "%s", prompt2);
         clrtoeol();
         refresh();

         char replace_term[256] = {0};
         input_pos = 0;
         cursor_x = prompt2_len;
         move(max_y - 2, cursor_x);
         refresh();

         while (1) {
                 int ch = getch();
                 if (ch == '\n' || ch == KEY_ENTER) {
                         break;
                 } else if (ch == 27) {
                         replace_term[0] = '\0';
                         break;
                 } else if (ch == KEY_LEFT) {
                         if (cursor_x > prompt2_len) {
                                 cursor_x--;
                                 input_pos = cursor_x - prompt2_len;
                         }
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_RIGHT) {
                         if (cursor_x < prompt2_len + (int)strlen(replace_term)) {
                                 cursor_x++;
                                 input_pos = cursor_x - prompt2_len;
                         }
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_HOME) {
                         cursor_x = prompt2_len;
                         input_pos = 0;
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_END) {
                         cursor_x = prompt2_len + (int)strlen(replace_term);
                         input_pos = cursor_x - prompt2_len;
                         move(max_y - 2, cursor_x);
                         refresh();
                 } else if (ch == KEY_DC) {
                         if (input_pos < (int)strlen(replace_term)) {
                                 memmove(&replace_term[input_pos], &replace_term[input_pos + 1], strlen(replace_term) - input_pos);
                                 mvprintw(max_y - 2, 0, "%s%s ", prompt2, replace_term);
                                 move(max_y - 2, cursor_x);
                                 refresh();
                         }
                 } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt2_len) {
                         int delete_pos = cursor_x - prompt2_len - 1;
                         if (delete_pos >= 0) {
                                 memmove(&replace_term[delete_pos], &replace_term[delete_pos + 1], strlen(replace_term) - delete_pos + 1);
                                 cursor_x--;
                                 input_pos = cursor_x - prompt2_len;
                                 mvprintw(max_y - 2, 0, "%s%s ", prompt2, replace_term);
                                 move(max_y - 2, cursor_x);
                                 refresh();
                         }
                 } else if (ch == 22) {  
                         char* clipboard = get_system_clipboard();
                         if (clipboard) {
                                 int len = strlen(clipboard);
                                 for (int i = 0; i < len && input_pos < (int)sizeof(replace_term) - 1; i++) {
                                         if (clipboard[i] != '\n') {
                                                 replace_term[input_pos++] = clipboard[i];
                                                 mvprintw(max_y - 2, cursor_x++, "%c", clipboard[i]);
                                         }
                                 }
                                 free(clipboard);
                                 refresh();
                         }
                 } else if (isprint(ch) && input_pos < (int)sizeof(replace_term) - 1 && cursor_x < max_x - 1) {
                         replace_term[input_pos++] = ch;
                         mvprintw(max_y - 2, cursor_x++, "%c", ch);
                         refresh();
                 }
         }

         noecho();
         curs_set(0);

         attroff(COLOR_PAIR(1));

         replace_text_simple(state, search_term, replace_term);
}

void replace_text_simple(EditorState* state,
        const char* search_term,
                const char* replace_term)
{

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
                snprintf(msg, sizeof(msg), "Replaced %s with %s", search_term, replace_term);
                show_status(state, msg);
        } else {
        }
}

void prompt_root_password(EditorState* state)
{
        echo();
        curs_set(1);

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        clear();
        attron(COLOR_PAIR(1));
        mvprintw(max_y / 2 - 3, max_x / 2 - 15, " Root password required ");
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

void init_theme_colors(EditorState* state)
{
    if (find_plugin_by_name(state, "Theme Plugin") != -1) return;

    use_default_colors();

    int t = state->theme_id;

    if (t < 0 || t > 3) t = 2;

    if (t == 0) {
        init_pair(1,  COLOR_BLACK,  -1);
        init_pair(6,  COLOR_BLACK,  -1);
        init_pair(13, COLOR_BLACK, -1);
        init_pair(28, COLOR_BLACK,  -1);

        init_pair(7, COLOR_YELLOW, -1);
        init_pair(8, COLOR_GREEN, -1);
        init_pair(9, COLOR_GREEN,  -1);
        init_pair(10, COLOR_RED,  -1);
        init_pair(11, COLOR_MAGENTA,  -1);
        init_pair(12, COLOR_MAGENTA,  -1);
        init_pair(14, COLOR_CYAN,  -1);
        init_pair(15, COLOR_BLACK,  -1);
        init_pair(16, COLOR_BLUE,  -1);
        init_pair(17, COLOR_RED,  -1);
        init_pair(18, COLOR_RED,  -1);
        init_pair(19, COLOR_WHITE, COLOR_RED);
        init_pair(20, COLOR_MAGENTA,  -1);
        init_pair(21, COLOR_MAGENTA,  -1);
        init_pair(22, COLOR_YELLOW,  -1);
        init_pair(23, COLOR_RED,  -1);
        init_pair(24, COLOR_GREEN,  -1);
        init_pair(25, COLOR_MAGENTA,  -1);
        init_pair(26, COLOR_BLUE,  -1);
        init_pair(27, COLOR_YELLOW,  -1);
    } else if (t == 1) {
        init_pair(1,  COLOR_WHITE,  -1);
        init_pair(6,  COLOR_WHITE,  -1);
        init_pair(13, COLOR_WHITE,  -1);
        init_pair(28, COLOR_WHITE,  -1);

        init_pair(7, COLOR_YELLOW,  -1);
        init_pair(8, COLOR_GREEN,  -1);
        init_pair(9, COLOR_GREEN,  -1);
        init_pair(10, COLOR_RED,  -1);
        init_pair(11, COLOR_MAGENTA,  -1);
        init_pair(12, COLOR_MAGENTA,  -1);
        init_pair(14, COLOR_CYAN,  -1);
        init_pair(15, COLOR_WHITE,  -1);
        init_pair(16, COLOR_BLUE,  -1);
        init_pair(17, COLOR_RED,  -1);
        init_pair(18, COLOR_RED,  -1);
        init_pair(19, COLOR_BLACK, COLOR_RED);
        init_pair(20, COLOR_MAGENTA,  -1);
        init_pair(21, COLOR_MAGENTA,  -1);
        init_pair(22, COLOR_YELLOW,  -1);
        init_pair(23, COLOR_RED,  -1);
        init_pair(24, COLOR_GREEN,  -1);
        init_pair(25, COLOR_MAGENTA,  -1);
        init_pair(26, COLOR_BLUE,  -1);
        init_pair(27, COLOR_YELLOW,  -1);
    } else if (t == 2) {
        init_pair(1,  COLOR_WHITE,  -1);
        init_pair(6,  COLOR_WHITE,  -1);
        init_pair(13, COLOR_WHITE,  -1);
        init_pair(28, COLOR_WHITE,    -1);

        init_pair(7, COLOR_YELLOW,  -1);
        init_pair(8, COLOR_GREEN,  -1);
        init_pair(9, COLOR_GREEN,  -1);
        init_pair(10, COLOR_RED,  -1);
        init_pair(11, COLOR_MAGENTA,  -1);
        init_pair(12, COLOR_MAGENTA,  -1);
        init_pair(14, COLOR_CYAN,  -1);
        init_pair(15, COLOR_WHITE,  -1);
        init_pair(16, COLOR_BLUE,  -1);
        init_pair(17, COLOR_RED,  -1);
        init_pair(18, COLOR_RED,  -1);
        init_pair(19, COLOR_BLACK, COLOR_RED);
        init_pair(20, COLOR_MAGENTA,  -1);
        init_pair(21, COLOR_MAGENTA,  -1);
        init_pair(22, COLOR_YELLOW,  -1);
        init_pair(23, COLOR_RED,  -1);
        init_pair(24, COLOR_GREEN,  -1);
        init_pair(25, COLOR_MAGENTA,  -1);
        init_pair(26, COLOR_BLUE,  -1);
        init_pair(27, COLOR_YELLOW,  -1);
        init_pair(29, COLOR_BLACK, COLOR_WHITE); // Selection highlight
    } else if (t == 3) {
        init_pair(1,  COLOR_WHITE,  -1);
        init_pair(6,  COLOR_WHITE,  -1);
        init_pair(13, COLOR_WHITE,  -1);
        init_pair(28, COLOR_WHITE,     -1);

        init_pair(7, COLOR_YELLOW,  -1);
        init_pair(8, COLOR_GREEN,  -1);
        init_pair(9, COLOR_GREEN,  -1);
        init_pair(10, COLOR_RED,  -1);
        init_pair(11, COLOR_MAGENTA,  -1);
        init_pair(12, COLOR_MAGENTA,  -1);
        init_pair(14, COLOR_CYAN,  -1);
        init_pair(15, COLOR_WHITE,  -1);
        init_pair(16, COLOR_BLUE,  -1);
        init_pair(17, COLOR_RED,  -1);
        init_pair(18, COLOR_RED,  -1);
        init_pair(19, COLOR_BLACK, COLOR_RED);
        init_pair(20, COLOR_MAGENTA,  -1);
        init_pair(21, COLOR_MAGENTA,  -1);
        init_pair(22, COLOR_YELLOW,  -1);
        init_pair(23, COLOR_RED,  -1);
        init_pair(24, COLOR_GREEN,  -1);
        init_pair(25, COLOR_MAGENTA,  -1);
        init_pair(26, COLOR_BLUE,  -1);
        init_pair(27, COLOR_YELLOW,  -1);
        init_pair(29, COLOR_BLACK, COLOR_WHITE); // Selection highlight
    }

}

void cycle_theme(EditorState* state)
{
    state->theme_id = 1;
    strncpy(state->theme_name, "Frappe", sizeof(state->theme_name)-1);
    state->theme_name[sizeof(state->theme_name)-1] = '\0';
    if (has_colors()) {
        start_color();
        init_theme_colors(state);
        clear();
        refresh();
    }
    save_config(state);


}

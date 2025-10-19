#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "editor.h"
#include "plugin.h"

void load_file(EditorState* state, const char* filename)
{


        FILE* file = fopen(filename, "r");
        if (!file) {

                file = fopen(filename, "w+");
                if (!file) {
                        show_status(state, "Error: Could not create file");
                        return;
                }
        }

        for (int i = 0; i < state -> line_count; i++) {
                free(state -> lines[i]);
        }

        state -> line_count = 0;

        
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (file_size > 0) {
                char * content = (char *) malloc(file_size + 1);
                if (!content) {
                        show_status(state, "Memory allocation failed for file content");
                        fclose(file);
                        return;
                }

                size_t read_size = fread(content, 1, file_size, file);
                content[read_size] = '\0';

                
                char * line_start = content;
                char * ptr = content;
                while (*ptr && state -> line_count < MAX_LINES) {
                        if (*ptr == '\n') {
                                *ptr = '\0';
                                state -> lines[state -> line_count] = (char * ) malloc(MAX_LINE_LENGTH);
                                if (!state -> lines[state -> line_count]) {
                                        show_status(state, "Memory allocation failed");
                                        free(content);
                                        fclose(file);
                                        return;
                                }
                                strncpy(state -> lines[state -> line_count], line_start, MAX_LINE_LENGTH - 1);
                                state -> lines[state -> line_count][MAX_LINE_LENGTH - 1] = '\0';
                                state -> line_count++;
                                line_start = ptr + 1;
                        }
                        ptr++;
                }

                
                if (line_start < ptr && state -> line_count < MAX_LINES) {
                        state -> lines[state -> line_count] = (char * ) malloc(MAX_LINE_LENGTH);
                        if (!state -> lines[state -> line_count]) {
                                show_status(state, "Memory allocation failed");
                                free(content);
                                fclose(file);
                                return;
                        }
                        strncpy(state -> lines[state -> line_count], line_start, MAX_LINE_LENGTH - 1);
                        state -> lines[state -> line_count][MAX_LINE_LENGTH - 1] = '\0';
                        state -> line_count++;
                }

                if (state->line_count > 0 && strlen(state->lines[state->line_count - 1]) > 0 && state->line_count < MAX_LINES) {
                        state->lines[state->line_count] = (char*)malloc(MAX_LINE_LENGTH);
                        if (state->lines[state->line_count]) {
                                state->lines[state->line_count][0] = '\0';
                                state->line_count++;
                        }
                }

                free(content);
        }

        fclose(file);

        if (state -> line_count == 0) {
                state -> lines[0] = (char * ) malloc(MAX_LINE_LENGTH);
                if (!state -> lines[0]) {
                        show_status(state, "Memory allocation failed");
                        return;
                }
                state -> lines[0][0] = '\0';
                state -> line_count = 1;
        }

        strcpy(state -> filename, filename);
        state -> cursor_x = 0;
        state -> cursor_y = 0;
        state -> scroll_offset = 0;

        
        save_original_content(state);
        update_dirty_status(state);

        detect_file_type(state);

        // Load config settings
        load_config(state);

        if (state->syntax_enabled) {
                init_syntax_highlighting(state);
        }


        call_plugin_file_load_hooks(state, filename);

}

void save_file(EditorState* state)
{
        if (state -> filename[0] == '\0') {

                prompt_filename(state);
                if (state -> filename[0] == '\0') {
                        return;
                }
        }

        FILE * file = fopen(state -> filename, "w");
        if (!file) {

                if (errno == EACCES || errno == EPERM) {
                        state -> needs_sudo = 1;
                        prompt_root_password(state);
                        return;
                }
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Error: Could not save file (%s)", strerror(errno));
                show_status(state, error_msg);
                return;
        }

        for (int i = 0; i < state -> line_count; i++) {
                if (fprintf(file, "%s\n", state -> lines[i]) < 0) {
                        char error_msg[256];
                        snprintf(error_msg, sizeof(error_msg), "Error: Failed to write to file (%s)", strerror(errno));
                        show_status(state, error_msg);
                        fclose(file);
                        return;
                }
        }

        if (fclose(file) != 0) {
                show_status(state, "Warning: File may not have been saved completely");
                return;
        }

        
        
        call_plugin_file_save_hooks(state, state->filename);

        save_original_content(state);
        update_dirty_status(state);
        state -> needs_sudo = 0;
}

void save_file_with_sudo(EditorState* state)
{
        if (state -> filename[0] == '\0') {
                show_status(state, "Error: No filename specified");
                return;
        }

        char temp_path[256];
        snprintf(temp_path, sizeof(temp_path), "/tmp/kilo_editor_temp_%d.txt", getpid());

        FILE * temp_file = fopen(temp_path, "w");
        if (!temp_file) {
                show_status(state, "Error: Could not create temporary file");
                return;
        }

        for (int i = 0; i < state -> line_count; i++) {
                fprintf(temp_file, "%s\n", state -> lines[i]);
        }
        fclose(temp_file);

        char command[512];
        snprintf(command, sizeof(command), "sudo cp %s %s", temp_path, state -> filename);

        int result = system(command);
        unlink(temp_path);

        if (result == 0) {

                save_original_content(state);
                update_dirty_status(state);
                state -> needs_sudo = 0;
        } else {
                show_status(state, "Error: Failed to save with sudo privileges");
        }
}

void prompt_filename(EditorState* state)
{

        echo();
        curs_set(1);

        attron(COLOR_PAIR(1));

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        const char *prompt = "Save file as: ";
        int prompt_len = strlen(prompt);


        mvprintw(max_y - 2, 0, "%s", prompt);
        clrtoeol();
        refresh();

        char filename[256] = {0};
        int input_pos = 0;      
        int cursor_x = prompt_len;

        move(max_y - 2, cursor_x);
        refresh();

        while (1) {
                int ch = getch();

                if (ch == '\n' || ch == KEY_ENTER) {
                        break;
                } else if (ch == 27) { 
                        filename[0] = '\0';
                        break;
                } else if (ch == KEY_LEFT) {
                        if (cursor_x > prompt_len) {
                                cursor_x--;
                                input_pos = cursor_x - prompt_len;
                        }
                        move(max_y - 2, cursor_x);
                        refresh();
                } else if (ch == KEY_RIGHT) {
                        if (cursor_x < prompt_len + (int)strlen(filename)) {
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
                        cursor_x = prompt_len + (int)strlen(filename);
                        input_pos = cursor_x - prompt_len;
                        move(max_y - 2, cursor_x);
                        refresh();
                } else if (ch == KEY_DC) {
                        
                        if (input_pos < (int)strlen(filename)) {
                                memmove(&filename[input_pos], &filename[input_pos + 1],
                                        strlen(filename) - input_pos);
                                mvprintw(max_y - 2, 0, "%s%s ", prompt, filename);
                                move(max_y - 2, cursor_x);
                                refresh();
                        }
                } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt_len) {
                        
                        int delete_pos = cursor_x - prompt_len - 1;
                        if (delete_pos >= 0) {
                                memmove(&filename[delete_pos], &filename[delete_pos + 1],
                                        strlen(filename) - delete_pos + 1);
                                cursor_x--;
                                input_pos = cursor_x - prompt_len;
                                mvprintw(max_y - 2, 0, "%s%s ", prompt, filename);
                                move(max_y - 2, cursor_x);
                                refresh();
                        }
                } else if (isprint(ch) && (int)strlen(filename) < (int)sizeof(filename) - 1 && cursor_x < max_x - 1) {
                        
                        int len = (int)strlen(filename);
                        if (input_pos <= len) {
                                memmove(&filename[input_pos + 1], &filename[input_pos], len - input_pos + 1);
                                filename[input_pos] = (char)ch;
                                input_pos++;
                                mvprintw(max_y - 2, 0, "%s%s ", prompt, filename);
                                cursor_x++;
                                move(max_y - 2, cursor_x);
                                refresh();
                        }
                }

                
                if (cursor_x < prompt_len) {
                        cursor_x = prompt_len;
                        input_pos = 0;
                        move(max_y - 2, cursor_x);
                        refresh();
                }
        }

        noecho();
        curs_set(0);

        attroff(COLOR_PAIR(1));

        if (strlen(filename) > 0) {
                strcpy(state->filename, filename);
        } else {
        }
}
static char* gui_select_file(void)
{
        return NULL;
}
void prompt_open_file(EditorState* state)
{
        if (state->dirty) {
                echo();
                curs_set(1);
                attron(COLOR_PAIR(1));
                int max_y, max_x;
                getmaxyx(stdscr, max_y, max_x);
                const char *prompt = "Unsaved changes. Save before opening? (y)es/(n)o/(c)ancel: ";
                mvprintw(max_y - 2, 0, "%s", prompt);
                clrtoeol();
                refresh();
                attroff(COLOR_PAIR(1));
                int ch;
                while (1) {
                        ch = getch();
                        if (ch == 'y' || ch == 'Y') {
                                noecho();
                                curs_set(0);
                                save_file(state);
                                break;
                        } else if (ch == 'n' || ch == 'N') {
                                noecho();
                                curs_set(0);
                                break;
                        } else if (ch == 'c' || ch == 'C' || ch == 27) {
                                noecho();
                                curs_set(0);
                                return;
                        }
                }
        }

        char* selected = gui_select_file();
        if (selected && selected[0] != '\0') {
                load_file(state, selected);
                free(selected);
                return;
        }
        if (selected) free(selected);


        echo();
        curs_set(1);

        attron(COLOR_PAIR(1));

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        const char *prompt = "Open file: ";
        int prompt_len = strlen(prompt);

        mvprintw(max_y - 2, 0, "%s", prompt);
        clrtoeol();
        refresh();

        char filename[256] = {0};
        int input_pos = 0;
        int cursor_x = prompt_len;

        move(max_y - 2, cursor_x);
        refresh();

        while (1) {
                int ch = getch();
                if (ch == '\n' || ch == KEY_ENTER) {
                        break;
                } else if (ch == 27) {
                        filename[0] = '\0';
                        break;
                } else if (ch == KEY_LEFT) {
                        if (cursor_x > prompt_len) {
                                cursor_x--;
                                input_pos = cursor_x - prompt_len;
                        }
                        move(max_y - 2, cursor_x);
                        refresh();
                } else if (ch == KEY_RIGHT) {
                        if (cursor_x < prompt_len + (int)strlen(filename)) {
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
                        cursor_x = prompt_len + (int)strlen(filename);
                        input_pos = cursor_x - prompt_len;
                        move(max_y - 2, cursor_x);
                        refresh();
                } else if (ch == KEY_DC) {
                        if (input_pos < (int)strlen(filename)) {
                                memmove(&filename[input_pos], &filename[input_pos + 1],
                                        strlen(filename) - input_pos);
                                mvprintw(max_y - 2, 0, "%s%s ", prompt, filename);
                                move(max_y - 2, cursor_x);
                                refresh();
                        }
                } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt_len) {
                        int delete_pos = cursor_x - prompt_len - 1;
                        if (delete_pos >= 0) {
                                memmove(&filename[delete_pos], &filename[delete_pos + 1],
                                        strlen(filename) - delete_pos + 1);
                                cursor_x--;
                                input_pos = cursor_x - prompt_len;
                                mvprintw(max_y - 2, 0, "%s%s ", prompt, filename);
                                move(max_y - 2, cursor_x);
                                refresh();
                        }
                } else if (isprint(ch) && (int)strlen(filename) < (int)sizeof(filename) - 1 && cursor_x < max_x - 1) {
                        int len = (int)strlen(filename);
                        if (input_pos <= len) {
                                memmove(&filename[input_pos + 1], &filename[input_pos], len - input_pos + 1);
                                filename[input_pos] = (char)ch;
                                input_pos++;
                                mvprintw(max_y - 2, 0, "%s%s ", prompt, filename);
                                cursor_x++;
                                move(max_y - 2, cursor_x);
                                refresh();
                        }
                }

                if (cursor_x < prompt_len) {
                        cursor_x = prompt_len;
                        input_pos = 0;
                        move(max_y - 2, cursor_x);
                        refresh();
                }
        }

        noecho();
        curs_set(0);

        attroff(COLOR_PAIR(1));

        if (strlen(filename) > 0) {
                load_file(state, filename);
        } else {
        }
}
void autosave_check(EditorState* state)
{
        if (!state) return;
        if (!state->autosave_enabled) return;
        if (state->filename[0] == '\0') return; 
        time_t now = time(NULL);
        if (state->dirty && difftime(now, state->last_autosave_time) >= state->autosave_interval_sec) {
                save_file(state);
                state->last_autosave_time = now;
        }
}

void toggle_autosave(EditorState* state)
{
        state->autosave_enabled = !state->autosave_enabled;
        state->last_autosave_time = time(NULL);
}
static void ensure_config_dir(char *out_dir, size_t out_sz)
{
        const char *home = getenv("HOME");
        if (!home) home = ".";
        snprintf(out_dir, out_sz, "%s/.config/root-editor", home);
        
        mkdir(out_dir, 0755);
}

static void get_config_path(char *out_path, size_t out_sz)
{
        char dir[512];
        ensure_config_dir(dir, sizeof(dir));
        snprintf(out_path, out_sz, "%s/config.ini", dir);
}

void load_config(EditorState* state)
{
        char path[512];
        get_config_path(path, sizeof(path));
        FILE *fp = fopen(path, "r");
        if (!fp) {
                // Create default config file if it doesn't exist
                save_config(state);
                return;
        }

        char line[512];
        while (fgets(line, sizeof(line), fp)) {

                size_t len = strlen(line);
                if (len && (line[len-1]=='\n' || line[len-1]=='\r')) line[len-1] = '\0';
                char key[64], val[256];
                if (sscanf(line, " %63[^=]=%255[^\n] ", key, val) == 2) {
                        if (strcmp(key, "tab_size")==0) {
                                int t = atoi(val);
                                if (t >= 1 && t <= 8) state->tab_size = t;
                        } else if (strcmp(key, "syntax_enabled")==0) {
                                state->syntax_enabled = atoi(val) ? 1 : 0;
                        } else if (strcmp(key, "autosave_enabled")==0) {
                                state->autosave_enabled = atoi(val) ? 1 : 0;
                        } else if (strcmp(key, "autosave_interval_sec")==0) {
                                int s = atoi(val);
                                if (s >= 5 && s <= 600) state->autosave_interval_sec = s;
                        } else if (strcmp(key, "auto_complete_enabled")==0) {
                                state->auto_complete_enabled = atoi(val) ? 1 : 0;
                        } else if (strcmp(key, "comment_complete_enabled")==0) {
                                state->comment_complete_enabled = atoi(val) ? 1 : 0;
                        } else if (strcmp(key, "auto_tabbing_enabled")==0) {
                                state->auto_tabbing_enabled = atoi(val) ? 1 : 0;
                        }
                }
        }
        fclose(fp);
}

void save_config(EditorState* state)
{
        char path[512];
        get_config_path(path, sizeof(path));

        char dir[512];
        ensure_config_dir(dir, sizeof(dir));

        FILE *fp = fopen(path, "w");
        if (!fp) {
                show_status(state, "Failed to save config");
                return;
        }
        fprintf(fp, "tab_size=%d\n", state->tab_size);
        fprintf(fp, "syntax_enabled=%d\n", state->syntax_enabled);
        fprintf(fp, "autosave_enabled=%d\n", state->autosave_enabled);
        fprintf(fp, "autosave_interval_sec=%d\n", state->autosave_interval_sec);
        fprintf(fp, "auto_complete_enabled=%d\n", state->auto_complete_enabled);
        fprintf(fp, "comment_complete_enabled=%d\n", state->comment_complete_enabled);
        fprintf(fp, "auto_tabbing_enabled=%d\n", state->auto_tabbing_enabled);
        fclose(fp);
}

void safe_quit(EditorState* state)
{
         if (!state->dirty) {

                  unload_all_plugins(state);

                  printf("\033[?2004l"); fflush(stdout);
                  endwin();
                  exit(0);
         }
        echo();
        curs_set(1);
        attron(COLOR_PAIR(1));
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        const char *prompt = "Unsaved changes. Save before exit? (y)es/(n)o/(c)ancel: ";
        mvprintw(max_y - 2, 0, "%s", prompt);
        clrtoeol();
        refresh();
        attroff(COLOR_PAIR(1));
        int ch;
        while (1) {
                ch = getch();
                if (ch == 'y' || ch == 'Y') {
                        noecho();
                        curs_set(0);
                        save_file(state);
                        
                        printf("\033[?2004l"); fflush(stdout);
                        endwin();
                        exit(0);
                } else if (ch == 'n' || ch == 'N') {
                        noecho();
                        curs_set(0);

                        
                        unload_all_plugins(state);

                        printf("\033[?2004l"); fflush(stdout);
                        endwin();
                        exit(0);
                } else if (ch == 'c' || ch == 'C' || ch == 27) {
                        noecho();
                        curs_set(0);
                        return;
                }
        }
}
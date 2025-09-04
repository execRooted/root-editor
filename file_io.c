#include "editor.h"



void load_file(EditorState * state,
        const char * filename) {
        FILE * file = fopen(filename, "r");
        if (!file) {

                file = fopen(filename, "w+");
                if (!file) {
                        show_status(state, "Error: Could not create file");
                        return;
                }
                show_status(state, "New file created");
        } else {
                show_status(state, "File loaded successfully");
        }

        for (int i = 0; i < state -> line_count; i++) {
                free(state -> lines[i]);
        }

        state -> line_count = 0;
        char buffer[MAX_LINE_LENGTH];

        while (fgets(buffer, MAX_LINE_LENGTH, file) && state -> line_count < MAX_LINES) {

                size_t len = strlen(buffer);
                if (len > 0 && buffer[len - 1] == '\n') {
                        buffer[len - 1] = '\0';
                }

                state -> lines[state -> line_count] = (char * ) malloc(MAX_LINE_LENGTH);
                if (!state -> lines[state -> line_count]) {
                        show_status(state, "Memory allocation failed");
                        return;
                }
                strcpy(state -> lines[state -> line_count], buffer);
                state -> line_count++;
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
        state -> dirty = 0;
        state -> cursor_x = 0;
        state -> cursor_y = 0;
        state -> scroll_offset = 0;

        show_status(state, "File loaded successfully");
}

void save_file(EditorState * state) {
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

        state -> dirty = 0;
        state -> needs_sudo = 0;
        show_status(state, "File saved successfully");
}

void save_file_with_sudo(EditorState * state) {
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
                state -> dirty = 0;
                state -> needs_sudo = 0;
                show_status(state, "File saved with sudo privileges");
        } else {
                show_status(state, "Error: Failed to save with sudo privileges");
        }
}

void prompt_filename(EditorState * state) {

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        clear();

        attron(COLOR_PAIR(1));
        mvprintw(max_y / 2 - 2, max_x / 2 - 20, " Save File As ");
        attroff(COLOR_PAIR(1));

        mvprintw(max_y / 2, max_x / 2 - 25, "Enter filename (or press Esc to cancel): ");
        refresh();

        echo();
        curs_set(1);

        char filename[256] = {
                0
        };
        int ch;
        int pos = 0;

        while (pos < sizeof(filename) - 1) {
                ch = getch();

                if (ch == '\n' || ch == KEY_ENTER) {

                        filename[pos] = '\0';
                        if (strlen(filename) > 0) {
                                strcpy(state -> filename, filename);
                                show_status(state, "File name set successfully");
                        } else {
                                show_status(state, "Cancelled - no filename entered");
                        }
                        break;
                } else if (ch == 27) {

                        show_status(state, "Save cancelled");
                        filename[0] = '\0';
                        break;
                } else if (ch == KEY_BACKSPACE || ch == 127) {

                        if (pos > 0) {
                                pos--;
                                mvprintw(max_y / 2 + 1, max_x / 2 - 25 + pos, " ");
                                move(max_y / 2 + 1, max_x / 2 - 25 + pos);
                        }
                } else if (isprint(ch)) {

                        filename[pos++] = ch;
                        addch(ch);
                }
        }

        noecho();
        curs_set(0);

        mvprintw(max_y / 2 + 1, max_x / 2 - 25, "%*s", 50, "");
}

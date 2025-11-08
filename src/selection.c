#include "editor.h"
#include <string.h>
#include <stdlib.h>



void select_all(EditorState* state)
{
        if (state -> line_count == 0) return;

        state -> select_mode = 1;
        state -> select_start_x = 0;
        state -> select_start_y = 0;
        state -> select_end_x = strlen(state -> lines[state -> line_count - 1]);
        state -> select_end_y = state -> line_count - 1;
        state -> cursor_x = state -> select_end_x;
        state -> cursor_y = state -> select_end_y;
}

void select_current_line(EditorState* state)
{
        if (state -> cursor_y < 0 || state -> cursor_y >= state -> line_count) return;

        state -> select_mode = 1;
        state -> select_start_x = 0;
        state -> select_start_y = state -> cursor_y;
        state -> select_end_x = strlen(state -> lines[state -> cursor_y]);
        state -> select_end_y = state -> cursor_y;
}

void copy_selected_text(EditorState* state)
{
        if (!state -> select_mode) {
                return;
        }

        int start_y = state -> select_start_y;
        int end_y = state -> select_end_y;
        int start_x = state -> select_start_x;
        int end_x = state -> select_end_x;

        char temp_file[] = "/tmp/kilo_editor_clipboard_temp.txt";
        FILE * fp = fopen(temp_file, "w");
        if (!fp) {
                show_status(state, "Failed to create temporary file for clipboard");
                return;
        }

        for (int i = start_y; i <= end_y; i++) {
                int line_len = strlen(state -> lines[i]);
                int line_start = (i == start_y) ? start_x : 0;
                int line_end = (i == end_y) ? end_x : line_len;
                int copy_len = line_end - line_start;

                if (copy_len > 0) {
                        fprintf(fp, "%.*s", copy_len, state -> lines[i] + line_start);
                }

                if (i < end_y) {
                        fprintf(fp, "\n");
                }
        }
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

void clear_selection(EditorState* state)
{
        state -> select_mode = 0;
        state -> select_start_x = 0;
        state -> select_start_y = 0;
        state -> select_end_x = 0;
        state -> select_end_y = 0;
}

void start_selection(EditorState* state)
{
        state -> select_mode = 1;
        state -> select_start_x = state -> cursor_x;
        state -> select_start_y = state -> cursor_y;

        
        state -> select_end_x = state -> cursor_x;
        state -> select_end_y = state -> cursor_y;
}

void extend_selection(EditorState* state)
{
        if (!state->select_mode) return;

        int anchor_x = state->select_start_x;
        int anchor_y = state->select_start_y;
        int cur_x = state->cursor_x;
        int cur_y = state->cursor_y;

        int forward = (cur_y > anchor_y) || (cur_y == anchor_y && cur_x >= anchor_x);


        state->select_end_x = cur_x;
        state->select_end_y = cur_y;


        if (state->select_start_y > state->select_end_y ||
            (state->select_start_y == state->select_end_y &&
             state->select_start_x > state->select_end_x)) {
            int temp_x = state->select_start_x;
            int temp_y = state->select_start_y;
            state->select_start_x = state->select_end_x;
            state->select_start_y = state->select_end_y;
            state->select_end_x = temp_x;
            state->select_end_y = temp_y;



            forward = 0;
        }


        if (forward) {
            int end_line_len = 0;
            if (state->select_end_y >= 0 && state->select_end_y < state->line_count && state->lines[state->select_end_y]) {
                end_line_len = (int)strlen(state->lines[state->select_end_y]);
            }
            int ex = state->select_end_x + 1;
            if (ex > end_line_len) ex = end_line_len;
            state->select_end_x = ex;
        }
}

int has_selection(EditorState* state)
{
        return state -> select_mode &&
                (state -> select_start_y != state -> select_end_y ||
                        state -> select_start_x != state -> select_end_x);
}

char* get_selected_text(EditorState* state)
{
        if (!has_selection(state)) return NULL;

        int start_y = state -> select_start_y;
        int end_y = state -> select_end_y;
        int start_x = state -> select_start_x;
        int end_x = state -> select_end_x;

        size_t total_length = 0;
        for (int i = start_y; i <= end_y; i++) {
                int line_len = strlen(state -> lines[i]);
                int line_start = (i == start_y) ? start_x : 0;
                int line_end = (i == end_y) ? end_x : line_len;
                total_length += (line_end - line_start);
                if (i < end_y) total_length++;
        }

        char * selected_text = malloc(total_length + 1);
        if (!selected_text) return NULL;
        int pos = 0;
        for (int i = start_y; i <= end_y; i++) {
                int line_len = strlen(state -> lines[i]);
                int line_start = (i == start_y) ? start_x : 0;
                int line_end = (i == end_y) ? end_x : line_len;
                int copy_len = line_end - line_start;

                if (copy_len > 0) {
                        strncpy(selected_text + pos, state -> lines[i] + line_start, copy_len);
                        pos += copy_len;
                }

                if (i < end_y) {
                        selected_text[pos++] = '\n';
                }
        }
        selected_text[pos] = '\0';

        return selected_text;
}

void delete_selected_text(EditorState* state)
{
        if (!has_selection(state)) return;

        int start_y = state -> select_start_y;
        int end_y = state -> select_end_y;
        int start_x = state -> select_start_x;
        int end_x = state -> select_end_x;

        if (start_y == end_y)
        {
                char * line = state -> lines[start_y];
                memmove( & line[start_x], & line[end_x], strlen(line) - end_x + 1);
                state -> cursor_x = start_x;
                state -> cursor_y = start_y;
        }
        else
        {
                char * first_line = state -> lines[start_y];
                first_line[start_x] = '\0';

                char * last_line = state -> lines[end_y];
                char * remaining_text = (char * ) malloc(MAX_LINE_LENGTH);
                if (!remaining_text) return;
                strcpy(remaining_text, & last_line[end_x]);

                int lines_to_remove = end_y - start_y;
                for (int i = start_y + 1; i <= end_y; i++) {
                        free(state -> lines[i]);
                }

                for (int i = start_y + 1; i < state -> line_count - lines_to_remove; i++) {
                        state -> lines[i] = state -> lines[i + lines_to_remove];
                }

                state -> line_count -= lines_to_remove;

                if (strlen(first_line) + strlen(remaining_text) < MAX_LINE_LENGTH) {
                        strcat(first_line, remaining_text);
                }

                state -> cursor_x = start_x;
                state -> cursor_y = start_y;
                free(remaining_text);
        }

        state -> dirty = 1;

        while (state -> line_count > 1 && strlen(state -> lines[state -> line_count - 1]) == 0) {
                free(state -> lines[state -> line_count - 1]);
                state -> line_count--;
        }

        if (state -> cursor_y >= state -> line_count) {
                state -> cursor_y = state -> line_count - 1;
        }
        if (state -> cursor_x > (int) strlen(state -> lines[state -> cursor_y])) {
                state -> cursor_x = strlen(state -> lines[state -> cursor_y]);
        }

        clear_selection(state);
        
}

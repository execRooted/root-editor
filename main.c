#include "editor.h"
#include <unistd.h>
#include <time.h>


void set_window_title() {
        printf("\033]0;root-editor\007");
        fflush(stdout);
}

int main(int argc, char * argv[]) {
         if (argc < 2) {
                 fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
                 return 1;
         }
         EditorState state;
         init_editor( & state);
         load_file( & state, argv[1]);
         init_syntax_highlighting( & state);
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        curs_set(1);
        set_window_title();
        if (has_colors()) {
                start_color();

                init_pair(1, COLOR_BLACK, COLOR_WHITE);
                init_pair(6, COLOR_BLACK, COLOR_WHITE);
                init_pair(15, COLOR_WHITE, COLOR_BLACK);

                // Syntax highlighting color pairs (updated to match sintax.json)
                init_pair(7, COLOR_YELLOW, COLOR_BLACK);   // COLOR_DATA_TYPE
                init_pair(8, COLOR_GREEN, COLOR_BLACK);    // COLOR_STRING
                init_pair(9, COLOR_WHITE, COLOR_BLACK);    // COLOR_COMMENT
                init_pair(10, COLOR_YELLOW, COLOR_BLACK);  // COLOR_INTEGER_LITERAL
                init_pair(11, COLOR_MAGENTA, COLOR_BLACK); // COLOR_MODIFIER
                init_pair(12, COLOR_MAGENTA, COLOR_BLACK); // COLOR_PREPROCESSOR
                init_pair(13, COLOR_WHITE, COLOR_BLACK);   // COLOR_DEFAULT
                init_pair(14, COLOR_CYAN, COLOR_BLACK);    // COLOR_OPERATOR
                init_pair(15, COLOR_WHITE, COLOR_BLACK);   // COLOR_DELIMITER
                init_pair(16, COLOR_BLUE, COLOR_BLACK);    // COLOR_FUNCTION
                init_pair(17, COLOR_YELLOW, COLOR_BLACK);  // COLOR_CONSTANT
                init_pair(18, COLOR_RED, COLOR_BLACK);     // COLOR_ERROR
                init_pair(19, COLOR_BLACK, COLOR_RED);     // COLOR_ERROR_BG
                init_pair(20, COLOR_MAGENTA, COLOR_BLACK); // COLOR_CONTROL_FLOW
                init_pair(21, COLOR_MAGENTA, COLOR_BLACK); // COLOR_STORAGE_CLASS
                init_pair(22, COLOR_YELLOW, COLOR_BLACK);  // COLOR_USER_TYPE
                init_pair(23, COLOR_YELLOW, COLOR_BLACK);  // COLOR_FLOAT_LITERAL
                init_pair(24, COLOR_GREEN, COLOR_BLACK);   // COLOR_CHAR_LITERAL
                init_pair(25, COLOR_MAGENTA, COLOR_BLACK); // COLOR_POINTER_REF
                init_pair(26, COLOR_BLUE, COLOR_BLACK);    // COLOR_STDLIB_FUNCTION
                init_pair(27, COLOR_YELLOW, COLOR_BLACK);  // COLOR_STDLIB_TYPE

        }
        int ch;
        while (1) {
                if (state.show_help) {
                        render_help_screen( & state);
                } else {
                        render_screen( & state);
                }
                ch = getch();
                handle_input( & state, ch);
                refresh();
        }
        endwin();
        for (int i = 0; i < state.line_count; i++) {
                free(state.lines[i]);
        }
        free(state.lines);
        return 0;
        
}
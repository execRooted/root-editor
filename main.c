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
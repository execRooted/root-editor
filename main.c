#include "editor.h"
#include <unistd.h>
#include <time.h>


static EditorState * global_state = NULL;

void set_window_title() {
        printf("\033]0;root-editor\007");
        fflush(stdout);
}

void signal_handler(int sig) {
        endwin();
        if (global_state && global_state -> filename[0] != '\0') {
                char emergency_path[256];
                snprintf(emergency_path, sizeof(emergency_path), "%s.emergency", global_state -> filename);
                FILE * emergency = fopen(emergency_path, "w");
                if (emergency) {
                        for (int i = 0; i < global_state -> line_count; i++) {
                                fprintf(emergency, "%s\n", global_state -> lines[i]);
                        }
                        fclose(emergency);
                        fprintf(stderr, "Emergency save created: %s\n", emergency_path);
                }
        }
        fprintf(stderr, "Editor crashed with signal %d. Emergency save may be available.\n", sig);
        exit(1);
}

void setup_signal_handlers() {
        signal(SIGSEGV, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGBUS, signal_handler);
        signal(SIGILL, signal_handler);
        signal(SIGFPE, signal_handler);
}

int main(int argc, char * argv[]) {
        setup_signal_handlers();
        EditorState state;
        global_state = & state;
        init_editor( & state);
        if (argc > 1) {
                load_file( & state, argv[1]);
                init_syntax_highlighting( & state);
        }
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

                // Syntax highlighting colors
                init_pair(7, COLOR_BLUE, COLOR_BLACK);      // data types
                init_pair(8, COLOR_YELLOW, COLOR_BLACK);    // strings
                init_pair(9, COLOR_GREEN, COLOR_BLACK);     // comments
                init_pair(10, COLOR_YELLOW, COLOR_BLACK);   // numbers
                init_pair(11, COLOR_YELLOW, COLOR_BLACK);   // preprocessor
                init_pair(12, COLOR_BLUE, COLOR_BLACK);     // keywords
                init_pair(13, COLOR_WHITE, COLOR_BLACK);    // default
                init_pair(14, COLOR_CYAN, COLOR_BLACK);     // operators
                init_pair(15, COLOR_CYAN, COLOR_BLACK);     // characters and line numbers
                init_pair(16, COLOR_YELLOW, COLOR_BLACK);   // functions
                init_pair(17, COLOR_MAGENTA, COLOR_BLACK);  // constants
                init_pair(18, COLOR_MAGENTA, COLOR_BLACK);  // booleans
                init_pair(19, COLOR_CYAN, COLOR_BLACK);     // brackets
                init_pair(20, COLOR_MAGENTA, COLOR_BLACK);  // control flow
                init_pair(26, COLOR_YELLOW, COLOR_BLACK);   // stdlib functions
        }
        int ch;
        while (1) {
                autosave_check( & state);
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
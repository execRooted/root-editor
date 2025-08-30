#include "editor.h"
#include <unistd.h>
#include <time.h>


static jmp_buf recovery_buffer;
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
        free(state.clipboard);
        return 0;
        
}
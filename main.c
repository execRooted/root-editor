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

                init_color(30, 306, 788, 690);
                init_color(31, 180, 761, 494);
                init_color(32, 773, 525, 753);
                init_color(33, 337, 612, 839);
                init_color(34, 843, 729, 490);
                init_color(35, 1000, 722, 424);
                init_color(36, 1000, 843, 439);
                init_color(37, 1000, 1000, 1000);
                init_color(38, 529, 808, 980);
                init_color(39, 1000, 533, 0);
                init_color(40, 1000, 361, 553);
                init_color(41, 820, 604, 400);
                init_color(42, 596, 765, 475);
                init_color(43, 416, 600, 333);
                init_color(44, 957, 278, 278);
                init_color(45, 533, 533, 533);
                init_color(46, 1000, 475, 776);
                init_color(47, 0, 808, 820);
                init_color(48, 498, 1000, 831);

                init_pair(1, COLOR_BLACK, COLOR_WHITE);
                init_pair(2, COLOR_CYAN, COLOR_BLACK);
                init_pair(3, COLOR_GREEN, COLOR_BLACK);
                init_pair(4, COLOR_YELLOW, COLOR_BLACK);
                init_pair(5, COLOR_RED, COLOR_BLACK);
                init_pair(6, COLOR_BLACK, COLOR_WHITE);

                init_pair(7, 30, COLOR_BLACK);
                init_pair(8, 42, COLOR_BLACK);
                init_pair(9, 43, COLOR_BLACK);
                init_pair(10, 39, COLOR_BLACK);
                init_pair(11, 31, COLOR_BLACK);
                init_pair(12, 34, COLOR_BLACK);
                init_pair(13, 37, COLOR_BLACK);
                init_pair(14, 44, COLOR_BLACK);
                init_pair(15, 45, COLOR_BLACK);
                init_pair(16, 36, COLOR_BLACK);
                init_pair(17, 35, COLOR_BLACK);
                init_pair(18, COLOR_RED, COLOR_WHITE);
                init_pair(19, COLOR_BLACK, COLOR_RED);
                init_pair(20, 33, COLOR_BLACK);
                init_pair(21, 32, COLOR_BLACK);
                init_pair(22, 38, COLOR_BLACK);
                init_pair(23, 40, COLOR_BLACK);
                init_pair(24, 41, COLOR_BLACK);
                init_pair(25, 46, COLOR_BLACK);
                init_pair(26, 47, COLOR_BLACK);
                init_pair(27, 48, COLOR_BLACK);
        }
        int ch;
        while (1) {
                autosave_check( & state);
                if (state.show_help) {
                        render_help_screen( & state);
                } else {
                        static time_t last_error_check = 0;
                        time_t current_time = time(NULL);
                        if (current_time - last_error_check >= 2) {
                                detect_syntax_errors( & state);
                                last_error_check = current_time;
                        }
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
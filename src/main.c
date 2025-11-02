#include "editor.h"
#include "plugin.h"
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>

volatile sig_atomic_t resized = 0;

static void enable_bracketed_paste(void)
{
        printf("\033[?2004h");
        fflush(stdout);
}
static void disable_bracketed_paste(void)
{
        printf("\033[?2004l");
        fflush(stdout);
}

static void handle_resize(int sig)
{
        resized = 1;
}

void set_window_title()
{
        printf("\033]0;root-editor\007");
        fflush(stdout);
}


void show_welcome_screen()
{
        clear();
        refresh();

        attron(COLOR_PAIR(1));

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);


        const char* root_editor_art[] = {
            "88888888ba",
            "88      \"8b",
            "88      ,8P",
            "88aaaaaa8P' ,adPPYba,",
            "88\"\"\"\"88'  a8P_____88",
            "88    `8b  8PP\"\"\"\"\"\"\"",
            "88     `8b \"8b,",
            "88      `8b `\"Ybbd8\"'"
            ""
        };

        int art_lines = sizeof(root_editor_art) / sizeof(root_editor_art[0]);

        int start_y = 13;

        int max_art_len = 0;
        for (int i = 0; i < art_lines; i++) {
            int len = strlen(root_editor_art[i]);
            if (len > max_art_len) max_art_len = len;
        }
        int art_start_x = (max_x - max_art_len) / 2;
        if (art_start_x < 0) art_start_x = 0;
        for (int i = 0; i < art_lines; i++) {
            mvprintw(start_y + i, art_start_x, root_editor_art[i]);
        }

        int text_start_y = start_y + art_lines + 1;
        int text_width = strlen("root-editor by execRooted");
        int start_x = (max_x - text_width) / 2;
        if (start_x < 0) start_x = 0;
        mvprintw(text_start_y, start_x, "root-editor by execRooted");

        int info_start_y = text_start_y + 2;
        if (info_start_y < max_y - 8) {
                mvprintw(info_start_y++, (max_x - 20) / 2, "Usage: re [filename]");
                mvprintw(info_start_y++, (max_x - 20) / 2, "");
                mvprintw(info_start_y++, (max_x - 25) / 2, "       Navigation:");
                mvprintw(info_start_y++, (max_x - 20) / 2, "");
                mvprintw(info_start_y++, (max_x - 15) / 2, "Ctrl+Q - Quit");
                mvprintw(info_start_y++, (max_x - 15) / 2, "Ctrl+S - Save");
                mvprintw(info_start_y++, (max_x - 25) / 2, "Arrow Keys - Move cursor");
                mvprintw(info_start_y++, (max_x - 10) / 2, "F1 - Help");
                mvprintw(info_start_y++, (max_x - 30) / 2, "");
                mvprintw(info_start_y++, (max_x - 40) / 2, "Press G for my Github, Enter to continue...");
        }

        attroff(COLOR_PAIR(1));
        refresh();

        timeout(-1);
        int ch;
        do {
            ch = getch();
            if (ch == 'g' || ch == 'G') {
                system("xdg-open https://github.com/execRooted");
            }
        } while (ch != '\n' && ch != KEY_ENTER);
        timeout(0);
}

int main(int argc, char * argv[])
{
         

         EditorState state;
         init_editor(&state);


         load_config(&state);

         
         initscr();
         raw();
         keypad(stdscr, TRUE);
         noecho();
         curs_set(1);

         signal(SIGWINCH, handle_resize);

         mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
         mouseinterval(0);
         set_window_title();
         if (has_colors()) {
                 start_color();
                 use_default_colors();
                 init_theme_colors(&state);
         }

         
         if (argc >= 2) {
                 load_file(&state, argv[1]);
         } else {
                 show_welcome_screen();
                 
                 for (int i = 0; i < state.line_count; i++) {
                         free(state.lines[i]);
                 }
                 free(state.lines);
                 free_original_content(&state);
                 
                 unload_all_plugins(&state);
                 endwin();
                 exit(0);
         }
init_syntax_highlighting(&state);

enable_bracketed_paste();

         int ch;
         while (1) {
                 
                 curs_set(1);

                 if (state.show_help) {
                         render_help_screen( & state);
                 } else {
                         render_screen( & state);
                         
                         call_plugin_render_hooks(&state);
                 }

                 

                 
                 time_t now = time(NULL);

                 static time_t last_syntax_update = 0;
                 if (state.rapid_input_mode && now - last_syntax_update >= 1) {
                         update_syntax_highlighting(&state);
                         last_syntax_update = now;
                 }

                 if (resized) {
                         resized = 0;
                         endwin();
                         refresh();
                         clear();
                 }

                 ch = getch();


                 call_plugin_keypress_hooks(&state, ch);

                 handle_input( & state, ch);

                 
                 time_t current_time = time(NULL);
                 double time_since_last_input = difftime(current_time, state.last_input_time);

                 
                 if (time_since_last_input < 0.1) {
                     state.rapid_input_mode = 1;
                 } else {
                     state.rapid_input_mode = 0;
                 }
                 state.last_input_time = current_time;

                 
                 
                 if (!state.rapid_input_mode) {
                     update_syntax_highlighting(&state);
                 }

                 
                 if (state.show_help) {
                         render_help_screen( & state);
                 } else {
                         render_screen( & state);
                 }

                 
                 curs_set(1);
                 refresh();
         }
         
         
         call_plugin_quit_hooks(&state);

         disable_bracketed_paste();
         endwin();
         for (int i = 0; i < state.line_count; i++) {
                 free(state.lines[i]);
         }
         free(state.lines);
         
         free_original_content(&state);
         return 0;


}




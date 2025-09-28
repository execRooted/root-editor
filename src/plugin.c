#include "plugin.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>

int load_plugin(EditorState* state, const char* plugin_path) {
    if (state->plugin_count >= MAX_PLUGINS) {
        return -1;
    }

    
    for (int i = 0; i < state->plugin_count; i++) {
        if (strcmp(state->plugins[i].path, plugin_path) == 0) {
            return -1;
        }
    }

    
    void* handle = dlopen(plugin_path, RTLD_LAZY);
    if (!handle) {
        return -1;
    }

    
    PluginInterface* (*get_interface)(void) = dlsym(handle, "get_plugin_interface");
    if (!get_interface) {
        dlclose(handle);
        return -1;
    }

    PluginInterface* interface = get_interface();
    if (!interface) {
        dlclose(handle);
        return -1;
    }

    
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].interface &&
            state->plugins[i].interface->name &&
            interface->name &&
            strcmp(state->plugins[i].interface->name, interface->name) == 0) {
            dlclose(handle);
            return -1;
        }
    }

    int index = state->plugin_count;
    state->plugins[index].handle = handle;
    state->plugins[index].interface = interface;
    strncpy(state->plugins[index].path, plugin_path, sizeof(state->plugins[index].path) - 1);
    state->plugins[index].loaded = 1;

    state->plugin_count++;

    
    if (interface->on_load) {
        interface->on_load(state);
    }

    return index;
}

void unload_plugin(EditorState* state, int plugin_index) {
    if (plugin_index < 0 || plugin_index >= state->plugin_count) {
        return;
    }

    Plugin* plugin = &state->plugins[plugin_index];

    
    if (plugin->interface && plugin->interface->on_unload) {
        plugin->interface->on_unload(state);
    }

    
    if (plugin->handle) {
        dlclose(plugin->handle);
    }

    
    for (int i = plugin_index; i < state->plugin_count - 1; i++) {
        state->plugins[i] = state->plugins[i + 1];
    }
    state->plugin_count--;
}

void unload_all_plugins(EditorState* state) {
    for (int i = state->plugin_count - 1; i >= 0; i--) {
        unload_plugin(state, i);
    }
}

void list_plugins(EditorState* state) {
    if (state->plugin_count == 0) {
        show_status_left(state, "No plugins loaded");
        return;
    }

    char msg[512] = "Loaded plugins: ";
    int len = strlen(msg);

    for (int i = 0; i < state->plugin_count; i++) {
        const char* name = state->plugins[i].interface->name ?
                          state->plugins[i].interface->name : "Unknown";
        if (len + strlen(name) + 2 < sizeof(msg)) {
            if (i > 0) {
                strcat(msg, ", ");
                len += 2;
            }
            strcat(msg, name);
            len += strlen(name);
        }
    }

    show_status_left(state, msg);
}

int find_plugin_by_name(EditorState* state, const char* name) {
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].interface &&
            state->plugins[i].interface->name &&
            strcmp(state->plugins[i].interface->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void call_plugin_keypress_hooks(EditorState* state, int ch) {
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].loaded &&
            state->plugins[i].interface &&
            state->plugins[i].interface->on_keypress) {
            state->plugins[i].interface->on_keypress(state, ch);
        }
    }
}

void call_plugin_render_hooks(EditorState* state) {
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].loaded &&
            state->plugins[i].interface &&
            state->plugins[i].interface->on_render) {
            state->plugins[i].interface->on_render(state);
        }
    }
}

void call_plugin_file_load_hooks(EditorState* state, const char* filename) {
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].loaded &&
            state->plugins[i].interface &&
            state->plugins[i].interface->on_file_load) {
            state->plugins[i].interface->on_file_load(state, filename);
        }
    }
}

void call_plugin_file_save_hooks(EditorState* state, const char* filename) {
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].loaded &&
            state->plugins[i].interface &&
            state->plugins[i].interface->on_file_save) {
            state->plugins[i].interface->on_file_save(state, filename);
        }
    }
}

void call_plugin_quit_hooks(EditorState* state) {
    for (int i = 0; i < state->plugin_count; i++) {
        if (state->plugins[i].loaded &&
            state->plugins[i].interface &&
            state->plugins[i].interface->on_quit) {
            state->plugins[i].interface->on_quit(state);
        }
    }
}

void load_plugins_from_directory(EditorState* state, const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        return;
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        
        if (strstr(entry->d_name, ".so") != NULL) {
            char plugin_path[512];
            snprintf(plugin_path, sizeof(plugin_path), "%s/%s", dir_path, entry->d_name);

            
            int already_loaded = 0;
            for (int i = 0; i < state->plugin_count; i++) {
                if (strcmp(state->plugins[i].path, plugin_path) == 0) {
                    already_loaded = 1;
                    break;
                }
            }

            if (!already_loaded) {
                load_plugin(state, plugin_path);
            }
        }
    }

    closedir(dir);
}

void autodetect_and_load_plugins(EditorState* state) {
    char home_plugins_path[512] = {0};
    const char* home = getenv("HOME");
    if (home) {
        snprintf(home_plugins_path, sizeof(home_plugins_path), "%s/.config/root-editor/plugins", home);
    }

    const char* plugin_search_paths[] = {
        "./plugins",
        "plugins",
        "../plugins",
        "/usr/local/lib/root-editor/plugins",
        home_plugins_path[0] ? home_plugins_path : NULL,
        NULL
    };

    int total_loaded = 0;

    
    for (int i = 0; plugin_search_paths[i] != NULL; i++) {
        DIR* dir = opendir(plugin_search_paths[i]);
        if (!dir) {
            continue; 
        }

        struct dirent* entry;
        int path_loaded = 0;

        while ((entry = readdir(dir)) != NULL) {
            
            if (strstr(entry->d_name, ".so") != NULL) {
                char plugin_path[512];
                snprintf(plugin_path, sizeof(plugin_path), "%s/%s",
                        plugin_search_paths[i], entry->d_name);

                
                int already_loaded = 0;
                for (int j = 0; j < state->plugin_count; j++) {
                    if (strcmp(state->plugins[j].path, plugin_path) == 0) {
                        already_loaded = 1;
                        break;
                    }
                }

                if (!already_loaded) {
                    int result = load_plugin(state, plugin_path);
                    if (result >= 0) {
                        path_loaded++;
                        total_loaded++;
                    }
                }
            }
        }

        closedir(dir);

        
    }

    
}

void load_plugin_interactive(EditorState* state) {
    echo();
    curs_set(1);

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    const char* prompt = "Load plugin: ";
    int prompt_len = strlen(prompt);

    mvprintw(max_y - 2, 0, "%s", prompt);
    clrtoeol();
    refresh();

    char plugin_path[256] = {0};
    int input_pos = 0;
    int cursor_x = prompt_len;

    move(max_y - 2, cursor_x);
    refresh();

    while (1) {
        int ch = getch();

        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        } else if (ch == 27) { 
            plugin_path[0] = '\0';
            break;
        } else if (ch == KEY_LEFT) {
            if (cursor_x > prompt_len) {
                cursor_x--;
                input_pos = cursor_x - prompt_len;
            }
            move(max_y - 2, cursor_x);
            refresh();
        } else if (ch == KEY_RIGHT) {
            if (cursor_x < prompt_len + (int)strlen(plugin_path)) {
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
            cursor_x = prompt_len + (int)strlen(plugin_path);
            input_pos = cursor_x - prompt_len;
            move(max_y - 2, cursor_x);
            refresh();
        } else if (ch == KEY_DC) {
            if (input_pos < (int)strlen(plugin_path)) {
                memmove(&plugin_path[input_pos], &plugin_path[input_pos + 1],
                        strlen(plugin_path) - input_pos);
                mvprintw(max_y - 2, 0, "%s%s ", prompt, plugin_path);
                move(max_y - 2, cursor_x);
                refresh();
            }
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && cursor_x > prompt_len) {
            int delete_pos = cursor_x - prompt_len - 1;
            if (delete_pos >= 0) {
                memmove(&plugin_path[delete_pos], &plugin_path[delete_pos + 1],
                        strlen(plugin_path) - delete_pos + 1);
                cursor_x--;
                input_pos = cursor_x - prompt_len;
                mvprintw(max_y - 2, 0, "%s%s ", prompt, plugin_path);
                move(max_y - 2, cursor_x);
                refresh();
            }
        } else if (isprint(ch) && (int)strlen(plugin_path) < (int)sizeof(plugin_path) - 1 && cursor_x < max_x - 1) {
            int len = (int)strlen(plugin_path);
            if (input_pos <= len) {
                memmove(&plugin_path[input_pos + 1], &plugin_path[input_pos], len - input_pos + 1);
                plugin_path[input_pos] = (char)ch;
                input_pos++;
                mvprintw(max_y - 2, 0, "%s%s ", prompt, plugin_path);
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

    if (strlen(plugin_path) > 0) {
        load_plugin(state, plugin_path);
    }
}
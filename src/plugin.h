#ifndef PLUGIN_H
#define PLUGIN_H

#include <dlfcn.h>



#include "editor.h"




void call_plugin_keypress_hooks(EditorState* state, int ch);
void call_plugin_render_hooks(EditorState* state);
void call_plugin_file_load_hooks(EditorState* state, const char* filename);
void call_plugin_file_save_hooks(EditorState* state, const char* filename);
void call_plugin_quit_hooks(EditorState* state);


void load_plugins_from_directory(EditorState* state, const char* dir_path);
void autodetect_and_load_plugins(EditorState* state);


void load_plugin_interactive(EditorState* state);


#endif
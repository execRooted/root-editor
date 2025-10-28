# Root Editor

> Made and tested on Arch Linux and Debian-based(Tails more specificly), should work fine in all major distros


<img src="github/picture0.png" width="500" alt="Root Editor Logo">



## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/execRooted/root-editor.git
   ```

2. Navigate to the project directory:
   ```bash
   cd root-editor
   ```


3. Run the installer for system-wide installation:
   ```bash
   sudo ./scripts/install.sh
   ```

If installed system-wide, simply run:

```bash
re [filename]
```

or 

```bash
root-editor [filename]
```



## Features

- **Syntax Highlighting**: Supports highlighting for various programming languages.
- **Themes**: Customizable color schemes for a personalized editing experience.
- **Autosave**: Automatic saving to prevent data loss.
- **Plugin Support**: Extensible architecture allowing users to add custom functionality via shared libraries.
- **Cross-Platform**: Built for Linux systems with ncurses.
- **Efficient**: Optimized C code with fast rendering and input handling.



## Keybinds

### Control Keys
- **Ctrl+Q**: Quit
- **Ctrl+S**: Save
- **Ctrl+O**: Open file
- **Ctrl+A**: Select all
- **Ctrl+B**: Select line
- **Ctrl+W**: Select word
- **Ctrl+X**: Cut
- **Ctrl+C**: Copy
- **Ctrl+V**: Paste
- **Ctrl+F**: Find
- **Ctrl+R**: Replace
- **Ctrl+L**: Jump to line
- **Ctrl+T**: Auto Indent
- **Ctrl+K**: Auto Complete
- **Ctrl+U**: Comment Complete
- **Ctrl+H**: Help

### Function Keys
- **F1**: Help
- **F2**: Find
- **F3**: Replace
- **F4**: Cut
- **F5**: Copy
- **F6**: Paste
- **F7**: Word Wrap
- **F8**: Syntax HL
- **F9**: Autosave
- **F10**: Syntax Disp



## Plugins

Root Editor features a powerful and flexible plugin system that allows developers to extend the editor's functionality through shared libraries (.so files on Linux). Plugins can hook into various editor events, enabling custom features such as enhanced rendering, specialized key handling, file processing, syntax highlighting extensions, and much more. This modular architecture makes Root Editor highly customizable and extensible, allowing users to tailor the editor to their specific workflow and needs.

Plugins are loaded dynamically at runtime and can access the full editor state, including the current file content, cursor position, selection, and configuration settings. The plugin system is designed to be safe and stable, with proper error handling to prevent crashes from malfunctioning plugins.

### Plugin Architecture

Plugins interact with the editor through a well-defined interface consisting of hook functions that are called at specific points during editor operation. Each plugin must export a `PluginInterface` structure that describes the plugin and provides function pointers for the hooks it implements.

### Available Hook Functions

The following hooks are available for plugins to implement:

- **`on_load`**: Called when the plugin is first loaded. Use this to initialize plugin-specific data structures, allocate memory, or set up initial state. Receives the `EditorState*` as parameter.

- **`on_unload`**: Called when the plugin is unloaded (e.g., when the editor quits or the plugin is manually unloaded). Use this to clean up resources, free memory, and perform any necessary teardown. Receives the `EditorState*` as parameter.

- **`on_keypress`**: Called for every key press before the default key handling. Return a non-zero value to indicate that the key was handled by the plugin and should not be processed by the default handler. Receives `EditorState*` and the key code `int ch`.

- **`on_render`**: Called during screen rendering, after the main content is drawn but before the final refresh. Use this to add custom UI elements, overlays, or status information. Receives the `EditorState*` as parameter.

- **`on_file_load`**: Called after a file has been successfully loaded into the editor. Receives `EditorState*` and the `const char* filename`.

- **`on_file_save`**: Called before a file is saved. This allows plugins to modify content or perform preprocessing. Receives `EditorState*` and the `const char* filename`.

- **`on_quit`**: Called when the editor is about to quit. Use this for final cleanup or saving plugin state. Receives the `EditorState*` as parameter.

- **`on_highlight_line`**: Called for each line during syntax highlighting. Allows plugins to implement custom highlighting rules. Receives `EditorState*`, line number `int`, screen row `int`, line number width `int`, and horizontal scroll offset `int`.

### Creating a Plugin

Follow these steps to create your own plugin:

1. **Create the plugin source file**: Create a new `.c` file in the `plugins/` directory (e.g., `my_plugin.c`).

2. **Include necessary headers**:
   ```c
   #include "../src/editor.h"
   #include <stdlib.h>  // For memory management
   #include <string.h>  // For string operations
   // Include other standard headers as needed
   ```

3. **Implement hook functions**: Define the hook functions you want to use. Each hook receives an `EditorState*` parameter that gives access to the full editor state. For example:
   ```c
   static void my_on_render(EditorState* state) {
       // Access editor state
       int cursor_y = state->cursor_y;
       int cursor_x = state->cursor_x;

       // Add custom rendering logic here
       // Use ncurses functions like mvprintw(), attron(), etc.
   }

   static int my_on_keypress(EditorState* state, int ch) {
       if (ch == 'm') {  // Example: handle 'm' key
           // Custom key handling
           return 1;  // Return 1 to prevent default handling
       }
       return 0;  // Return 0 to allow default handling
   }
   ```

4. **Define the PluginInterface struct**: Create a static instance of the plugin interface:
   ```c
   static PluginInterface plugin_interface = {
       .name = "My Custom Plugin",
       .version = "1.0.0",
       .description = "A plugin that adds custom functionality to Root Editor",
       .on_load = NULL,           // Set to your function or NULL
       .on_unload = NULL,         // Set to your function or NULL
       .on_keypress = my_on_keypress,
       .on_render = my_on_render,
       .on_file_load = NULL,
       .on_file_save = NULL,
       .on_quit = NULL,
       .on_highlight_line = NULL
   };
   ```

5. **Export the interface**: Provide the required `get_plugin_interface` function:
   ```c
   PluginInterface* get_plugin_interface(void) {
       return &plugin_interface;
   }
   ```

6. **Compile the plugin**: Use the provided Makefile to build your plugin:
   ```bash
   cd plugins/
   make
   ```
   This will generate a `.so` file with the same name as your source file.

### Accessing Editor State

Plugins receive a pointer to the `EditorState` struct, which contains all the editor's current state. Key fields include:

- `char** lines`: Array of strings representing file content
- `int line_count`: Number of lines in the file
- `int cursor_x, cursor_y`: Current cursor position
- `char filename[256]`: Current file name
- `int select_mode`: Whether text is selected
- `int select_start_x, select_start_y, select_end_x, select_end_y`: Selection boundaries
- Various configuration flags like `syntax_enabled`, `autosave_enabled`, etc.

**Important**: Always check for NULL pointers and validate array bounds when accessing editor state to prevent crashes.

### Loading and Managing Plugins

- **Automatic loading**: Plugins in the `plugins/` directory are automatically loaded when the editor starts.
- **Manual loading**: Use the interactive plugin loader (accessible via editor commands) to load plugins from custom locations.
- **System-wide installation**: After building new plugins, run `sudo ./scripts/install.sh` to install them system-wide.
- **Plugin management**: The editor provides commands to list, load, and unload plugins dynamically.

### Best Practices and Safety

- **Error handling**: Always check return values and handle errors gracefully. Malfunctioning plugins should not crash the editor.
- **Memory management**: Properly allocate and free memory. Avoid memory leaks by cleaning up in `on_unload`.
- **Thread safety**: The plugin system is not thread-safe. Avoid using threads in plugins.
- **Performance**: Keep hook functions efficient, especially `on_render` and `on_keypress`, as they are called frequently.
- **Compatibility**: Test plugins with different file types and editor configurations.
- **Documentation**: Clearly document your plugin's functionality and requirements.

### Example Plugin

See `plugins/word_count_plugin.c` for a complete example that displays word count in the status bar. This plugin demonstrates:

- Accessing selected text or entire file content
- Using ncurses functions for rendering
- Proper memory management
- Integration with the editor's rendering pipeline

### Debugging Plugins

- Use `printf` or logging functions for debugging (output goes to terminal)
- Test plugins in a separate terminal session to see error messages
- Check the editor's status messages for plugin loading errors
- Use gdb to debug plugin code: `gdb re` then load a file and trigger plugin hooks

**Note**: Plugins have full access to editor internals. Write plugins responsibly to maintain editor stability and security.



## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes. Ensure your code follows the existing style and includes appropriate tests.

## License

This project is licensed under the MIT License. 
It's open source like all things should be.


---

***Made by execRooted***

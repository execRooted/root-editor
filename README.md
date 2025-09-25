# Root Editor

A terminal-based text editor written in C with ncurses and a plugin system.

<img src="github/logo.png" width="500">




## Project Structure

```
root-editor/
├── src/                  # Source code files
│   ├── main.c            # Main entry point
│   ├── editor.c          # Core editor functionality
│   ├── editor.h          # Main header file
│   ├── file_io.c         # File operations
│   ├── input_handling.c  # Input processing
│   ├── rendering.c       # Screen rendering
│   ├── selection.c       # Text selection
│   ├── syntax.c          # Syntax highlighting
│   ├── plugin.c          # Plugin system implementation
│   └── plugin.h          # Plugin interface definitions
├── build/                # Build artifacts
│   ├── editor            # Compiled executable
│   └── obj/              # Object files
├── plugins/              # Plugin directory
│   ├── test_plugin.c     # Example plugin
│   ├── test_plugin.so    # Compiled plugin
│   └── Makefile          # Plugin build system
├── config/               # Configuration files
│   └── sintax.json       # Syntax highlighting rules
├── scripts/              # Installation scripts
│   ├── install.sh        # Installation script
│   └── uninstall.sh      # Uninstallation script
├── github/               # GitHub assets
│   └── logo.png          # Project logo
└── Makefile              # Main build system
```

## Installation

Install system-wide:
```
sudo ./scripts/install.sh
```

Uninstall:
```
sudo ./scripts/uninstall.sh
```

## Usage

Run the editor:
```
root-editor [filename]
```

## Features

- Syntax highlighting for multiple languages
- Auto-completion for brackets and quotes
- Auto-tabbing
- Find & replace
- Line numbers
- Word wrap
- Mouse support
- Autosave
- Plugin system
- Multiple themes

## Keyboard Shortcuts


- `F1`: Help
- `Ctrl+Q`: Quit
- `Ctrl+S`: Save
- `Ctrl+O`: Open file
- `Ctrl+F`: Find
- `Ctrl+R`: Replace
- `Ctrl+A`: Select all
- `Ctrl+X/C/V`: Cut/Copy/Paste
- `F1`: Help
- `F8`: Toggle word wrap
- `F9`: Toggle syntax highlighting
- `F10`: Toggle autosave


## How to Create a Plugin

1. Create a new `.c` file in the `plugins/` directory, e.g., `my_plugin.c`.

2. Include the plugin header:
   ```c
   #include "../src/plugin.h"
   ```

3. Define your hook functions, e.g.:
   ```c
   static int my_on_keypress(EditorState* state, int ch) {
       if (ch == 'a') {
           // Do something
           return 0; 
       }
       return 0;
   }
   ```

4. Define the plugin interface:
   ```c
   static PluginInterface plugin_interface = {
       .name = "My Plugin",
       .version = "1.0",
       .description = "Description",
       .on_keypress = my_on_keypress,
       // Set other hooks as needed
   };
   ```

5. Implement the required function:
   ```c
   PluginInterface* get_plugin_interface(void) {
       return &plugin_interface;
   }
   ```

6. Build the plugin:
   ```
   cd plugins && make
   ```

The plugin will be loaded automatically on editor startup.
***You will need to run the instalation script again for the plugins to take effect, for a system-wide instalation***


## License

Open source. As all things should be.  
This has been a journey;

---

***Made by execRooted***
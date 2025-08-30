# root-editor

A terminal-based text editor written in C using ncurses. Designed for efficient text editing with a focus on reliability and user experience.

## Features

- **Full Terminal Interface**: Uses ncurses for a rich terminal-based editing experience
- **Undo/Redo**: Multi-level undo and redo functionality (up to 100 states)
- **Autosave**: Automatic saving with configurable intervals
- **Text Selection**: Visual text selection
- **Clipboard Operations**: Cut, copy, and paste with system clipboard integration
- **Find & Replace**: Powerful search and replace functionality
- **Line Numbers**: Display line numbers for better navigation
- **Word Wrap**: Configurable word wrapping
- **Emergency Save**: Automatic emergency saves on crashes
- **Help System**: Built-in help screen with keybindings
- **Linux Support**: Optimized for Linux systems

## Installation

### Quick Install (Recommended)

1. Clone or download the repository

```
git clone https://guthub.com/execRooted/root-editor.git
```


2. Navigate to the project directory

```
cd root-editor
```

3. Run the installer with root privileges:

```
chmod +x installer.sh
```

```bash
sudo ./install.sh
```

The installer will:
- Detect your package manager (apt, yum, dnf, pacman, zypper)
- Install required dependencies (gcc, cmake, make, ncurses)
- Build the editor
- Install system-wide
- Create command aliases

### Manual Installation

If you prefer manual installation:

1. Install dependencies:


    ##### Ubuntu/Debian
    ```bash
    sudo apt-get install gcc cmake make libncurses5-dev libncursesw5-dev
    ```
    ##### CentOS/RHEL/Fedora
    ```
    sudo yum install gcc cmake make ncurses-devel  # or dnf
    ```
    ##### Arch Linux
    ```
    sudo pacman -S gcc cmake make ncurses
    ```
    ##### openSUSE
    ```
    sudo zypper install gcc cmake make ncurses-devel
    ```

2. Build the editor:
    ```bash
    make
    ```

3. Install manually:
    ```bash
    sudo cp editor /usr/local/bin/root-editor
    ```
    ```
    sudo chmod 755 /usr/local/bin/root-editor
    ```

## Usage

### Basic Usage


 ###### Launch
```bash
root-editor
```
 
 ###### Open a file
```
root-editor filename.txt
```

##### Using the alias
```
re filename.txt
```

### Keybindings

```
Keyboard Shortcuts:
Ctrl+Q        - Quit
Ctrl+S        - Save file
Ctrl+A        - Select all
Ctrl+X        - Cut selected text
Ctrl+Shift+C  - Copy to clipboard
Ctrl+Shift+V  - Paste from clipboard
Ctrl+Z        - Undo last line addition
Ctrl+Y        - Redo
Ctrl+F        - Find
Ctrl+L        - Jump to line
Ctrl+M        - Make new line
Ctrl+H        - Toggle this help


Function Keys:
F1            - Toggle help
F2            - Toggle word wrap
F3            - Find all occurrences (shows line numbers)
F4            - Replace text
F5            - Show statistics
F6            - Cut current line
F7            - Copy editor cliboard current line
F8            - Paste editor clipboard line
F9            - Undo
F10           - Redo
F11           - Toggle autosave
F12           - Toggle syntax highlighting


Navigation:
Arrow Keys    - Move cursor
Backspace     - Delete character
Enter         - New line
Tab           - Insert spaces


File Operations:
./editor <filename> - Open/create file
Files are created automatically if they don't exist
```

## Build from Source

To build from source manually:


##### Clone the repository
```bash
git clone <repository-url>
cd root-editor
```


##### Build
```
make
```

##### Clean build files
```
make clean
```

##### Run
```
./editor
```

## Dependencies

- **gcc**: C compiler
- **make**: Build system
- **ncurses**: Terminal UI library
- **cmake**: Build configuration (optional)

## Configuration

The editor includes several configurable options:

- **Tab Size**: Default 4 spaces (configurable in code)
- **Autosave Interval**: Configurable timing for automatic saves
- **Color Scheme**: Advanced color support for comprehensive syntax highlighting
- **Multi-Language Syntax Highlighting**: Automatic syntax highlighting for multiple languages (toggle with F12)
  - **C/C++** (.c, .cpp, .cc, .cxx files)
  - **C#** (.cs files)
  - **Rust** (.rs files)
  - **Python** (.py files)

- **Syntax Elements** (colors may vary by language):
  - 🔵 **Keywords**: Language-specific keywords (Blue)
  - 🔴 **Data Types**: int, string, bool, etc. (Red)
  - 🟢 **Comments**: //, /* */, #, etc. (Green)
  - 🟣 **Strings**: "text" and 'c' (Magenta)
  - 🟡 **Numbers**: 42, 3.14, 0xFF (Yellow)
  - 🟦 **Operators**: +, -, *, /, =, ==, &&, ||, etc. (Cyan)
  - 🟡 **Brackets**: (), [], {} (Yellow)
  - 🟢 **Functions**: function_name() (Green)
  - 🟣 **Constants**: true, false, NULL, None (Magenta)
  - 🔴 **Preprocessor/Special**: #include, #define, using, etc. (Red)

- **Word Wrap**: Toggle word wrapping on/off


## Troubleshooting

### Common Issues

1. **Missing Dependencies**: Run `sudo ./install.sh` to auto-install dependencies
2. **Terminal Colors**: Ensure your terminal supports colors
3. **Clipboard Issues**: Install `xclip`, `wl-clipboard`, or `xsel` for system clipboard

### Emergency Recovery

If the editor crashes, it automatically creates emergency save files with `.emergency` extension in the same directory as the original file.

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Support

For issues, questions, or contributions:

- Create an issue on GitHub
- Check the built-in help (`F1` or `Ctrl+H` in the editor)
- Review the source code for implementation details

---


**Root-Editor** - ***Made by execRooted***

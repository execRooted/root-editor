#!/bin/bash


set -e


RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' 



clear
echo "========================================="
echo "    ROOT-EDITOR INSTALLER                "
echo "========================================="
echo


if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}[ERROR]${YELLOW} This script needs to be run as root ${NC}"
    echo
    echo
    exit 1
fi


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"


cd "$PROJECT_DIR"


echo -e "${YELLOW}[WARNING]${NC} This will install Root-Editor system-wide."
echo
read -p "Continue with installation? (Y/n): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]] && [[ ! -z $REPLY ]]; then
    echo "Installation cancelled."
    exit 0
fi

clear
echo -e "${BLUE}[INFO]${NC} Starting installation process..."


detect_package_manager() {
    if command -v pacman &> /dev/null; then
        echo "pacman"
    elif command -v apt &> /dev/null; then
        echo "apt"
    elif command -v yum &> /dev/null; then
        echo "yum"
    elif command -v dnf &> /dev/null; then
        echo "dnf"
    elif command -v zypper &> /dev/null; then
        echo "zypper"
    else
        echo "unknown"
    fi
}

PKG_MANAGER=$(detect_package_manager)
echo -e "${BLUE}[INFO]${NC} Detected package manager: $PKG_MANAGER"

install_dependencies() {
    case $PKG_MANAGER in
        pacman)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with pacman..."
            sudo pacman -S --needed --noconfirm gcc make ncurses
            ;;
        apt)
            echo -e "${BLUE}[INFO]${NC} Updating package list and installing dependencies with apt..."
            sudo apt update
            sudo apt install -y build-essential libncurses-dev
            ;;
        yum)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with yum..."
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y ncurses-devel
            ;;
        dnf)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with dnf..."
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y ncurses-devel
            ;;
        zypper)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with zypper..."
            sudo zypper install -y -t pattern devel_basis
            sudo zypper install -y ncurses-devel
            ;;
        *)
            echo -e "${RED}[ERROR]${NC} Unsupported package manager. Please install dependencies manually:"
            echo "  - GCC compiler (gcc)"
            echo "  - Make build system"
            echo "  - Ncurses development libraries (libncurses-dev, ncurses-devel, etc.)"
            exit 1
            ;;
    esac
}

check_dependencies() {
    clear
    echo -e "${BLUE}[INFO]${NC} Checking for required dependencies..."

    local missing_deps=()

    if ! command -v gcc &> /dev/null; then
        missing_deps+=("gcc")
    fi

    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi

    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo -e "${YELLOW}[WARNING]${NC} Missing dependencies: ${missing_deps[*]}"
        echo -e "${BLUE}[INFO]${NC} Installing missing dependencies..."
        install_dependencies
    else
        echo -e "${GREEN}[OK]${NC} All basic dependencies found."
    fi

    echo -e "${BLUE}[INFO]${NC} Ensuring ncurses development libraries are available..."
    install_dependencies
}

check_dependencies

clear

echo -e "${BLUE}[INFO]${NC} Building Root-Editor..."
export PATH=/usr/bin:/bin:/usr/local/bin:$PATH

echo -e "${BLUE}[INFO]${NC} Cleaning up any build artifacts..."
rm -rf build_new 2>/dev/null || true
/usr/bin/make clean
clear
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Make clean failed."
    exit 1
fi
/usr/bin/make
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Make failed."
    exit 1
fi
clear

if [ ! -f "build/editor" ]; then
    echo -e "${RED}[ERROR]${NC} Build failed. Editor binary not found at build/editor"
    exit 1
fi

echo -e "${BLUE}[INFO]${NC} Build successful."

clear
echo -e "${BLUE}[INFO]${NC} Building plugins..."
cd plugins
/usr/bin/make clean
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Plugin make clean failed."
    exit 1
fi
/usr/bin/make
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Plugin make failed."
    exit 1
fi
cd ..

clear

echo -e "${BLUE}[INFO]${NC} Installing binary to /usr/local/bin..."
sudo cp build/editor /usr/local/bin/root-editor
sudo chmod +x /usr/local/bin/root-editor
sudo ln -sf /usr/local/bin/root-editor /usr/local/bin/re


echo -e "${BLUE}[INFO]${NC} Installing plugins..."
sudo mkdir -p /usr/local/lib/root-editor/plugins

echo -e "${BLUE}[INFO]${NC} Removing existing plugins..."
sudo rm -f /usr/local/lib/root-editor/plugins/*.so
if [ -f "plugins/theme_plugin.so" ]; then
    sudo cp plugins/theme_plugin.so /usr/local/lib/root-editor/plugins/
fi


USER_PLUGINS_DIR="$HOME/.config/root-editor/plugins"
mkdir -p "$USER_PLUGINS_DIR"
rm -f "$USER_PLUGINS_DIR"/*.so 2>/dev/null || true
if [ -f "plugins/theme_plugin.so" ]; then
    cp plugins/theme_plugin.so "$USER_PLUGINS_DIR/"
fi



sudo mkdir -p /usr/local/share/root-editor
sudo cp -r config/* /usr/local/share/root-editor/ 2>/dev/null || true

clear

echo -e "${BLUE}[INFO]${NC} Creating desktop entry..."
sudo tee /usr/share/applications/root-editor.desktop > /dev/null << EOF
[Desktop Entry]
Name=Root Editor
Comment=A C terminal-based text editor
Exec=re %f
Icon=terminal
Terminal=true
Type=Application
Categories=Utility;TextEditor;
MimeType=text/plain;
EOF

clear

echo -e "${GREEN}[SUCCESS]${NC} Root-Editor has been installed successfully!"
echo
echo "You can now run 'root-editor' or 're' from anywhere, or use the desktop entry."
echo "Plugins are installed in /usr/local/lib/root-editor/plugins/"
echo
echo -e "${YELLOW}[NOTE]${NC} Make sure /usr/local/bin is in your PATH if it's not already."
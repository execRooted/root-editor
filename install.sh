#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' 



clear
echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}    ROOT-EDITOR INSTALLER                ${NC}"
echo -e "${BLUE}=========================================${NC}"
echo


if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}[ERROR]${YELLOW} This script needs to be run as root ${NC}"
    echo
    echo
    exit 1
fi


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


cd "$SCRIPT_DIR"


echo -e "${YELLOW}[WARNING]${NC} This will install root-editor system-wide."
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
            sudo pacman -S --needed --noconfirm gcc cmake ncurses
            ;;
        apt)
            echo -e "${BLUE}[INFO]${NC} Updating package list and installing dependencies with apt..."
            sudo apt update
            sudo apt install -y build-essential libncurses-dev cmake
            ;;
        yum)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with yum..."
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y ncurses-devel cmake
            ;;
        dnf)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with dnf..."
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y ncurses-devel cmake
            ;;
        zypper)
            echo -e "${BLUE}[INFO]${NC} Installing dependencies with zypper..."
            sudo zypper install -y -t pattern devel_basis
            sudo zypper install -y ncurses-devel cmake
            ;;
        *)
            echo -e "${RED}[ERROR]${NC} Unsupported package manager. Please install dependencies manually:"
            echo "  - GCC compiler (gcc)"
            echo "  - CMake build system"
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

    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
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

echo -e "${BLUE}[INFO]${NC} Building root-editor..."
export PATH=/usr/bin:/bin:/usr/local/bin:$PATH

echo -e "${BLUE}[INFO]${NC} Cleaning up any build artifacts..."
rm -rf build
cmake -S . -B build
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} CMake configure failed."
    exit 1
fi
cmake --build build
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} CMake build failed."
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
rm -rf build
cmake -S . -B build
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Plugin CMake configure failed."
    exit 1
fi
cmake --build build
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Plugin CMake build failed."
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
sudo cp plugins/build/*.so /usr/local/lib/root-editor/plugins/ 2>/dev/null || true


USER_PLUGINS_DIR="$HOME/.config/root-editor/plugins"
mkdir -p "$USER_PLUGINS_DIR"
rm -f "$USER_PLUGINS_DIR"/*.so 2>/dev/null || true
cp plugins/build/*.so "$USER_PLUGINS_DIR/" 2>/dev/null || true



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

echo -e "${GREEN}[SUCCESS]${NC} root-editor has been installed successfully!"
echo
echo "You can now run 'root-editor' or 're' from anywhere, or use the desktop entry."
echo "Plugins are installed in /usr/local/lib/root-editor/plugins/"
echo
echo -e "${YELLOW}[NOTE]${NC} Make sure /usr/local/bin is in your PATH if it's not already."


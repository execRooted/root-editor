#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

clear

echo -e "${BLUE}=======================================${NC}"
echo -e "${BLUE}    ROOT-EDITOR INSTALLER              ${NC}"
echo -e "${BLUE}=======================================${NC}"
echo


if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}[ERROR]${YELLOW} This script needs to be run as root ${NC}"
    exit 1
fi


if [ ! -f /etc/fedora-release ]; then
    echo -e "${RED}[ERROR]${NC} This installer only supports Fedora."
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

echo -e "${BLUE}[INFO]${NC} Starting Fedora installation..."


install_dependencies() {
    echo -e "${BLUE}[INFO]${NC} Installing Fedora dependencies..."

    dnf install -y \
        gcc \
        gcc-c++ \
        make \
        cmake \
        ncurses-devel \
        pkg-config

    echo -e "${GREEN}[OK]${NC} Dependencies installed."
}


check_dependencies() {
    clear

    echo -e "${BLUE}[INFO]${NC} Checking dependencies..."

    local missing=()

    command -v gcc >/dev/null || missing+=("gcc")
    command -v cmake >/dev/null || missing+=("cmake")
    command -v make >/dev/null || missing+=("make")

    if [ ${#missing[@]} -ne 0 ]; then
        echo -e "${YELLOW}[WARNING]${NC} Missing dependencies: ${missing[*]}"
        install_dependencies
    else
        echo -e "${GREEN}[OK]${NC} Build tools found."
    fi

    echo -e "${BLUE}[INFO]${NC} Ensuring ncurses development files..."
    dnf install -y ncurses-devel
}


check_dependencies


clear

echo -e "${BLUE}[INFO]${NC} Building root-editor..."

export PATH=/usr/bin:/bin:/usr/local/bin:$PATH


echo -e "${BLUE}[INFO]${NC} Cleaning build directory..."

rm -rf build

cmake -S . -B build

cmake --build build


if [ ! -f "build/editor" ]; then
    echo -e "${RED}[ERROR]${NC} Build failed. Missing build/editor"
    exit 1
fi


echo -e "${GREEN}[OK]${NC} Build successful."


clear

echo -e "${BLUE}[INFO]${NC} Building plugins..."

cd plugins

rm -rf build

cmake -S . -B build

cmake --build build

cd ..


clear

echo -e "${BLUE}[INFO]${NC} Installing binary..."

cp build/editor /usr/local/bin/root-editor
chmod +x /usr/local/bin/root-editor

ln -sf /usr/local/bin/root-editor /usr/local/bin/re


echo -e "${BLUE}[INFO]${NC} Installing plugins..."

mkdir -p /usr/local/lib/root-editor/plugins

rm -f /usr/local/lib/root-editor/plugins/*.so

cp plugins/build/*.so /usr/local/lib/root-editor/plugins/ 2>/dev/null || true


USER_PLUGINS_DIR="$HOME/.config/root-editor/plugins"

mkdir -p "$USER_PLUGINS_DIR"

rm -f "$USER_PLUGINS_DIR"/*.so 2>/dev/null || true

cp plugins/build/*.so "$USER_PLUGINS_DIR/" 2>/dev/null || true


mkdir -p /usr/local/share/root-editor

cp -r config/* /usr/local/share/root-editor/ 2>/dev/null || true


clear


echo -e "${BLUE}[INFO]${NC} Creating desktop entry..."

tee /usr/share/applications/root-editor.desktop > /dev/null << EOF
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
echo "Run 'root-editor' or 're' from anywhere."
echo "Plugins: /usr/local/lib/root-editor/plugins/"
echo


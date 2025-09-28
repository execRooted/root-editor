#!/bin/bash


set -e


RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' 

clear
echo "========================================="
echo "    ROOT-EDITOR UNINSTALLER             "
echo "========================================="
echo


if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}[ERROR]${YELLOW} This script needs to be run as root ${NC}"
    echo
    echo
    exit 1
fi


echo -e "${YELLOW}[WARNING]${NC} This will remove Root-Editor from your system."
echo
read -p "Continue with uninstallation? (Y/n): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]] && [[ ! -z $REPLY ]]; then
    echo "Uninstallation cancelled."
    exit 0
fi

echo
echo -e "${BLUE}[INFO]${NC} Starting uninstallation process..."


echo -e "${BLUE}[INFO]${NC} Removing binary..."
sudo rm -f /usr/local/bin/root-editor
sudo rm -f /usr/local/bin/re


echo -e "${BLUE}[INFO]${NC} Removing plugins..."
sudo rm -rf /usr/local/lib/root-editor


echo -e "${BLUE}[INFO]${NC} Removing user plugins..."
rm -rf "$HOME/.config/root-editor/plugins"

echo
echo -e "${BLUE}[INFO]${NC} Removing user configuration..."
rm -rf "$HOME/.config/root-editor"

echo -e "${BLUE}[INFO]${NC} Removing configuration..."
sudo rm -rf /usr/local/share/root-editor

echo -e "${BLUE}[INFO]${NC} Removing desktop entry..."
sudo rm -f /usr/share/applications/root-editor.desktop


echo
echo -e "${GREEN}[SUCCESS]${NC} Root-Editor has been uninstalled successfully!"
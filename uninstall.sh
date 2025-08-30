#!/bin/bash

set -e
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'
INSTALL_DIR="/usr/local/bin"
BINARY_NAME="root-editor"
COMMAND_NAME="re"
print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
check_root() {
    if [ "$EUID" -eq 0 ]; then
        print_success "Running with root privileges"
        return 0
    else
        print_warning "Root privileges required for uninstallation"
        echo
        echo "Please enter your password to continue with uninstallation:"
        echo

        
        if sudo -v 2>/dev/null; then
            print_success "Authentication successful - proceeding with uninstallation"
            return 0
        else
            print_error "Authentication failed or sudo not available"
            print_error "Please run this uninstaller as root: sudo ./uninstall.sh"
            exit 1
        fi
    fi
}
remove_binary() {
    print_status "Removing binary and command alias..."
    local removed_count=0
    if [ -f "$INSTALL_DIR/$BINARY_NAME" ]; then
        if rm -f "$INSTALL_DIR/$BINARY_NAME"; then
            print_success "Removed $BINARY_NAME"
            ((removed_count++))
        else
            print_warning "Failed to remove $BINARY_NAME"
        fi
    else
        print_warning "$BINARY_NAME not found"
    fi
    if [ -f "$INSTALL_DIR/$COMMAND_NAME" ]; then
        if rm -f "$INSTALL_DIR/$COMMAND_NAME"; then
            print_success "Removed $COMMAND_NAME command alias"
            ((removed_count++))
        else
            print_warning "Failed to remove $COMMAND_NAME command alias"
        fi
    else
        print_warning "$COMMAND_NAME command alias not found"
    fi
    if [ $removed_count -eq 0 ]; then
        print_warning "No editor files found to remove"
        return 1
    fi
    return 0
}
remove_temp_files() {
    print_status "Removing temporary files..."
    local removed_count=0
    if [ -f "/tmp/kilo_editor_clipboard.txt" ]; then
        if rm -f "/tmp/kilo_editor_clipboard.txt"; then
            print_success "Removed temporary clipboard file"
            ((removed_count++))
        fi
    fi
    if find . -name "*.autosave" -type f -delete 2>/dev/null; then
        print_success "Removed autosave files"
        ((removed_count++))
    fi
    if find . -name "*.emergency" -type f -delete 2>/dev/null; then
        print_success "Removed emergency save files"
        ((removed_count++))
    fi
    if [ $removed_count -eq 0 ]; then
        print_status "No temporary files found to remove"
    fi
}
prompt_remove_dependencies() {
    echo
    print_warning "Remove build dependencies? (gcc, cmake, make, ncurses)"
    echo "These packages may be used by other programs."
    echo
    read -p "Remove build dependencies? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Nn]$ ]] || [[ -z $REPLY ]]; then
        return 1
    else
        return 0
    fi
}
remove_dependencies() {
    if ! prompt_remove_dependencies; then
        print_status "Keeping build dependencies"
        return 0
    fi
    print_status "Detecting package manager..."
    local package_manager=""
    local remove_cmd=""
    if command -v apt-get >/dev/null 2>&1; then
        package_manager="apt-get"
        remove_cmd="apt-get remove -y"
    elif command -v yum >/dev/null 2>&1; then
        package_manager="yum"
        remove_cmd="yum remove -y"
    elif command -v dnf >/dev/null 2>&1; then
        package_manager="dnf"
        remove_cmd="dnf remove -y"
    elif command -v pacman >/dev/null 2>&1; then
        package_manager="pacman"
        remove_cmd="pacman -R --noconfirm"
    elif command -v zypper >/dev/null 2>&1; then
        package_manager="zypper"
        remove_cmd="zypper remove -y"
    else
        print_warning "Unknown package manager. Skipping dependency removal."
        return 1
    fi
    print_status "Detected package manager: $package_manager"
    print_status "Removing build dependencies..."
    local packages=("gcc" "cmake" "make" "ncurses-dev" "libncurses5-dev" "libncursesw5-dev" "ncurses-devel")
    if $remove_cmd "${packages[@]}" 2>/dev/null; then
        print_success "Removed build dependencies"
    else
        print_warning "Some dependencies could not be removed (may be required by other packages)"
    fi
}
verify_uninstallation() {
    print_status "Verifying uninstallation..."
    local errors=0
    if [ -f "$INSTALL_DIR/$BINARY_NAME" ]; then
        print_error "Binary still exists: $INSTALL_DIR/$BINARY_NAME"
        ((errors++))
    fi
    if [ -f "$INSTALL_DIR/$COMMAND_NAME" ]; then
        print_error "Command alias still exists: $INSTALL_DIR/$COMMAND_NAME"
        ((errors++))
    fi
    if [ $errors -eq 0 ]; then
        print_success "Uninstallation verified successfully"
        return 0
    else
        print_error "Uninstallation completed with $errors error(s)"
        return 1
    fi
}
show_summary() {
    clear
    echo
    echo "=========================================="
    echo "    ROOT-EDITOR UNINSTALLATION COMPLETE   "
    echo "=========================================="
    echo
    print_success "Removed:"
    echo "  • Main binary ($BINARY_NAME)"
    echo "  • Command alias ($COMMAND_NAME)"
    echo "  • Temporary files"
    echo "  • Autosave files"
    echo "  • Emergency save files"
}
main() {
    clear
    echo "========================================="
    echo "    ROOT-EDITOR UNINSTALLER              "
    echo "========================================="
    echo
    print_warning "This will remove Root-Edit from your system."
    echo
    read -p "Continue with uninstallation? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_status "Uninstallation cancelled"
        exit 0
    fi
    print_status "Starting uninstallation process..."
    check_root
    if remove_binary; then
        remove_temp_files
        remove_dependencies
        verify_uninstallation
        show_summary
        print_success "Root-Edit has been successfully removed!"
    else
        print_warning "No editor installation found to remove"
        exit 1
    fi
}

main "$@"
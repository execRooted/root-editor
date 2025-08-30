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
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
detect_package_manager() {
    if command -v apt-get >/dev/null 2>&1; then
        PACKAGE_MANAGER="apt-get"
        INSTALL_CMD="apt-get install -y"
        UPDATE_CMD="apt-get update"
        CHECK_CMD="dpkg -l"
        REMOVE_CMD="apt-get remove -y"
    elif command -v yum >/dev/null 2>&1; then
        PACKAGE_MANAGER="yum"
        INSTALL_CMD="yum install -y"
        UPDATE_CMD="yum check-update || true"
        CHECK_CMD="rpm -q"
        REMOVE_CMD="yum remove -y"
    elif command -v dnf >/dev/null 2>&1; then
        PACKAGE_MANAGER="dnf"
        INSTALL_CMD="dnf install -y"
        UPDATE_CMD="dnf check-update || true"
        CHECK_CMD="rpm -q"
        REMOVE_CMD="dnf remove -y"
    elif command -v pacman >/dev/null 2>&1; then
        PACKAGE_MANAGER="pacman"
        INSTALL_CMD="pacman -S --noconfirm"
        UPDATE_CMD="pacman -Sy"
        CHECK_CMD="pacman -Q"
        REMOVE_CMD="pacman -R --noconfirm"
    elif command -v zypper >/dev/null 2>&1; then
        PACKAGE_MANAGER="zypper"
        INSTALL_CMD="zypper install -y"
        UPDATE_CMD="zypper refresh"
        CHECK_CMD="rpm -q"
        REMOVE_CMD="zypper remove -y"
    else
        print_warning "Unknown package manager. Assuming apt-get."
        PACKAGE_MANAGER="apt-get"
        INSTALL_CMD="apt-get install -y"
        UPDATE_CMD="apt-get update"
        CHECK_CMD="dpkg -l"
        REMOVE_CMD="apt-get remove -y"
    fi
    print_status "Detected package manager: $PACKAGE_MANAGER"
}
check_root() {
    if [ "$EUID" -eq 0 ]; then
        print_success "Running with root privileges"
        return 0
    else
        print_warning "Root privileges required for installation"
        echo
        echo "Please enter your password to continue with installation:"
        echo

       
        if sudo -v 2>/dev/null; then
            print_success "Authentication successful - proceeding with installation"
            return 0
        else
            print_error "Authentication failed or sudo not available"
            print_error "Please run this installer as root: sudo ./install.sh"
            exit 1
        fi
    fi
}
install_dependencies() {
    print_status "Checking system dependencies..."
    local required_packages=("gcc" "cmake" "make" "ncurses-dev")
    local missing_packages=()
    case $PACKAGE_MANAGER in
        "pacman") required_packages=("gcc" "cmake" "make" "ncurses");;
        "apt-get") required_packages=("gcc" "cmake" "make" "libncurses5-dev" "libncursesw5-dev");;
        "yum"|"dnf") required_packages=("gcc" "cmake" "make" "ncurses-devel");;
        "zypper") required_packages=("gcc" "cmake" "make" "ncurses-devel");;
    esac
    for pkg in "${required_packages[@]}"; do
        if ! $CHECK_CMD "$pkg" >/dev/null 2>&1; then
            missing_packages+=("$pkg")
        fi
    done
    if [ ${#missing_packages[@]} -gt 0 ]; then
        print_status "Installing missing packages: ${missing_packages[*]}"
        if [ "$PACKAGE_MANAGER" != "pacman" ]; then $UPDATE_CMD; fi
        if $INSTALL_CMD "${missing_packages[@]}"; then
            print_success "Required packages installed"
        else
            print_error "Failed to install required packages"
            exit 1
        fi
    else
        print_success "All required packages already installed"
    fi
    local clipboard_found=false
    local clipboard_packages=("xclip" "wl-clipboard" "xsel")
    for pkg in "${clipboard_packages[@]}"; do
        if $CHECK_CMD "$pkg" >/dev/null 2>&1; then
            print_success "Clipboard support available ($pkg)"
            clipboard_found=true
            break
        fi
    done
    if [ "$clipboard_found" = false ]; then
        print_warning "No clipboard tools found - copy/paste will use fallback method"
    fi
}
build_editor() {
    print_status "Building text editor..."
    cd "$SOURCE_DIR"
    if [ -f Makefile ]; then make clean >/dev/null 2>&1; fi
    if make; then
        if [ -f "./editor" ]; then
            print_success "Editor built successfully"
        else
            print_error "Build completed but binary not found"
            exit 1
        fi
    else
        print_error "Failed to build editor"
        exit 1
    fi
}
install_editor() {
    print_status "Installing editor system-wide..."
    if [ ! -d "$INSTALL_DIR" ]; then mkdir -p "$INSTALL_DIR"; fi
    if ! cp "$SOURCE_DIR/editor" "$INSTALL_DIR/$BINARY_NAME"; then
        print_error "Failed to copy binary to $INSTALL_DIR"
        exit 1
    fi
    if ! chmod 755 "$INSTALL_DIR/$BINARY_NAME"; then
        print_error "Failed to set executable permissions"
        exit 1
    fi
    cat > "$INSTALL_DIR/$COMMAND_NAME" << EOF
#!/bin/bash
exec "$INSTALL_DIR/$BINARY_NAME" "\$@"
EOF
    if ! chmod 755 "$INSTALL_DIR/$COMMAND_NAME"; then
        print_error "Failed to set executable permissions for alias"
        exit 1
    fi

    if ! echo "$PATH" | grep -q "$INSTALL_DIR"; then
        print_warning "Adding $INSTALL_DIR to PATH..."
        echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> ~/.bashrc
        echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> ~/.zshrc 2>/dev/null || true
        print_success "Added $INSTALL_DIR to PATH in shell configuration"
        print_warning "Please restart your terminal or run 'source ~/.bashrc' to use the new PATH"
    fi

    print_success "Editor installed as '$BINARY_NAME'"
    print_success "Command alias '$COMMAND_NAME' created"
}
verify_installation() {
    print_status "Verifying installation..."
    local errors=0
    if [ ! -x "$INSTALL_DIR/$BINARY_NAME" ]; then
        print_error "Binary not found or not executable: $INSTALL_DIR/$BINARY_NAME"
        ((errors++))
    fi
    if [ ! -x "$INSTALL_DIR/$COMMAND_NAME" ]; then
        print_error "Command alias not found or not executable: $INSTALL_DIR/$COMMAND_NAME"
        ((errors++))
    fi
    if [ $errors -eq 0 ]; then
        print_success "Installation verified successfully"
        return 0
    else
        print_error "Installation verification failed with $errors error(s)"
        return 1
    fi
}
show_summary() {
    clear
    echo
    echo "=========================================="
    echo "    ROOT-EDITOR INSTALLATION COMPLETE     "
    echo "=========================================="
    echo
    print_success "Installation Summary:"
    echo "  Binary: $INSTALL_DIR/$BINARY_NAME"
    echo "  Alias:  $INSTALL_DIR/$COMMAND_NAME"
    echo
    echo "Usage:"
    echo "  $BINARY_NAME [filename]    # Launch editor"
    echo "  $COMMAND_NAME [filename]   # Launch with alias"
    echo
    print_success "Installation completed successfully!"
    echo
    echo "Type '$COMMAND_NAME' or '$BINARY_NAME' to start using the editor."
}
main() {
    clear
    echo "========================================="
    echo "    ROOT-EDITOR INSTALLER                "
    echo "========================================="
    echo
    print_status "Starting installation process..."
    detect_package_manager
    check_root
    install_dependencies
    build_editor
    install_editor
    if verify_installation; then
        show_summary
        print_success "Root-Edit has been successfully installed on your system!"
    else
        print_error "Installation completed with errors."
        exit 1
    fi
}
main "$@"
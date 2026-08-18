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


# ============================================================
# Root check
# ============================================================

if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}[ERROR]${YELLOW} This script needs to be run as root ${NC}"
    exit 1
fi


# ============================================================
# Detect Linux distribution
# ============================================================

DISTRO_ID="unknown"
DISTRO_LIKE=""
PRETTY_NAME="Unknown Linux"

if [ -f /etc/os-release ]; then
    # shellcheck disable=SC1091
    source /etc/os-release

    DISTRO_ID="${ID,,}"
    DISTRO_LIKE="${ID_LIKE,,}"
    PRETTY_NAME="${PRETTY_NAME:-$DISTRO_ID}"
fi


# ============================================================
# Detect package manager
#
# IMPORTANT:
# We identify Debian using /etc/os-release.
# We do NOT identify Debian simply because "apt" exists.
# ============================================================

PACKAGE_MANAGER=""

case "$DISTRO_ID" in

    fedora|rhel|rocky|almalinux|ol)
        PACKAGE_MANAGER="dnf"
        ;;

    centos)
        if command -v dnf >/dev/null 2>&1; then
            PACKAGE_MANAGER="dnf"
        elif command -v yum >/dev/null 2>&1; then
            PACKAGE_MANAGER="yum"
        fi
        ;;

    debian|ubuntu|linuxmint|pop|elementary|zorin)
        PACKAGE_MANAGER="apt-get"
        ;;

    arch|manjaro|endeavouros|garuda)
        PACKAGE_MANAGER="pacman"
        ;;

    opensuse*|sles)
        PACKAGE_MANAGER="zypper"
        ;;

    gentoo)
        PACKAGE_MANAGER="emerge"
        ;;

    alpine)
        PACKAGE_MANAGER="apk"
        ;;

    void)
        PACKAGE_MANAGER="xbps-install"
        ;;

esac


# ============================================================
# Fallback for unknown distributions
#
# If the distro wasn't detected, try to find a package manager.
# This DOES NOT use "apt" as distro detection.
# ============================================================

if [ -z "$PACKAGE_MANAGER" ]; then

    if command -v dnf >/dev/null 2>&1; then
        PACKAGE_MANAGER="dnf"

    elif command -v yum >/dev/null 2>&1; then
        PACKAGE_MANAGER="yum"

    elif command -v pacman >/dev/null 2>&1; then
        PACKAGE_MANAGER="pacman"

    elif command -v zypper >/dev/null 2>&1; then
        PACKAGE_MANAGER="zypper"

    elif command -v emerge >/dev/null 2>&1; then
        PACKAGE_MANAGER="emerge"

    elif command -v apk >/dev/null 2>&1; then
        PACKAGE_MANAGER="apk"

    elif command -v xbps-install >/dev/null 2>&1; then
        PACKAGE_MANAGER="xbps-install"

    elif command -v apt-get >/dev/null 2>&1; then
        PACKAGE_MANAGER="apt-get"
    fi

fi


# ============================================================
# Script directory
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "$SCRIPT_DIR"


# ============================================================
# Installation warning
# ============================================================

echo -e "${YELLOW}[WARNING]${NC} This will install root-editor system-wide."
echo

read -p "Continue with installation? (Y/n): " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Yy]$ ]] && [[ -n $REPLY ]]; then
    echo "Installation cancelled."
    exit 0
fi


clear

echo -e "${BLUE}[INFO]${NC} Starting ${PRETTY_NAME} installation..."


# ============================================================
# Dependency package mappings
# ============================================================

install_missing_dependencies() {

    local missing=()
    local packages=()

    # --------------------------------------------------------
    # Check actual programs first
    # --------------------------------------------------------

    command -v gcc >/dev/null 2>&1 || missing+=("gcc")
    command -v cmake >/dev/null 2>&1 || missing+=("cmake")
    command -v make >/dev/null 2>&1 || missing+=("make")
    command -v pkg-config >/dev/null 2>&1 || missing+=("pkg-config")

    # --------------------------------------------------------
    # Nothing missing
    # --------------------------------------------------------

    if [ ${#missing[@]} -eq 0 ]; then
        echo -e "${GREEN}[OK]${NC} Build tools found."
        return 0
    fi


    echo -e "${YELLOW}[WARNING]${NC} Missing dependencies: ${missing[*]}"


    # --------------------------------------------------------
    # No package manager
    # --------------------------------------------------------

    if [ -z "$PACKAGE_MANAGER" ]; then
        echo
        echo -e "${RED}ERROR: essential programs were not found: ${missing[*]}${NC}"
        exit 1
    fi


    echo -e "${BLUE}[INFO]${NC} Installing missing dependencies..."


    # --------------------------------------------------------
    # Build package list based ONLY on missing programs
    # --------------------------------------------------------

    case "$PACKAGE_MANAGER" in

        # ----------------------------------------------------
        # Fedora / RHEL / Rocky / AlmaLinux / CentOS
        # ----------------------------------------------------

        dnf|yum)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("gcc" "gcc-c++")
                        ;;
                    cmake)
                        packages+=("cmake")
                        ;;
                    make)
                        packages+=("make")
                        ;;
                    pkg-config)
                        packages+=("pkgconf-pkg-config")
                        ;;
                esac
            done

            "$PACKAGE_MANAGER" install -y "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # Debian / Ubuntu / Mint / Pop!_OS
        # ----------------------------------------------------

        apt-get)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("gcc" "g++")
                        ;;
                    cmake)
                        packages+=("cmake")
                        ;;
                    make)
                        packages+=("make")
                        ;;
                    pkg-config)
                        packages+=("pkg-config")
                        ;;
                esac
            done

            export DEBIAN_FRONTEND=noninteractive

            apt-get update
            apt-get install -y "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # Arch / Manjaro / EndeavourOS
        # ----------------------------------------------------

        pacman)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("gcc")
                        ;;
                    cmake)
                        packages+=("cmake")
                        ;;
                    make)
                        packages+=("make")
                        ;;
                    pkg-config)
                        packages+=("pkgconf")
                        ;;
                esac
            done

            pacman -Sy --needed --noconfirm "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # openSUSE / SLES
        # ----------------------------------------------------

        zypper)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("gcc" "gcc-c++")
                        ;;
                    cmake)
                        packages+=("cmake")
                        ;;
                    make)
                        packages+=("make")
                        ;;
                    pkg-config)
                        packages+=("pkg-config")
                        ;;
                esac
            done

            zypper --non-interactive refresh
            zypper --non-interactive install "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # Gentoo
        # ----------------------------------------------------

        emerge)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("sys-devel/gcc")
                        ;;
                    cmake)
                        packages+=("dev-util/cmake")
                        ;;
                    make)
                        packages+=("sys-devel/make")
                        ;;
                    pkg-config)
                        packages+=("dev-util/pkgconf")
                        ;;
                esac
            done

            emerge --ask=n "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # Alpine
        # ----------------------------------------------------

        apk)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("gcc" "g++")
                        ;;
                    cmake)
                        packages+=("cmake")
                        ;;
                    make)
                        packages+=("make")
                        ;;
                    pkg-config)
                        packages+=("pkgconf")
                        ;;
                esac
            done

            apk add "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # Void Linux
        # ----------------------------------------------------

        xbps-install)

            for program in "${missing[@]}"; do
                case "$program" in
                    gcc)
                        packages+=("gcc")
                        ;;
                    cmake)
                        packages+=("cmake")
                        ;;
                    make)
                        packages+=("make")
                        ;;
                    pkg-config)
                        packages+=("pkg-config")
                        ;;
                esac
            done

            xbps-install -Sy "${packages[@]}"
            ;;


        # ----------------------------------------------------
        # Unknown package manager
        # ----------------------------------------------------

        *)

            echo
            echo -e "${RED}ERROR: essential programs were not found: ${missing[*]}${NC}"
            exit 1
            ;;

    esac


    # --------------------------------------------------------
    # Check AGAIN after installing
    # --------------------------------------------------------

    missing=()

    command -v gcc >/dev/null 2>&1 || missing+=("gcc")
    command -v cmake >/dev/null 2>&1 || missing+=("cmake")
    command -v make >/dev/null 2>&1 || missing+=("make")
    command -v pkg-config >/dev/null 2>&1 || missing+=("pkg-config")


    if [ ${#missing[@]} -ne 0 ]; then
        echo
        echo -e "${RED}ERROR: essential programs were not found: ${missing[*]}${NC}"
        exit 1
    fi


    echo -e "${GREEN}[OK]${NC} Dependencies installed."
}


# ============================================================
# Dependency check
# ============================================================

check_dependencies() {

    clear

    echo -e "${BLUE}[INFO]${NC} Checking dependencies..."

    install_missing_dependencies

    echo -e "${BLUE}[INFO]${NC} Ensuring ncurses development files..."

    # --------------------------------------------------------
    # ncurses development package
    # --------------------------------------------------------

    case "$PACKAGE_MANAGER" in

        dnf|yum)
            if ! rpm -q ncurses-devel >/dev/null 2>&1; then
                "$PACKAGE_MANAGER" install -y ncurses-devel
            fi
            ;;

        apt-get)
            if ! dpkg -s libncurses-dev >/dev/null 2>&1; then
                apt-get update
                apt-get install -y libncurses-dev
            fi
            ;;

        pacman)
            if ! pacman -Q ncurses >/dev/null 2>&1; then
                pacman -Sy --needed --noconfirm ncurses
            fi
            ;;

        zypper)
            if ! rpm -q ncurses-devel >/dev/null 2>&1; then
                zypper --non-interactive install ncurses-devel
            fi
            ;;

        emerge)
            if ! equery list sys-libs/ncurses >/dev/null 2>&1; then
                emerge --ask=n sys-libs/ncurses
            fi
            ;;

        apk)
            if ! apk info -e ncurses-dev >/dev/null 2>&1; then
                apk add ncurses-dev
            fi
            ;;

        xbps-install)
            if ! xbps-query -Rs ncurses-devel 2>/dev/null | grep -q ncurses-devel; then
                xbps-install -Sy ncurses-devel
            fi
            ;;

        *)
            # Unknown distro/package manager.
            #
            # Do not abort here. The compiler/build process will
            # report an ncurses error if the development files
            # are actually unavailable.
            ;;
    esac
}


check_dependencies


# ============================================================
# Build root-editor
# ============================================================

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


# ============================================================
# Build plugins
# ============================================================

clear

echo -e "${BLUE}[INFO]${NC} Building plugins..."

if [ -d plugins ]; then

    cd plugins

    rm -rf build

    cmake -S . -B build

    cmake --build build

    cd ..

else
    echo -e "${YELLOW}[WARNING]${NC} plugins directory not found. Skipping plugins."
fi


# ============================================================
# Install binary
# ============================================================

clear

echo -e "${BLUE}[INFO]${NC} Installing binary..."

cp build/editor /usr/local/bin/root-editor
chmod +x /usr/local/bin/root-editor

ln -sf /usr/local/bin/root-editor /usr/local/bin/re


# ============================================================
# Install plugins
# ============================================================

echo -e "${BLUE}[INFO]${NC} Installing plugins..."

mkdir -p /usr/local/lib/root-editor/plugins

rm -f /usr/local/lib/root-editor/plugins/*.so

cp plugins/build/*.so /usr/local/lib/root-editor/plugins/ 2>/dev/null || true


# ============================================================
# User plugins
# ============================================================

USER_PLUGINS_DIR="$HOME/.config/root-editor/plugins"

mkdir -p "$USER_PLUGINS_DIR"

rm -f "$USER_PLUGINS_DIR"/*.so 2>/dev/null || true

cp plugins/build/*.so "$USER_PLUGINS_DIR/" 2>/dev/null || true


# ============================================================
# Configuration
# ============================================================

mkdir -p /usr/local/share/root-editor

cp -r config/* /usr/local/share/root-editor/ 2>/dev/null || true


# ============================================================
# Desktop entry
# ============================================================

clear

echo -e "${BLUE}[INFO]${NC} Creating desktop entry..."

cp logo.png /usr/share/pixmaps/root-editor.png

tee /usr/share/applications/root-editor.desktop > /dev/null << EOF
[Desktop Entry]
Name=Root Editor
Comment=A C terminal-based text editor
Exec=re %f
Icon=root-editor
Terminal=true
Type=Application
Categories=Utility;TextEditor;
MimeType=text/plain;
EOF


# ============================================================
# Done
# ============================================================

clear

echo -e "${GREEN}[SUCCESS]${NC} root-editor has been installed successfully!"
echo
echo "Run 'root-editor' or 're' from anywhere."
echo "Plugins: /usr/local/lib/root-editor/plugins/"
echo

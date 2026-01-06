#! /usr/bin/env bash

set -euo pipefail

c0=$(tput sgr0)
myself=$(basename "$0")

# Fonctions de messagerie
msg() {
	[ -n "${2-}" ] && tput setaf "$2"
	printf "%s%s\n" "$1" "$c0"
}

error() {
	msg "Erreur: $1" 9
	[ -z "${2-}" ] || exit "$2"
}

msg "Installing dependencies for Vulkan Tutorial..." 14

# Function to detect the package manager
detect_package_manager() {
    if command -v apt-get &> /dev/null; then
        msg "apt" 11
    elif command -v dnf &> /dev/null; then
        msg "dnf" 11
    elif command -v pacman &> /dev/null; then
        msg "pacman" 11
    else
        msg "unknown" 9
    fi
}

# Install dependencies based on the package manager
PACKAGE_MANAGER=$(detect_package_manager)

case $PACKAGE_MANAGER in
    apt)
        msg "Detected Ubuntu/Debian-based system" 11
        msg "Installing build essentials..." 14
        sudo apt-get update
        sudo apt-get install -y build-essential cmake ninja-build

        msg "Installing GLFW..." 14
        sudo apt-get install -y libglfw3-dev

        msg "Installing GLM..." 14
        sudo apt-get install -y libglm-dev

        msg "Installing tinyobjloader..." 14
        sudo apt-get install -y libtinyobjloader-dev || msg "tinyobjloader not found in apt, will need to be installed manually or via CMake FetchContent" 9
        msg "Installing stb..." 14
        sudo apt-get install -y libstb-dev || msg "stb not found in apt, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing tinygltf..." 14
        sudo apt-get install -y libtinygltf-dev || msg "tinygltf not found in apt, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing nlohmann-json..." 14
        sudo apt-get install -y nlohmann-json3-dev || msg "nlohmann-json not found in apt, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing X Window System dependencies..." 14
        sudo apt-get install -y libxxf86vm-dev libxi-dev

        msg "Installing clang compiler..." 14
        sudo apt-get install -y clang
        ;;
    dnf)
        msg "Detected Fedora/RHEL-based system" 11
        msg "Installing build essentials..." 14
        sudo dnf install -y gcc-c++ cmake ninja-build

        msg "Installing GLFW..." 14
        sudo dnf install -y glfw-devel

        msg "Installing GLM..." 14
        sudo dnf install -y glm-devel

        msg "Installing tinyobjloader..." 14
        sudo dnf install -y tinyobjloader-devel || msg "tinyobjloader not found in dnf, will need to be installed manually or via CMake FetchContent" 9
        msg "Installing tinygltf..." 14
        sudo dnf install -y tinygltf-devel || msg "tinygltf not found in dnf, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing nlohmann-json..." 14
        sudo dnf install -y nlohmann-json-devel || msg "nlohmann-json not found in dnf, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing X Window System dependencies..." 14
        sudo dnf install -y libXxf86vm-devel libXi-devel

        msg "Installing clang compiler..." 14
        sudo dnf install -y clang
        ;;
    pacman)
        msg "Detected Arch-based system" 11
        msg "Installing build essentials..." 14
        sudo pacman -S --needed base-devel cmake ninja

        msg "Installing GLFW..." 14
        sudo pacman -S --needed glfw-x11 || sudo pacman -S --needed glfw-wayland

        msg "Installing GLM..." 14
        sudo pacman -S --needed glm

        msg "Installing tinyobjloader..." 14
        sudo pacman -S --needed tinyobjloader || msg "tinyobjloader not found in pacman, will need to be installed manually or via CMake FetchContent" 9
        msg "Installing tinygltf..." 14
        sudo pacman -S --needed tinygltf || msg "tinygltf not found in pacman, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing nlohmann-json..." 14
        sudo pacman -S --needed nlohmann-json || msg "nlohmann-json not found in pacman, will need to be installed manually or via CMake FetchContent" 9

        msg "Installing clang compiler..." 14
        sudo pacman -S --needed clang
        ;;
    *)
        echo "Unsupported package manager. Please install the following packages manually:"
        echo "- build-essential or equivalent (gcc, g++, make)"
        echo "- cmake"
        echo "- ninja-build"
        echo "- libglfw3-dev or equivalent"
        echo "- libglm-dev or equivalent"
        echo "- libtinyobjloader-dev or equivalent"
        echo "- libstb-dev or equivalent"
        echo "- libtinygltf-dev or equivalent"
        echo "- nlohmann-json3-dev or equivalent"
        echo "- libxxf86vm-dev and libxi-dev or equivalent"
        echo "- clang compiler"
        exit 1
        ;;
esac

# Vulkan SDK installation instructions
echo ""
echo "Now you need to install the Vulkan SDK:"
echo "1. Download the tarball from https://vulkan.lunarg.com/"
echo "2. Extract it to a convenient location, for example:"
echo "   mkdir -p ~/vulkansdk"
echo "   tar -xf vulkansdk-linux-x86_64-<version>.tar.xz -C ~/vulkansdk"
echo "   cd ~/vulkansdk"
echo "   ln -s <version> default"
echo ""
echo "3. Add the following to your ~/.bashrc or ~/.zshrc:"
echo "   source ~/vulkansdk/default/setup-env.sh"
echo ""
echo "4. Restart your terminal or run: source ~/.bashrc"
echo ""
echo "5. Verify installation by running: vkcube"
echo ""

echo "All dependencies have been installed successfully!"
echo "You can now use CMake to build your Vulkan project:"
echo "cmake -B build -S . -G Ninja"
echo "cmake --build build"

exit 0
#!/bin/bash
# metadata-name: micro-multica
# metadata-description: A headless C++ state-machine engine for managing AI agent issue execution lifecycles.
# metadata-author: Group of 4
# metadata-version: 1.0.0
# metadata-tags: cplusplus, ai-agents, automation, task-manager

# Determine OS and Architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

GITHUB_REPO="khanhthanhdev/nano_multica"
BINARY_NAME="multica"
INSTALL_DIR="/usr/local/bin"

echo "=== Installing MicroMultica CLI Engine ==="

# Check if running as root or have sudo permissions for standard installs
SUDO=""
if [ "$EUID" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    else
        # Fallback to local user home bin directory if no root/sudo
        INSTALL_DIR="$HOME/.local/bin"
        mkdir -p "$INSTALL_DIR"
        # Add to PATH temporarily if not present, and warn
        if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
            echo "Warning: $INSTALL_DIR is not in your PATH. You may need to add it to your shell config."
        fi
    fi
fi

# In a fully deployed release pipeline, precompiled binaries would be pulled from:
# URL="https://github.com/${GITHUB_REPO}/releases/latest/download/${BINARY_NAME}-${OS}-${ARCH}"
#
# For convenience, we will compile it locally if build tools are available,
# or simulate downloading a precompiled release.
if command -v g++ >/dev/null 2>&1 && [ -f "Makefile" ]; then
    echo "Build tools detected. Compiling binary from local source for optimal performance..."
    make clean && make
    if [ -f "./multica" ]; then
        echo "Installing binary to $INSTALL_DIR..."
        $SUDO cp ./multica "$INSTALL_DIR/$BINARY_NAME"
        $SUDO chmod +x "$INSTALL_DIR/$BINARY_NAME"
        echo "MicroMultica installed successfully to $INSTALL_DIR/$BINARY_NAME"
        exit 0
    fi
fi

echo "Compilation not possible, searching for pre-compiled assets..."
# Fallback placeholder to download the binary
TEMP_FILE=$(mktemp)
# Simulate/Run download from GitHub Releases
DOWNLOAD_URL="https://github.com/${GITHUB_REPO}/releases/latest/download/${BINARY_NAME}-linux-x86_64"
echo "Fetching from: $DOWNLOAD_URL"
curl -fsSL "$DOWNLOAD_URL" -o "$TEMP_FILE" || {
    echo "Error: Pre-compiled binary not found on GitHub Releases."
    echo "Please make sure a release is published on GitHub or build from source."
    rm -f "$TEMP_FILE"
    exit 1
}

$SUDO cp "$TEMP_FILE" "$INSTALL_DIR/$BINARY_NAME"
$SUDO chmod +x "$INSTALL_DIR/$BINARY_NAME"
rm -f "$TEMP_FILE"

echo "MicroMultica CLI Engine installed successfully!"

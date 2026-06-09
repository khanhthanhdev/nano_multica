#!/bin/bash
# Agent-driven install script for nano-multica
# Called by AI agents when multica binary is not found in PATH.

set -euo pipefail

REPO_URL="https://github.com/khanhthanhdev/nano_multica.git"
BINARY_NAME="multica"
TEMP_DIR=$(mktemp -d)

cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

echo "=== Installing NanoMultica CLI ==="

# 1. Clone the repo
echo "[1/3] Cloning repository..."
git clone --depth 1 "$REPO_URL" "$TEMP_DIR"

# 2. Build from source
echo "[2/3] Building..."
cd "$TEMP_DIR"
if ! command -v g++ &>/dev/null; then
    echo "Error: g++ not found. Install build-essential or Xcode Command Line Tools."
    exit 1
fi
make clean && make

# 3. Install
echo "[3/3] Installing..."
INSTALL_DIR="/usr/local/bin"
if [ ! -w "$INSTALL_DIR" ]; then
    INSTALL_DIR="$HOME/.local/bin"
    mkdir -p "$INSTALL_DIR"
    echo "No sudo access — installing to $INSTALL_DIR"
    echo "Make sure $INSTALL_DIR is in your PATH."
fi
cp "$TEMP_DIR/$BINARY_NAME" "$INSTALL_DIR/$BINARY_NAME"
chmod +x "$INSTALL_DIR/$BINARY_NAME"

echo "NanoMultica installed successfully at $INSTALL_DIR/$BINARY_NAME"

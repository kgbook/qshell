#!/bin/bash
# macOS build and package script for qshell

set -e  # Exit on error

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)
cd ${SCRIPT_DIR}/..

echo "======================================"
echo "qshell macOS Build and Package Script"
echo "======================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Check dependencies
echo -e "${YELLOW}Step 1: Checking dependencies...${NC}"

if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is not installed"
    echo "Install with: brew install cmake"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "Error: make is not installed"
    echo "Install Xcode Command Line Tools with: xcode-select --install"
    exit 1
fi

# Check for Qt6
if [ -z "$Qt6_DIR" ]; then
    echo "Warning: Qt6_DIR environment variable not set"
    echo "Trying to find Qt6 automatically..."
    # Try common locations
    if [ -d "/usr/local/opt/qt6" ]; then
        export Qt6_DIR="/usr/local/opt/qt6"
        echo "Found Qt6 at: $Qt6_DIR"
    elif [ -d "/opt/homebrew/opt/qt" ]; then
        export Qt6_DIR="/opt/homebrew/opt/qt"
        echo "Found Qt6 at: $Qt6_DIR"
    else
        echo "Error: Qt6 not found. Please set Qt6_DIR environment variable"
        echo "Example: export Qt6_DIR=/path/to/Qt6"
        exit 1
    fi
fi

echo -e "${GREEN}✓ Dependencies check passed${NC}"
echo ""

# Step 2: Create ICNS icon if needed
echo -e "${YELLOW}Step 2: Preparing macOS icon...${NC}"

ICNS_FILE="src/resources/images/qshell.icns"

if [ ! -f "$ICNS_FILE" ]; then
    echo "ICNS icon not found. Converting PNG to ICNS..."
    if command -v sips &> /dev/null; then
        ./scripts/macos-create-icon.sh
        echo -e "${GREEN}✓ Icon created successfully${NC}"
    else
        echo "Warning: Cannot create ICNS icon (requires macOS)"
        echo "Please create the ICNS file manually or skip this step"
    fi
else
    echo -e "${GREEN}✓ ICNS icon already exists${NC}"
fi

echo ""

# Step 3: Build
echo -e "${YELLOW}Step 3: Building qshell...${NC}"

BUILD_DIR="build-macos"

# Clean build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

echo "Building..."
cmake --build . --config Release

echo -e "${GREEN}✓ Build completed successfully${NC}"
echo ""

# Step 4: Package (optional)
echo -e "${YELLOW}Step 4: Creating DMG package...${NC}"

read -p "Do you want to create a DMG package? (y/n): " CREATE_DMG

if [ "$CREATE_DMG" = "y" ] || [ "$CREATE_DMG" = "Y" ]; then
    echo "Creating DMG package..."
    cpack -G DragNDrop
    
    # Find the generated DMG file
    DMG_FILE=$(find . -name "*.dmg" -type f | head -n 1)
    
    if [ -n "$DMG_FILE" ]; then
        echo ""
        echo -e "${GREEN}✓ DMG package created: $DMG_FILE${NC}"
        echo ""
        echo "Package location: $(pwd)/$DMG_FILE"
    else
        echo "Error: DMG file not found after packaging"
        exit 1
    fi
else
    echo "Skipping DMG package creation"
fi

echo ""
echo -e "${GREEN}======================================"
echo "Build completed successfully!"
echo "======================================${NC}"
echo ""
echo "Binary location: $(pwd)/qshell"
echo ""
echo "To run: ./qshell"
echo ""

#!/bin/bash
# Convert PNG icon to ICNS format for macOS

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)
cd ${SCRIPT_DIR}

# Source PNG file
PNG_FILE="${SCRIPT_DIR}/../src/resources/images/qshell.png"

# Output directory for iconset
ICONSET_DIR="${SCRIPT_DIR}/../src/resources/images/qshell.iconset"

# Output ICNS file
ICNS_FILE="${SCRIPT_DIR}/../src/resources/images/qshell.icns"

# Check if source PNG exists
if [ ! -f "${PNG_FILE}" ]; then
    echo "Error: ${PNG_FILE} not found!"
    exit 1
fi

# Create iconset directory
mkdir -p "${ICONSET_DIR}"

# Convert PNG to different sizes required for ICNS
# macOS requires these specific sizes:
# icon_16x16.png
# icon_16x16@2x.png (32x32)
# icon_32x32.png
# icon_32x32@2x.png (64x64)
# icon_128x128.png
# icon_128x128@2x.png (256x256)
# icon_256x256.png
# icon_256x256@2x.png (512x512)
# icon_512x512.png
# icon_512x512@2x.png (1024x1024)

echo "Converting ${PNG_FILE} to ICNS format..."

# Check if sips is available (macOS built-in tool)
if command -v sips &> /dev/null; then
    echo "Using sips (macOS built-in tool)..."
    
    # Create different sizes
    sips -z 16 16 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_16x16.png"
    sips -z 32 32 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_16x16@2x.png"
    sips -z 32 32 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_32x32.png"
    sips -z 64 64 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_32x32@2x.png"
    sips -z 128 128 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_128x128.png"
    sips -z 256 256 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_128x128@2x.png"
    sips -z 256 256 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_256x256.png"
    sips -z 512 512 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_256x256@2x.png"
    sips -z 512 512 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_512x512.png"
    sips -z 1024 1024 "${PNG_FILE}" --out "${ICONSET_DIR}/icon_512x512@2x.png"
    
    # Convert iconset to icns
    iconutil -c icns "${ICONSET_DIR}" -o "${ICNS_FILE}"
    
    echo "ICNS file created: ${ICNS_FILE}"
    
    # Clean up iconset directory
    rm -rf "${ICONSET_DIR}"
    
else
    echo "Warning: sips command not found. This script requires macOS."
    echo "Please run this script on macOS, or manually convert the PNG to ICNS."
    echo ""
    echo "Alternative: Use an online converter or create the ICNS file manually."
    echo "Required sizes: 16x16, 32x32, 64x64, 128x128, 256x256, 512x512, 1024x1024"
    exit 1
fi

echo "Done!"

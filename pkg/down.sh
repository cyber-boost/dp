#!/bin/bash

# KDE Dynamic Wallpaper Downloader
# Downloads the latest package from cyberboost.com

set -e

DOWNLOAD_URL="http://cyberboost.com/latest/dp.tar.gz"
PACKAGE_FILE="dpaper.tar.gz"

echo "Downloading KDE Dynamic Wallpaper package..."
echo "URL: $DOWNLOAD_URL"

# Check if wget or curl is available
if command -v wget >/dev/null 2>&1; then
    echo "Using wget to download..."
    wget -O "$PACKAGE_FILE" "$DOWNLOAD_URL"
elif command -v curl >/dev/null 2>&1; then
    echo "Using curl to download..."
    curl -L -o "$PACKAGE_FILE" "$DOWNLOAD_URL"
else
    echo "ERROR: Neither wget nor curl is available!"
    echo "Please install wget or curl and try again."
    exit 1
fi

# Verify download
if [ ! -f "$PACKAGE_FILE" ]; then
    echo "ERROR: Download failed - file not found!"
    exit 1
fi

FILE_SIZE=$(stat -c%s "$PACKAGE_FILE" 2>/dev/null || stat -f%z "$PACKAGE_FILE" 2>/dev/null || echo "0")
if [ "$FILE_SIZE" -eq 0 ]; then
    echo "ERROR: Downloaded file is empty!"
    rm -f "$PACKAGE_FILE"
    exit 1
fi

echo "Download complete: $PACKAGE_FILE (${FILE_SIZE} bytes)"
echo ""
echo "To install:"
echo "  tar -xzf $PACKAGE_FILE"
echo "  cd kde-dp-*/"
echo "  ./install.sh"
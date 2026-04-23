#!/bin/bash
set -e

echo ">>> Installing MinGW-w64 cross compiler..."
sudo apt-get update -qq
sudo apt-get install -y -qq mingw-w64

echo ">>> Building number-generator.exe..."
make clean
make

echo ""
echo "✅ Build complete: number-generator.exe"
echo "   Download it from the Codespaces file explorer and run on Windows."
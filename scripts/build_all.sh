#!/bin/bash
set -e

# Go to project root
cd "$(dirname "$0")/.."

echo "Building Compiler..."
./compiler/scripts/build_compiler.sh

echo ""
echo "Setup Server..."
if [ ! -d "server/.venv" ]; then
    python3 -m venv server/.venv
    ./server/.venv/bin/pip install -r server/requirements.txt
else
    echo "Server venv exists."
fi

echo ""
echo "Setup Frontend..."
echo "Note: You need npm installed to build frontend."
if command -v npm &> /dev/null; then
    cd frontend
    npm install
    # npm run build # Optional, usually we just run dev
else
    echo "Warning: npm not found. Skipping frontend install."
fi

echo ""
echo "Build verification complete."

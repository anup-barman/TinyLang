#!/bin/bash

# Function to handle cleanup on exit
cleanup() {
    echo ""
    echo "Stopping servers..."
    kill $(jobs -p) 2>/dev/null
}
trap cleanup EXIT

# Go to project root
cd "$(dirname "$0")/.."

echo "========================================="
echo "Starting TinyLang Development Servers"
echo "========================================="
echo ""

echo "Starting Backend Server..."
./server/.venv/bin/python3 -m uvicorn server.app.main:app --port 8000 &
SERVER_PID=$!
sleep 2

echo "Starting Frontend Dev Server..."
if command -v npm &> /dev/null; then
    cd frontend
    npm run dev &
    FRONT_PID=$!
    cd ..
else
    echo "npm not found. Cannot start frontend."
    echo "Backend is running on localhost:8000"
fi

sleep 3
echo ""
echo "========================================="
echo "TinyLang is running!"
echo "========================================="
echo ""
echo "Frontend: http://localhost:5173"
echo "Backend:  http://localhost:8000"
echo ""
echo "⚠️  IMPORTANT - BROWSER CACHE ISSUE ⚠️"
echo "----------------------------------------"
echo "If you're seeing wrong output (like '45' and '0'),"
echo "it's because your browser has cached old files."
echo ""
echo "SOLUTIONS (pick one):"
echo ""
echo "1. 🎭 EASIEST: Open in Incognito/Private mode"
echo "   - Chrome/Brave: Ctrl+Shift+N"
echo "   - Firefox: Ctrl+Shift+P"
echo ""
echo "2. 🔄 Hard Reload (Chrome/Brave/Edge):"
echo "   - Open DevTools (F12)"
echo "   - RIGHT-CLICK the reload button"
echo "   - Select 'Empty Cache and Hard Reload'"
echo ""
echo "3. 🗑️  Clear Cache (Firefox):"
echo "   - Press Ctrl+Shift+Delete"
echo "   - Select 'Cached Web Content'"
echo "   - Click 'Clear Now', then Ctrl+F5"
echo ""
echo "========================================="
echo ""
echo "Press Ctrl+C to stop all servers."
echo ""

wait

#!/bin/bash
set -e

# Go to project root
cd "$(dirname "$0")/.."

echo "========================================="
echo "TinyLang: Build & Run"
echo "========================================="

echo "Step 1: Building..."
./scripts/build_all.sh

echo ""
echo "Step 2: Starting Servers..."
./scripts/run_local.sh

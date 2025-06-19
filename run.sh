#!/bin/sh
set -e

make

echo "[run.sh]   Starting Engine..."
./interlope

#!/bin/sh
set -e

BIN=bin
SRC=src
VENDOR=vendor

echo "[build.sh] Compiling Engine..."

gcc \
    -I ${VENDOR}/include \
    -L ${VENDOR} \
    -lm -lGL -lGLEW -lglfw \
    -lcglm \
    -o interlope\
    \
    ${SRC}/file.c \
    ${SRC}/gfx.c \
    ${SRC}/log.c \
    ${SRC}/main.c \
    ${SRC}/platform.c \
    ${SRC}/time.c \

echo "[build.sh] Compiling Done"
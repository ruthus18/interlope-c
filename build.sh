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
    ${SRC}/platform/file.c \
    ${SRC}/platform/input.c \
    ${SRC}/platform/time.c \
    ${SRC}/camera.c \
    ${SRC}/cgm.c \
    ${SRC}/gfx.c \
    ${SRC}/log.c \
    ${SRC}/main.c \

echo "[build.sh] Compiling Done"

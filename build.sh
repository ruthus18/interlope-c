#!/bin/sh
set -e

BIN=bin
SRC=src
VENDOR=vendor

echo "[build.sh] Compiling Engine..."

time gcc \
    -I ${VENDOR}/include \
    -L ${VENDOR} \
    -lm -lGL -lGLEW -lglfw \
    -lcglm \
    -o interlope\
    \
    ${SRC}/gfx.c \
    \
    ${SRC}/platform/file.c \
    ${SRC}/platform/input.c \
    ${SRC}/platform/time.c \
    \
    ${SRC}/assets.c \
    ${SRC}/camera.c \
    ${SRC}/cgm.c \
    ${SRC}/log.c \
    ${SRC}/main.c \

echo "[build.sh] Compiling Done"

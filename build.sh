#!/bin/sh
set -e

BIN=bin
SRC=src
VENDOR=vendor

echo "[build.sh] Compiling Engine..."

time gcc \
    -std="c23" -g \
    -I ${VENDOR}/include \
    -L ${VENDOR} \
    -Wl,-rpath=${VENDOR} \
    -lm -lGL -lGLEW -lglfw -lcglm -lode \
    -o ${BIN}/interlope\
    \
    ${SRC}/assets/model.c \
    ${SRC}/assets/texture.c \
    \
    ${SRC}/core/cgm.c \
    ${SRC}/core/log.c \
    \
    ${SRC}/data_read/objdb_reader.c \
    ${SRC}/data_read/scene_reader.c \
    \
    ${SRC}/editor/ui.c \
    \
    ${SRC}/platform/file.c \
    ${SRC}/platform/input.c \
    ${SRC}/platform/time.c \
    ${SRC}/platform/window.c \
    \
    ${SRC}/render/camera.c \
    ${SRC}/render/gfx.c \
    ${SRC}/render/gfx_shader.c \
    \
    ${SRC}/world/objdb.c \
    ${SRC}/world/object.c \
    ${SRC}/world/player.c \
    ${SRC}/world/scene.c \
    \
    ${SRC}/physics.c \
    ${SRC}/main.c \
    \
    ${VENDOR}/src/toml.c \

echo "[build.sh] Compiling Done"

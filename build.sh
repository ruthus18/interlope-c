#!/bin/sh
set -e

BIN=bin
SRC=src
VENDOR=vendor

echo "[build.sh] Compiling Engine..."

time gcc \
    -std="c23" \
    -I ${VENDOR}/include \
    -L ${VENDOR} \
    -Wl,-rpath=${VENDOR} \
    -lm -lGL -lGLEW -lglfw -lcglm -lode \
    -o ${BIN}/interlope\
    \
    ${SRC}/platform/file.c \
    ${SRC}/platform/input.c \
    ${SRC}/platform/time.c \
    ${SRC}/platform/window.c \
    \
    ${SRC}/gfx.c \
    ${SRC}/gfx_shader.c \
    \
    ${SRC}/camera.c \
    ${SRC}/cgm.c \
    ${SRC}/editor.c \
    ${SRC}/log.c \
    ${SRC}/main.c \
    ${SRC}/model.c \
    ${SRC}/object.c \
    ${SRC}/objects_db.c \
    ${SRC}/physics.c \
    ${SRC}/scene.c \
    ${SRC}/texture.c \
    \
    ${VENDOR}/src/toml.c \

echo "[build.sh] Compiling Done"

# Makefile for interlope-c2 with incremental builds
# Build files placed in .build/ directory, executable in project root

CC = gcc
CFLAGS = -std=c23 -g
INCLUDES = -I vendor/include
LDFLAGS = -L vendor -Wl,-rpath=vendor
LIBS = -lm -lGL -lGLEW -lglfw -lcglm -lode -lcjson -lcjson_utils

TARGET = interlope
BUILD_DIR = .build
SRC_DIR = src
VENDOR_DIR = vendor

# Source files
SOURCES = \
	$(SRC_DIR)/assets/mesh_gltf.c \
	$(SRC_DIR)/assets/model.c \
	$(SRC_DIR)/assets/texture.c \
	$(SRC_DIR)/core/cgm.c \
	$(SRC_DIR)/core/log.c \
	$(SRC_DIR)/database/db.c \
	$(SRC_DIR)/database/loader.c \
	$(SRC_DIR)/editor/geometry.c \
	$(SRC_DIR)/gameplay/player.c \
	$(SRC_DIR)/platform/file.c \
	$(SRC_DIR)/platform/input.c \
	$(SRC_DIR)/platform/time.c \
	$(SRC_DIR)/platform/window.c \
	$(SRC_DIR)/render/camera.c \
	$(SRC_DIR)/render/geometry.c \
	$(SRC_DIR)/render/gfx.c \
	$(SRC_DIR)/render/gfx_shader.c \
	$(SRC_DIR)/world/object.c \
	$(SRC_DIR)/world/object_ref.c \
	$(SRC_DIR)/world/scene.c \
	$(SRC_DIR)/world/world.c \
	$(SRC_DIR)/engine.c \
	$(SRC_DIR)/physics.c \
	$(SRC_DIR)/main.c \
	$(VENDOR_DIR)/src/toml.c

# Object files (replace .c with .o and place in build directory)
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Dependency files
DEPS = $(OBJECTS:.o=.d)

.PHONY: all clean

all: 
	@echo "[make] Compiling Engine..."
	@START=$$(date +%s.%N); \
	$(MAKE) $(TARGET) --no-print-directory; \
	END=$$(date +%s.%N); \
	TIME=$$(echo "$$END - $$START" | bc -l); \
	printf "Build time: %.2fs\n" $$TIME

# Link the executable
$(TARGET): $(OBJECTS)
	@echo "[make] Linking $(TARGET)..."
	@$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@
	@echo "[make] Build complete"

# Compile source files to object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Clean build files
clean:
	@echo "[make] Cleaning build files..."
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	@echo "[make] Clean complete"

# Print variables for debugging
debug:
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"
	@echo "Deps: $(DEPS)"
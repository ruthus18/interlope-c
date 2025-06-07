# Python Integration Technical Plan

## Overview
Transform the current C engine into a shared library with Python bindings using a multithreaded architecture. C handles the main loop, rendering, physics, and platform systems on the main thread, while Python runs game logic on separate threads using a hook-based system similar to Unity's event model.

## Library Architecture

### C Shared Library (libinterlope.so)

#### 1. Library Structure
The C engine will be built as a shared library with two main components:

**Core Engine Library (`libinterlope_core.so`)**
- Rendering pipeline
- Physics engine  
- Platform abstraction
- Asset loading
- Memory management

**Python C Extension (`interlope_engine.so`)**
- Python API bindings
- Reference counting
- Exception handling
- Type conversions

#### 2. Build System Modifications

**Static Library Build (`build_lib.sh`)**
```bash
#!/bin/sh
set -e

# Build core engine as static library first
ar rcs lib/libinterlope_core.a \
    src/assets/model.o \
    src/assets/texture.o \
    src/core/cgm.o \
    src/core/log.o \
    src/platform/*.o \
    src/render/*.o \
    src/world/*.o \
    src/physics.o

# Build shared library
gcc -shared -fPIC \
    -o lib/libinterlope.so \
    -Wl,--whole-archive lib/libinterlope_core.a -Wl,--no-whole-archive \
    -lm -lGL -lGLEW -lglfw -lcglm -lode
```

**Python Extension Build (`setup.py`)**
```python
from setuptools import setup, Extension
import os

# Get Python include directory
import sysconfig
python_include = sysconfig.get_path('include')

ext_modules = [
    Extension(
        "interlope_engine",
        sources=[
            "src/python/python_integration.c",
            "src/python/python_module.c",
            "src/python/player_types.c",  # PlayerState/PlayerCommand implementations
        ],
        include_dirs=[
            "src", 
            "vendor/include",
            python_include
        ],
        libraries=["interlope"],
        library_dirs=["lib"],
        extra_compile_args=["-std=c23", "-fPIC"],
        extra_link_args=["-pthread"],
        language='c',
    ),
]

setup(
    name="interlope",
    ext_modules=ext_modules,
    zip_safe=False,
    python_requires=">=3.8",
)
```

## Multithreaded Architecture

### Thread Model
The engine uses a producer-consumer model with thread-safe communication:

**Main Thread (C Engine)**
- Rendering pipeline (60+ FPS)
- Physics simulation (fixed timestep)
- Input processing
- Window management
- Audio processing

**Python Logic Thread**
- Game logic execution
- AI processing  
- Script callbacks
- Non-critical updates

**Communication**
- Lock-free message queues between threads
- Thread-safe state synchronization
- Python hooks triggered by C events

```
┌─────────────────┐    Messages    ┌─────────────────┐
│   Main Thread   │ ◄─────────────► │ Python Thread   │
│   (C Engine)    │                 │ (Game Logic)    │
│                 │    State Sync   │                 │
│ • Rendering     │ ◄─────────────► │ • Scripts       │
│ • Physics       │                 │ • Game Logic    │
│ • Input         │                 │ • AI            │
└─────────────────┘                 └─────────────────┘
```

## C API Design

### 1. Threading & Message System

**`src/api/threading.h`**
```c
#ifndef THREADING_API_H
#define THREADING_API_H

#include <pthread.h>
#include <stdbool.h>
#include "../core/types.h"

// Message types for cross-thread communication
typedef enum {
    MSG_PLAYER_MOVED,
    MSG_OBJECT_SPAWNED,
    MSG_OBJECT_DESTROYED,
    MSG_COLLISION_EVENT,
    MSG_INPUT_EVENT,
    MSG_GAME_STATE_CHANGED,
    MSG_CUSTOM_EVENT
} MessageType;

typedef struct {
    MessageType type;
    uint32_t sender_id;
    uint32_t target_id;
    float timestamp;
    size_t data_size;
    void* data;
} Message;

typedef struct MessageQueue MessageQueue;

// Message queue operations (lock-free)
MessageQueue* message_queue_create(size_t capacity);
void message_queue_destroy(MessageQueue* queue);
bool message_queue_push(MessageQueue* queue, const Message* msg);
bool message_queue_pop(MessageQueue* queue, Message* msg);
size_t message_queue_size(MessageQueue* queue);

// Python callback system
typedef void (*PythonCallback)(const Message* msg);

typedef struct {
    MessageType type;
    PythonCallback callback;
    bool enabled;
} CallbackRegistration;

// Hook registration
bool engine_register_python_callback(MessageType type, PythonCallback callback);
void engine_unregister_python_callback(MessageType type);
void engine_trigger_python_callbacks(MessageType type, const void* data, size_t size);

#endif
```

### 2. Engine Context System
Create a central context that manages all engine state with thread safety:

**`src/api/engine.h`**
```c
#ifndef ENGINE_API_H
#define ENGINE_API_H

#include <stdbool.h>
#include <pthread.h>
#include "../core/types.h"
#include "threading.h"

typedef struct EngineContext EngineContext;

// Engine lifecycle
EngineContext* engine_create(void);
bool engine_init(EngineContext* ctx);
void engine_run(EngineContext* ctx);  // Runs main loop on current thread
void engine_stop(EngineContext* ctx);
void engine_destroy(EngineContext* ctx);

// Thread management
bool engine_start_python_thread(EngineContext* ctx);
void engine_stop_python_thread(EngineContext* ctx);

// Engine state (thread-safe)
bool engine_should_close(EngineContext* ctx);
void engine_set_should_close(EngineContext* ctx, bool should_close);
float engine_get_delta_time(EngineContext* ctx);
float engine_get_fixed_delta_time(EngineContext* ctx);

// Frame synchronization
void engine_wait_for_frame_sync(EngineContext* ctx);
uint64_t engine_get_frame_number(EngineContext* ctx);

// Resource management (thread-safe)
typedef struct {
    uint32_t id;
    char* path;
} ResourceHandle;

ResourceHandle engine_load_scene(EngineContext* ctx, const char* path);
ResourceHandle engine_load_objdb(EngineContext* ctx, const char* path);
void engine_unload_resource(EngineContext* ctx, ResourceHandle handle);

// Message system integration
MessageQueue* engine_get_python_queue(EngineContext* ctx);
MessageQueue* engine_get_main_queue(EngineContext* ctx);

#endif
```

**`src/api/engine.c`**
```c
#include "engine.h"
#include "../platform/window.h"
#include "../platform/input.h"
#include "../platform/time.h"
#include "../render/gfx.h"
#include "../physics.h"
#include "../world/scene.h"
#include "../world/objdb.h"

struct EngineContext {
    bool initialized;
    bool should_close;
    bool python_thread_active;
    
    // Core systems
    Scene* current_scene;
    ObjectsDB* current_objdb;
    
    // Threading
    pthread_t python_thread;
    pthread_mutex_t state_mutex;
    pthread_cond_t frame_sync_cond;
    
    // Frame timing
    float delta_time;
    float fixed_delta_time;
    uint64_t frame_number;
    bool frame_ready;
    
    // Message queues
    MessageQueue* python_queue;  // Main -> Python
    MessageQueue* main_queue;    // Python -> Main
    
    // Resource tracking
    ResourceHandle* resources;
    size_t resource_count;
    size_t resource_capacity;
    pthread_mutex_t resource_mutex;
};

// Python thread entry point
static void* python_thread_main(void* arg);
static void process_python_messages(EngineContext* ctx);

EngineContext* engine_create(void) {
    EngineContext* ctx = calloc(1, sizeof(EngineContext));
    if (!ctx) return NULL;
    
    // Initialize mutexes
    pthread_mutex_init(&ctx->state_mutex, NULL);
    pthread_mutex_init(&ctx->resource_mutex, NULL);
    pthread_cond_init(&ctx->frame_sync_cond, NULL);
    
    // Create message queues
    ctx->python_queue = message_queue_create(1024);
    ctx->main_queue = message_queue_create(1024);
    
    // Initialize resources
    ctx->resource_capacity = 64;
    ctx->resources = calloc(ctx->resource_capacity, sizeof(ResourceHandle));
    
    ctx->fixed_delta_time = 1.0f / 60.0f;  // 60 Hz physics
    
    return ctx;
}

bool engine_init(EngineContext* ctx) {
    if (!ctx || ctx->initialized) return false;
    
    if (!window_init()) return false;
    if (!input_init()) return false;
    if (!gfx_init()) return false;
    if (!physics_init()) return false;
    
    ctx->initialized = true;
    return true;
}

void engine_run(EngineContext* ctx) {
    if (!ctx || !ctx->initialized) return;
    
    float accumulator = 0.0f;
    float physics_timestep = ctx->fixed_delta_time;
    
    while (!ctx->should_close) {
        // Update timing
        time_update();
        ctx->delta_time = time_get_delta();
        accumulator += ctx->delta_time;
        
        // Process input
        window_poll_events();
        input_update();
        
        // Process messages from Python thread
        process_python_messages(ctx);
        
        // Fixed timestep physics
        while (accumulator >= physics_timestep) {
            physics_update();
            accumulator -= physics_timestep;
            
            // Trigger physics callbacks
            engine_trigger_python_callbacks(MSG_PHYSICS_TICK, NULL, 0);
        }
        
        // Update scene
        if (ctx->current_scene) {
            scene_update(ctx->current_scene);
        }
        
        // Render
        if (ctx->current_scene) {
            scene_draw(ctx->current_scene);
        }
        
        window_swap_buffers();
        if (!WINDOW_VSYNC) time_limit_framerate();
        
        // Signal frame completion for Python thread sync
        pthread_mutex_lock(&ctx->state_mutex);
        ctx->frame_number++;
        ctx->frame_ready = true;
        pthread_cond_broadcast(&ctx->frame_sync_cond);
        pthread_mutex_unlock(&ctx->state_mutex);
        
        ctx->should_close = gfx_need_stop();
    }
}

bool engine_start_python_thread(EngineContext* ctx) {
    if (!ctx || ctx->python_thread_active) return false;
    
    ctx->python_thread_active = true;
    
    if (pthread_create(&ctx->python_thread, NULL, python_thread_main, ctx) != 0) {
        ctx->python_thread_active = false;
        return false;
    }
    
    return true;
}

void engine_stop_python_thread(EngineContext* ctx) {
    if (!ctx || !ctx->python_thread_active) return;
    
    ctx->python_thread_active = false;
    pthread_join(ctx->python_thread, NULL);
}

static void process_python_messages(EngineContext* ctx) {
    Message msg;
    
    // Process up to 10 messages per frame to avoid blocking
    for (int i = 0; i < 10 && message_queue_pop(ctx->main_queue, &msg); i++) {
        switch (msg.type) {
            case MSG_OBJECT_SPAWNED:
                // Handle object spawning from Python
                break;
            case MSG_PLAYER_MOVED:
                // Handle player movement commands
                break;
            default:
                break;
        }
        
        // Free message data if allocated
        if (msg.data) {
            free(msg.data);
        }
    }
}

static void* python_thread_main(void* arg) {
    EngineContext* ctx = (EngineContext*)arg;
    
    // Initialize Python interpreter on this thread
    // ... Python initialization code ...
    
    while (ctx->python_thread_active) {
        // Wait for frame sync if needed
        engine_wait_for_frame_sync(ctx);
        
        // Process messages from main thread
        Message msg;
        while (message_queue_pop(ctx->python_queue, &msg)) {
            // Trigger appropriate Python callbacks
            engine_trigger_python_callbacks(msg.type, msg.data, msg.data_size);
            
            if (msg.data) {
                free(msg.data);
            }
        }
        
        // Sleep for a bit to avoid busy waiting
        usleep(1000);  // 1ms
    }
    
    return NULL;
}

void engine_wait_for_frame_sync(EngineContext* ctx) {
    pthread_mutex_lock(&ctx->state_mutex);
    
    while (!ctx->frame_ready) {
        pthread_cond_wait(&ctx->frame_sync_cond, &ctx->state_mutex);
    }
    
    ctx->frame_ready = false;
    pthread_mutex_unlock(&ctx->state_mutex);
}
```

### 3. Hook-Based Event System

Unity-style hooks that allow Python to register callbacks for engine events:

**`src/api/hooks.h`**
```c
#ifndef HOOKS_API_H
#define HOOKS_API_H

#include "threading.h"

// Engine event types (similar to Unity's MonoBehaviour)
typedef enum {
    HOOK_START,          // Called once when game starts
    HOOK_UPDATE,         // Called every frame
    HOOK_FIXED_UPDATE,   // Called at fixed physics timestep
    HOOK_LATE_UPDATE,    // Called after all updates
    HOOK_ON_COLLISION,   // Physics collision events
    HOOK_ON_TRIGGER,     // Physics trigger events
    HOOK_ON_INPUT,       // Input events
    HOOK_ON_SCENE_LOAD,  // Scene loading events
    HOOK_CUSTOM          // Custom user-defined events
} HookType;

// Event data structures
typedef struct {
    vec3 position;
    vec3 velocity;
    uint32_t object_id;
} CollisionEvent;

typedef struct {
    uint32_t key_code;
    bool pressed;
    bool held;
    bool released;
} InputEvent;

typedef struct {
    char* scene_name;
    uint32_t object_count;
} SceneLoadEvent;

// Hook registration API
typedef void (*HookCallback)(HookType type, const void* data, size_t data_size);

bool engine_register_hook(HookType type, HookCallback callback);
void engine_unregister_hook(HookType type, HookCallback callback);
void engine_trigger_hook(HookType type, const void* data, size_t data_size);

// Specific hook triggers (called by engine systems)
void hooks_trigger_start(void);
void hooks_trigger_update(float delta_time);
void hooks_trigger_fixed_update(float fixed_delta);
void hooks_trigger_collision(const CollisionEvent* event);
void hooks_trigger_input(const InputEvent* event);

#endif
```

### 4. Thread-Safe Player API

**`src/api/player.h`**
```c
#ifndef PLAYER_API_H
#define PLAYER_API_H

#include "../core/types.h"
#include <pthread.h>

typedef struct PlayerState {
    vec3 position;
    vec3 velocity;
    float yaw;
    float pitch;
    bool on_ground;
    float health;
    uint32_t flags;
} PlayerState;

typedef struct PlayerConfig {
    float move_speed;
    float jump_force;
    float mouse_sensitivity;
    float collision_radius;
    float collision_height;
} PlayerConfig;

typedef struct PlayerCommand {
    vec3 move_direction;
    bool jump;
    bool sprint;
    vec2 look_delta;
} PlayerCommand;

// Player management (main thread only)
bool player_api_init(vec3 start_pos, float yaw, float pitch);
void player_api_destroy(void);
void player_api_update(void);

// Thread-safe state access
PlayerState player_api_get_state_safe(void);
void player_api_set_state_safe(PlayerState state);

// Thread-safe configuration
PlayerConfig player_api_get_config_safe(void);
void player_api_set_config_safe(PlayerConfig config);

// Command system (Python -> C)
void player_api_send_command(PlayerCommand cmd);
bool player_api_has_pending_commands(void);

// Input bindings (main thread only)
void player_api_set_input_enabled(bool enabled);
bool player_api_is_input_enabled(void);

// Event hooks
void player_api_register_move_hook(HookCallback callback);
void player_api_register_collision_hook(HookCallback callback);

#endif
```

### 3. Scene Management API
**`src/api/scene.h`**
```c
#ifndef SCENE_API_H
#define SCENE_API_H

#include "../core/types.h"

typedef struct ObjectHandle {
    uint32_t id;
    uint32_t scene_id;
} ObjectHandle;

// Scene management
bool scene_api_load(EngineContext* ctx, const char* scene_path, const char* objdb_path);
void scene_api_unload(EngineContext* ctx);

// Object queries
ObjectHandle scene_api_find_object(EngineContext* ctx, const char* name);
bool scene_api_object_exists(EngineContext* ctx, ObjectHandle handle);

// Object manipulation
vec3 scene_api_get_object_position(EngineContext* ctx, ObjectHandle handle);
void scene_api_set_object_position(EngineContext* ctx, ObjectHandle handle, vec3 pos);
vec3 scene_api_get_object_rotation(EngineContext* ctx, ObjectHandle handle);
void scene_api_set_object_rotation(EngineContext* ctx, ObjectHandle handle, vec3 rot);

// Object creation/destruction
ObjectHandle scene_api_spawn_object(EngineContext* ctx, const char* type, vec3 position);
void scene_api_destroy_object(EngineContext* ctx, ObjectHandle handle);

#endif
```

## Python Bindings Implementation

### 1. Native Python C API Integration

Using plain C with Python's native C API for maximum performance:

**`src/python/python_integration.h`**
```c
#ifndef PYTHON_INTEGRATION_H
#define PYTHON_INTEGRATION_H

#include <Python.h>
#include "../api/engine.h"
#include "../api/hooks.h"
#include "../api/threading.h"

// Python integration initialization
bool python_integration_init(void);
void python_integration_destroy(void);

// Python thread management
bool python_start_interpreter(EngineContext* ctx);
void python_stop_interpreter(void);

// Hook callback management
typedef struct {
    PyObject* callback;
    HookType type;
    bool enabled;
} PythonHook;

// Hook registration
bool python_register_hook(HookType type, PyObject* callback);
void python_unregister_hook(HookType type);
void python_trigger_hook(HookType type, const void* data, size_t data_size);

// Utility functions
PyObject* vec3_to_python(const vec3 v);
bool python_to_vec3(PyObject* obj, vec3 out);
PyObject* create_collision_dict(const CollisionEvent* event);
PyObject* create_input_dict(const InputEvent* event);

#endif
```

**`src/python/python_integration.c`**
```c
#include "python_integration.h"
#include "../core/log.h"
#include <stdio.h>

// Global Python state
static PyObject* python_hooks[HOOK_CUSTOM + 1] = {NULL};
static bool python_initialized = false;
static PyThreadState* main_thread_state = NULL;

// Forward declarations
static void python_hook_wrapper(HookType type, const void* data, size_t data_size);

bool python_integration_init(void) {
    if (python_initialized) return true;
    
    // Initialize Python interpreter
    Py_Initialize();
    if (!Py_IsInitialized()) {
        log_error("Failed to initialize Python interpreter");
        return false;
    }
    
    // Initialize threading
    PyEval_InitThreads();
    main_thread_state = PyEval_SaveThread();
    
    python_initialized = true;
    log_info("Python integration initialized");
    return true;
}

void python_integration_destroy(void) {
    if (!python_initialized) return;
    
    // Cleanup hooks
    for (int i = 0; i <= HOOK_CUSTOM; i++) {
        if (python_hooks[i]) {
            PyEval_RestoreThread(main_thread_state);
            Py_DECREF(python_hooks[i]);
            python_hooks[i] = NULL;
            main_thread_state = PyEval_SaveThread();
        }
    }
    
    // Finalize Python
    PyEval_RestoreThread(main_thread_state);
    Py_Finalize();
    python_initialized = false;
}

bool python_register_hook(HookType type, PyObject* callback) {
    if (!python_initialized || type > HOOK_CUSTOM) return false;
    
    PyEval_RestoreThread(main_thread_state);
    
    // Remove old callback if exists
    if (python_hooks[type]) {
        Py_DECREF(python_hooks[type]);
    }
    
    // Store new callback
    Py_INCREF(callback);
    python_hooks[type] = callback;
    
    // Register with engine
    engine_register_hook(type, python_hook_wrapper);
    
    main_thread_state = PyEval_SaveThread();
    return true;
}

void python_unregister_hook(HookType type) {
    if (!python_initialized || type > HOOK_CUSTOM) return;
    
    if (python_hooks[type]) {
        PyEval_RestoreThread(main_thread_state);
        Py_DECREF(python_hooks[type]);
        python_hooks[type] = NULL;
        main_thread_state = PyEval_SaveThread();
        
        engine_unregister_hook(type, python_hook_wrapper);
    }
}

static void python_hook_wrapper(HookType type, const void* data, size_t data_size) {
    if (!python_initialized || !python_hooks[type]) return;
    
    // Acquire GIL for Python operations
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    PyObject* py_data = NULL;
    
    // Convert C data to Python objects
    switch (type) {
        case HOOK_UPDATE: {
            float delta = *(const float*)data;
            py_data = PyFloat_FromDouble((double)delta);
            break;
        }
        case HOOK_FIXED_UPDATE: {
            float fixed_delta = *(const float*)data;
            py_data = PyFloat_FromDouble((double)fixed_delta);
            break;
        }
        case HOOK_ON_COLLISION: {
            const CollisionEvent* event = (const CollisionEvent*)data;
            py_data = create_collision_dict(event);
            break;
        }
        case HOOK_ON_INPUT: {
            const InputEvent* event = (const InputEvent*)data;
            py_data = create_input_dict(event);
            break;
        }
        case HOOK_START:
        case HOOK_LATE_UPDATE:
        default:
            py_data = Py_None;
            Py_INCREF(Py_None);
            break;
    }
    
    if (py_data) {
        // Call Python function
        PyObject* result = PyObject_CallFunctionObjArgs(python_hooks[type], py_data, NULL);
        
        if (result) {
            Py_DECREF(result);
        } else {
            // Print Python error but don't crash
            PyErr_Print();
            log_error("Error in Python hook callback");
        }
        
        Py_DECREF(py_data);
    }
    
    // Release GIL
    PyGILState_Release(gstate);
}

// Utility functions
PyObject* vec3_to_python(const vec3 v) {
    return PyTuple_Pack(3, 
        PyFloat_FromDouble(v[0]), 
        PyFloat_FromDouble(v[1]), 
        PyFloat_FromDouble(v[2])
    );
}

bool python_to_vec3(PyObject* obj, vec3 out) {
    if (!PyTuple_Check(obj) || PyTuple_Size(obj) != 3) {
        return false;
    }
    
    for (int i = 0; i < 3; i++) {
        PyObject* item = PyTuple_GetItem(obj, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            return false;
        }
        out[i] = (float)PyFloat_AsDouble(item);
    }
    
    return true;
}

PyObject* create_collision_dict(const CollisionEvent* event) {
    PyObject* dict = PyDict_New();
    
    PyDict_SetItemString(dict, "position", vec3_to_python(event->position));
    PyDict_SetItemString(dict, "velocity", vec3_to_python(event->velocity));
    PyDict_SetItemString(dict, "object_id", PyLong_FromUnsignedLong(event->object_id));
    
    return dict;
}

PyObject* create_input_dict(const InputEvent* event) {
    PyObject* dict = PyDict_New();
    
    PyDict_SetItemString(dict, "key_code", PyLong_FromUnsignedLong(event->key_code));
    PyDict_SetItemString(dict, "pressed", PyBool_FromLong(event->pressed));
    PyDict_SetItemString(dict, "held", PyBool_FromLong(event->held));
    PyDict_SetItemString(dict, "released", PyBool_FromLong(event->released));
    
    return dict;
}
```

**`src/python/python_module.c`**
```c
#include "python_integration.h"
#include "../api/player.h"
#include "../api/scene.h"

// Engine object wrapper
typedef struct {
    PyObject_HEAD
    EngineContext* ctx;
    bool owns_context;
} PyEngineObject;

// PlayerState object wrapper
typedef struct {
    PyObject_HEAD
    PlayerState state;
} PyPlayerStateObject;

// PlayerCommand object wrapper
typedef struct {
    PyObject_HEAD
    PlayerCommand command;
} PyPlayerCommandObject;

// Forward declarations
static PyTypeObject PyEngineType;
static PyTypeObject PyPlayerStateType;
static PyTypeObject PyPlayerCommandType;

// Engine methods
static PyObject* PyEngine_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    PyEngineObject* self = (PyEngineObject*)type->tp_alloc(type, 0);
    if (self) {
        self->ctx = engine_create();
        self->owns_context = true;
        if (!self->ctx) {
            Py_DECREF(self);
            PyErr_SetString(PyExc_RuntimeError, "Failed to create engine context");
            return NULL;
        }
    }
    return (PyObject*)self;
}

static void PyEngine_dealloc(PyEngineObject* self) {
    if (self->owns_context && self->ctx) {
        engine_destroy(self->ctx);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* PyEngine_init(PyEngineObject* self, PyObject* args) {
    if (!self->ctx) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid engine context");
        return NULL;
    }
    
    bool result = engine_init(self->ctx);
    if (result) {
        // Start Python thread
        engine_start_python_thread(self->ctx);
    }
    
    return PyBool_FromLong(result);
}

static PyObject* PyEngine_run(PyEngineObject* self, PyObject* args) {
    if (!self->ctx) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid engine context");
        return NULL;
    }
    
    // Release GIL during engine run (C main loop)
    Py_BEGIN_ALLOW_THREADS
    engine_run(self->ctx);
    Py_END_ALLOW_THREADS
    
    Py_RETURN_NONE;
}

static PyObject* PyEngine_stop(PyEngineObject* self, PyObject* args) {
    if (!self->ctx) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid engine context");
        return NULL;
    }
    
    engine_stop(self->ctx);
    Py_RETURN_NONE;
}

static PyObject* PyEngine_get_delta_time(PyEngineObject* self, PyObject* args) {
    if (!self->ctx) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid engine context");
        return NULL;
    }
    
    float delta = engine_get_delta_time(self->ctx);
    return PyFloat_FromDouble((double)delta);
}

static PyObject* PyEngine_get_frame_number(PyEngineObject* self, PyObject* args) {
    if (!self->ctx) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid engine context");
        return NULL;
    }
    
    uint64_t frame = engine_get_frame_number(self->ctx);
    return PyLong_FromUnsignedLongLong(frame);
}

static PyObject* PyEngine_register_hook(PyEngineObject* self, PyObject* args) {
    int hook_type;
    PyObject* callback;
    
    if (!PyArg_ParseTuple(args, "iO", &hook_type, &callback)) {
        return NULL;
    }
    
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "Callback must be callable");
        return NULL;
    }
    
    if (hook_type < 0 || hook_type > HOOK_CUSTOM) {
        PyErr_SetString(PyExc_ValueError, "Invalid hook type");
        return NULL;
    }
    
    bool result = python_register_hook((HookType)hook_type, callback);
    return PyBool_FromLong(result);
}

static PyObject* PyEngine_unregister_hook(PyEngineObject* self, PyObject* args) {
    int hook_type;
    
    if (!PyArg_ParseTuple(args, "i", &hook_type)) {
        return NULL;
    }
    
    if (hook_type < 0 || hook_type > HOOK_CUSTOM) {
        PyErr_SetString(PyExc_ValueError, "Invalid hook type");
        return NULL;
    }
    
    python_unregister_hook((HookType)hook_type);
    Py_RETURN_NONE;
}

// Engine method definitions
static PyMethodDef PyEngine_methods[] = {
    {"init", (PyCFunction)PyEngine_init, METH_NOARGS, "Initialize engine"},
    {"run", (PyCFunction)PyEngine_run, METH_NOARGS, "Run engine main loop"},
    {"stop", (PyCFunction)PyEngine_stop, METH_NOARGS, "Stop engine"},
    {"get_delta_time", (PyCFunction)PyEngine_get_delta_time, METH_NOARGS, "Get delta time"},
    {"get_frame_number", (PyCFunction)PyEngine_get_frame_number, METH_NOARGS, "Get frame number"},
    {"register_hook", (PyCFunction)PyEngine_register_hook, METH_VARARGS, "Register hook callback"},
    {"unregister_hook", (PyCFunction)PyEngine_unregister_hook, METH_VARARGS, "Unregister hook callback"},
    {NULL}
};

// Engine type definition
static PyTypeObject PyEngineType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "interlope_engine.Engine",
    .tp_basicsize = sizeof(PyEngineObject),
    .tp_dealloc = (destructor)PyEngine_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Interlope Engine",
    .tp_methods = PyEngine_methods,
    .tp_new = PyEngine_new,
};

// Module-level functions
static PyObject* py_player_get_state(PyObject* self, PyObject* args) {
    PlayerState state = player_api_get_state_safe();
    
    PyPlayerStateObject* py_state = (PyPlayerStateObject*)PyObject_CallObject((PyObject*)&PyPlayerStateType, NULL);
    if (py_state) {
        py_state->state = state;
    }
    
    return (PyObject*)py_state;
}

static PyObject* py_player_send_command(PyObject* self, PyObject* args) {
    PyPlayerCommandObject* py_cmd;
    
    if (!PyArg_ParseTuple(args, "O!", &PyPlayerCommandType, &py_cmd)) {
        return NULL;
    }
    
    player_api_send_command(py_cmd->command);
    Py_RETURN_NONE;
}

// Module method definitions
static PyMethodDef module_methods[] = {
    {"player_get_state", py_player_get_state, METH_NOARGS, "Get player state"},
    {"player_send_command", py_player_send_command, METH_VARARGS, "Send player command"},
    {NULL}
};

// Module definition
static struct PyModuleDef interlope_engine_module = {
    PyModuleDef_HEAD_INIT,
    "interlope_engine",
    "Interlope C Engine Python Bindings",
    -1,
    module_methods
};

// Module initialization
PyMODINIT_FUNC PyInit_interlope_engine(void) {
    PyObject* module;
    
    // Initialize Python integration
    if (!python_integration_init()) {
        return NULL;
    }
    
    // Prepare types
    if (PyType_Ready(&PyEngineType) < 0) return NULL;
    if (PyType_Ready(&PyPlayerStateType) < 0) return NULL;
    if (PyType_Ready(&PyPlayerCommandType) < 0) return NULL;
    
    // Create module
    module = PyModule_Create(&interlope_engine_module);
    if (!module) return NULL;
    
    // Add types to module
    Py_INCREF(&PyEngineType);
    PyModule_AddObject(module, "Engine", (PyObject*)&PyEngineType);
    
    Py_INCREF(&PyPlayerStateType);
    PyModule_AddObject(module, "PlayerState", (PyObject*)&PyPlayerStateType);
    
    Py_INCREF(&PyPlayerCommandType);
    PyModule_AddObject(module, "PlayerCommand", (PyObject*)&PyPlayerCommandType);
    
    // Add constants
    PyModule_AddIntConstant(module, "HOOK_START", HOOK_START);
    PyModule_AddIntConstant(module, "HOOK_UPDATE", HOOK_UPDATE);
    PyModule_AddIntConstant(module, "HOOK_FIXED_UPDATE", HOOK_FIXED_UPDATE);
    PyModule_AddIntConstant(module, "HOOK_LATE_UPDATE", HOOK_LATE_UPDATE);
    PyModule_AddIntConstant(module, "HOOK_ON_COLLISION", HOOK_ON_COLLISION);
    PyModule_AddIntConstant(module, "HOOK_ON_TRIGGER", HOOK_ON_TRIGGER);
    PyModule_AddIntConstant(module, "HOOK_ON_INPUT", HOOK_ON_INPUT);
    PyModule_AddIntConstant(module, "HOOK_ON_SCENE_LOAD", HOOK_ON_SCENE_LOAD);
    PyModule_AddIntConstant(module, "HOOK_CUSTOM", HOOK_CUSTOM);
    
    return module;
}
```

### 2. Alternative: ctypes Bindings (Simpler but less performant)

**`python/interlope/engine_ctypes.py`**
```python
import ctypes
from ctypes import Structure, c_float, c_bool, c_uint32, c_char_p, POINTER
import os

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '../../lib/libinterlope.so')
lib = ctypes.CDLL(lib_path)

# Define C structures
class Vec3(Structure):
    _fields_ = [("x", c_float), ("y", c_float), ("z", c_float)]

class PlayerState(Structure):
    _fields_ = [
        ("position", Vec3),
        ("velocity", Vec3), 
        ("yaw", c_float),
        ("pitch", c_float),
        ("on_ground", c_bool)
    ]

class PlayerConfig(Structure):
    _fields_ = [
        ("move_speed", c_float),
        ("jump_force", c_float),
        ("mouse_sensitivity", c_float),
        ("collision_radius", c_float),
        ("collision_height", c_float)
    ]

# Define function signatures
lib.engine_create.restype = ctypes.c_void_p
lib.engine_init.argtypes = [ctypes.c_void_p]
lib.engine_init.restype = c_bool
lib.engine_update.argtypes = [ctypes.c_void_p]
lib.engine_render.argtypes = [ctypes.c_void_p]
lib.engine_should_close.argtypes = [ctypes.c_void_p]
lib.engine_should_close.restype = c_bool

lib.player_api_get_state.restype = PlayerState
lib.player_api_set_state.argtypes = [PlayerState]

class Engine:
    def __init__(self):
        self.ctx = lib.engine_create()
        if not self.ctx:
            raise RuntimeError("Failed to create engine context")
    
    def __del__(self):
        if hasattr(self, 'ctx') and self.ctx:
            lib.engine_destroy(self.ctx)
    
    def init(self):
        return lib.engine_init(self.ctx)
    
    def update(self):
        lib.engine_update(self.ctx)
    
    def render(self):
        lib.engine_render(self.ctx)
    
    def should_close(self):
        return lib.engine_should_close(self.ctx)
```

## Python Framework Layer

### 1. Hook-Based Python Framework

**`python/interlope/engine.py`**
```python
import threading
from typing import Optional, Dict, Any, Callable, List
from .interlope_engine import Engine as _Engine, HookType
import time

class GameBehaviour:
    """Base class for game logic components (similar to Unity's MonoBehaviour)"""
    
    def __init__(self, name: str = "GameBehaviour"):
        self.name = name
        self.enabled = True
        self._started = False
    
    def start(self):
        """Called once when the behaviour starts"""
        pass
    
    def update(self, delta_time: float):
        """Called every frame"""
        pass
    
    def fixed_update(self, fixed_delta: float):
        """Called at fixed timestep for physics"""
        pass
    
    def late_update(self, delta_time: float):
        """Called after all updates"""
        pass
    
    def on_collision(self, collision_data: dict):
        """Called when collision occurs"""
        pass
    
    def on_input(self, input_data: dict):
        """Called on input events"""
        pass
    
    def on_scene_load(self, scene_data: dict):
        """Called when scene loads"""
        pass

class InterlopeEngine:
    """High-level Python engine interface using hooks"""
    
    def __init__(self):
        self._engine = _Engine()
        self._initialized = False
        self._behaviours: List[GameBehaviour] = []
        self._hook_registered = False
    
    def add_behaviour(self, behaviour: GameBehaviour):
        """Add a game behaviour to the engine"""
        self._behaviours.append(behaviour)
        
        # Register hooks if not already done
        if not self._hook_registered:
            self._register_hooks()
            self._hook_registered = True
    
    def remove_behaviour(self, behaviour: GameBehaviour):
        """Remove a game behaviour"""
        if behaviour in self._behaviours:
            self._behaviours.remove(behaviour)
    
    def init(self) -> bool:
        """Initialize the engine"""
        if self._initialized:
            return True
            
        if not self._engine.init():
            return False
        
        self._initialized = True
        return True
    
    def run(self):
        """Run the engine (blocks until engine stops)"""
        if not self._initialized:
            raise RuntimeError("Engine not initialized")
        
        # Start all behaviours
        for behaviour in self._behaviours:
            if behaviour.enabled and not behaviour._started:
                behaviour.start()
                behaviour._started = True
        
        # Run the main loop (C handles threading)
        self._engine.run()
    
    def stop(self):
        """Stop the engine"""
        self._engine.stop()
    
    def get_delta_time(self) -> float:
        """Get frame delta time"""
        return self._engine.get_delta_time()
    
    def get_frame_number(self) -> int:
        """Get current frame number"""
        return self._engine.get_frame_number()
    
    def _register_hooks(self):
        """Register engine hooks with C engine"""
        self._engine.register_hook(HookType.UPDATE, self._on_update)
        self._engine.register_hook(HookType.FIXED_UPDATE, self._on_fixed_update)
        self._engine.register_hook(HookType.LATE_UPDATE, self._on_late_update)
        self._engine.register_hook(HookType.ON_COLLISION, self._on_collision)
        self._engine.register_hook(HookType.ON_INPUT, self._on_input)
        self._engine.register_hook(HookType.ON_SCENE_LOAD, self._on_scene_load)
    
    def _on_update(self, delta_time):
        """Internal update hook"""
        try:
            for behaviour in self._behaviours:
                if behaviour.enabled:
                    behaviour.update(delta_time)
        except Exception as e:
            print(f"Error in update: {e}")
    
    def _on_fixed_update(self, fixed_delta):
        """Internal fixed update hook"""
        try:
            for behaviour in self._behaviours:
                if behaviour.enabled:
                    behaviour.fixed_update(fixed_delta)
        except Exception as e:
            print(f"Error in fixed_update: {e}")
    
    def _on_late_update(self, delta_time):
        """Internal late update hook"""
        try:
            for behaviour in self._behaviours:
                if behaviour.enabled:
                    behaviour.late_update(delta_time)
        except Exception as e:
            print(f"Error in late_update: {e}")
    
    def _on_collision(self, collision_data):
        """Internal collision hook"""
        try:
            for behaviour in self._behaviours:
                if behaviour.enabled:
                    behaviour.on_collision(collision_data)
        except Exception as e:
            print(f"Error in on_collision: {e}")
    
    def _on_input(self, input_data):
        """Internal input hook"""
        try:
            for behaviour in self._behaviours:
                if behaviour.enabled:
                    behaviour.on_input(input_data)
        except Exception as e:
            print(f"Error in on_input: {e}")
    
    def _on_scene_load(self, scene_data):
        """Internal scene load hook"""
        try:
            for behaviour in self._behaviours:
                if behaviour.enabled:
                    behaviour.on_scene_load(scene_data)
        except Exception as e:
            print(f"Error in on_scene_load: {e}")

# Global engine instance
engine = InterlopeEngine()
```

### 2. Python Game Logic Framework

**`python/interlope/game.py`**
```python
from abc import ABC, abstractmethod
from typing import Dict, Any
from .engine import engine
from .engine_bindings import player_init, player_update, player_get_state, player_set_state

class GameObject(ABC):
    """Base class for game objects"""
    
    def __init__(self, name: str):
        self.name = name
        self.active = True
        self.transform = Transform()
    
    @abstractmethod
    def update(self, delta_time: float):
        pass
    
    @abstractmethod  
    def render(self):
        pass

class Transform:
    """Transform component"""
    
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.position = [x, y, z]
        self.rotation = [0.0, 0.0, 0.0]
        self.scale = [1.0, 1.0, 1.0]

class Player(GameObject):
    """Python player controller"""
    
    def __init__(self):
        super().__init__("Player")
        self.move_speed = 5.0
        self.jump_force = 10.0
        self._initialized = False
    
    def init(self, start_pos=(0, 0, 0), yaw=0.0, pitch=0.0):
        """Initialize player"""
        if player_init(start_pos, yaw, pitch):
            self._initialized = True
            return True
        return False
    
    def update(self, delta_time: float):
        """Update player logic"""
        if not self._initialized:
            return
        
        # Update C player systems
        player_update()
        
        # Get updated state from C
        state = player_get_state()
        self.transform.position = [state.position.x, state.position.y, state.position.z]
        
        # Custom Python logic here
        self._update_custom_logic(delta_time)
    
    def _update_custom_logic(self, delta_time: float):
        """Override this for custom player logic"""
        pass
    
    def render(self):
        """Player rendering handled by C engine"""
        pass

class GameManager:
    """Manages game objects and state"""
    
    def __init__(self):
        self.objects: Dict[str, GameObject] = {}
        self.player: Optional[Player] = None
    
    def add_object(self, obj: GameObject):
        """Add game object"""
        self.objects[obj.name] = obj
    
    def remove_object(self, name: str):
        """Remove game object"""
        if name in self.objects:
            del self.objects[name]
    
    def create_player(self, start_pos=(0, 0, 0)) -> Player:
        """Create and initialize player"""
        self.player = Player()
        if self.player.init(start_pos):
            self.add_object(self.player)
            return self.player
        return None
    
    def update(self, engine_instance):
        """Update all game objects"""
        delta_time = engine_instance.get_delta_time()
        
        for obj in self.objects.values():
            if obj.active:
                obj.update(delta_time)
    
    def render(self, engine_instance):
        """Render all game objects"""
        for obj in self.objects.values():
            if obj.active:
                obj.render()

# Global game manager
game = GameManager()
```

## Example Usage

### 1. Unity-Style Game Script

**`examples/simple_game.py`**
```python
#!/usr/bin/env python3

from interlope.engine import engine, GameBehaviour
from interlope import player_get_state, player_send_command, PlayerCommand

class PlayerController(GameBehaviour):
    """Simple player controller using hooks"""
    
    def __init__(self):
        super().__init__("PlayerController")
        self.move_speed = 5.0
        self.sprint_speed = 10.0
        self.is_sprinting = False
    
    def start(self):
        """Initialize player controller"""
        print("Player controller started")
    
    def update(self, delta_time: float):
        """Update player logic every frame"""
        # Get current player state
        state = player_get_state()
        
        # Custom logic based on state
        if state.health < 50:
            print("Warning: Low health!")
    
    def fixed_update(self, fixed_delta: float):
        """Physics updates at fixed timestep"""
        # This is where you'd typically handle physics-based movement
        pass
    
    def on_input(self, input_data: dict):
        """Handle input events"""
        key_code = input_data.get('key_code', 0)
        pressed = input_data.get('pressed', False)
        
        if pressed:
            if key_code == 42:  # Sprint key
                self.is_sprinting = True
                print("Sprinting enabled")
            elif key_code == 32:  # Space (jump)
                # Send jump command to C engine
                cmd = PlayerCommand()
                cmd.jump = True
                player_send_command(cmd)
    
    def on_collision(self, collision_data: dict):
        """Handle collision events"""
        object_id = collision_data.get('object_id', 0)
        position = collision_data.get('position', [0, 0, 0])
        
        print(f"Player collided with object {object_id} at {position}")

class GameManager(GameBehaviour):
    """Main game logic manager"""
    
    def __init__(self):
        super().__init__("GameManager")
        self.game_time = 0.0
        self.score = 0
    
    def start(self):
        """Initialize game"""
        print("Game started!")
    
    def update(self, delta_time: float):
        """Update game state"""
        self.game_time += delta_time
        
        # Example: Increase score over time
        if int(self.game_time) % 10 == 0:  # Every 10 seconds
            self.score += 1
    
    def on_scene_load(self, scene_data: dict):
        """Handle scene loading"""
        scene_name = scene_data.get('scene_name', 'Unknown')
        print(f"Scene loaded: {scene_name}")

class DebugInfo(GameBehaviour):
    """Debug information display"""
    
    def __init__(self):
        super().__init__("DebugInfo")
        self.frame_count = 0
    
    def update(self, delta_time: float):
        """Update debug info"""
        self.frame_count += 1
        
        # Print FPS every 60 frames
        if self.frame_count % 60 == 0:
            fps = 1.0 / delta_time if delta_time > 0 else 0
            frame_num = engine.get_frame_number()
            print(f"Frame {frame_num}, FPS: {fps:.1f}")

def main():
    """Main entry point"""
    print("Starting Interlope Engine with Python game logic...")
    
    # Create game behaviours
    player_controller = PlayerController()
    game_manager = GameManager()
    debug_info = DebugInfo()
    
    # Add behaviours to engine
    engine.add_behaviour(player_controller)
    engine.add_behaviour(game_manager)
    engine.add_behaviour(debug_info)
    
    # Initialize and run
    if not engine.init():
        print("Failed to initialize engine")
        return 1
    
    print("Engine initialized. Starting main loop...")
    
    try:
        # This blocks until the engine stops
        engine.run()
    except KeyboardInterrupt:
        print("Received interrupt, stopping engine...")
        engine.stop()
    
    print("Engine stopped.")
    return 0

if __name__ == "__main__":
    exit(main())
```

### 2. Advanced AI Behaviour

**`examples/ai_behaviour.py`**
```python
from interlope.engine import GameBehaviour
from interlope import player_get_state, scene_find_object, scene_get_object_position
import random
import math

class EnemyAI(GameBehaviour):
    """AI enemy that chases the player"""
    
    def __init__(self, enemy_object_name: str):
        super().__init__(f"EnemyAI_{enemy_object_name}")
        self.enemy_name = enemy_object_name
        self.target_position = [0, 0, 0]
        self.chase_speed = 3.0
        self.detection_range = 10.0
        self.state = "patrol"  # patrol, chase, attack
        self.patrol_points = []
        self.current_patrol_index = 0
        
    def start(self):
        """Initialize AI"""
        # Setup patrol points around spawn location
        enemy_obj = scene_find_object(self.enemy_name)
        if enemy_obj:
            pos = scene_get_object_position(enemy_obj)
            self.setup_patrol_points(pos)
        
        print(f"Enemy AI {self.enemy_name} initialized")
    
    def setup_patrol_points(self, center_pos):
        """Create patrol points around center position"""
        self.patrol_points = [
            [center_pos[0] + 5, center_pos[1], center_pos[2]],
            [center_pos[0] - 5, center_pos[1], center_pos[2] + 5],
            [center_pos[0], center_pos[1], center_pos[2] - 5],
            center_pos.copy()
        ]
    
    def update(self, delta_time: float):
        """AI update loop"""
        # Get player and enemy positions
        player_state = player_get_state()
        enemy_obj = scene_find_object(self.enemy_name)
        
        if not enemy_obj:
            return
        
        enemy_pos = scene_get_object_position(enemy_obj)
        player_pos = player_state.position
        
        # Calculate distance to player
        distance = self.calculate_distance(enemy_pos, player_pos)
        
        # State machine
        if self.state == "patrol":
            self.update_patrol(enemy_pos, delta_time)
            
            # Switch to chase if player is nearby
            if distance < self.detection_range:
                self.state = "chase"
                print(f"{self.enemy_name} detected player!")
        
        elif self.state == "chase":
            self.update_chase(enemy_pos, player_pos, delta_time)
            
            # Return to patrol if player is too far
            if distance > self.detection_range * 1.5:
                self.state = "patrol"
                print(f"{self.enemy_name} lost sight of player")
    
    def update_patrol(self, current_pos, delta_time):
        """Update patrol behavior"""
        if not self.patrol_points:
            return
        
        target = self.patrol_points[self.current_patrol_index]
        distance = self.calculate_distance(current_pos, target)
        
        if distance < 1.0:  # Reached patrol point
            self.current_patrol_index = (self.current_patrol_index + 1) % len(self.patrol_points)
        
        # Move towards current patrol point
        self.move_towards(current_pos, target, self.chase_speed * 0.5, delta_time)
    
    def update_chase(self, current_pos, target_pos, delta_time):
        """Update chase behavior"""
        self.move_towards(current_pos, target_pos, self.chase_speed, delta_time)
    
    def move_towards(self, current_pos, target_pos, speed, delta_time):
        """Move enemy towards target position"""
        direction = [
            target_pos[0] - current_pos[0],
            target_pos[1] - current_pos[1], 
            target_pos[2] - current_pos[2]
        ]
        
        # Normalize direction
        length = math.sqrt(sum(d*d for d in direction))
        if length > 0:
            direction = [d/length for d in direction]
            
            # Calculate new position
            new_pos = [
                current_pos[0] + direction[0] * speed * delta_time,
                current_pos[1] + direction[1] * speed * delta_time,
                current_pos[2] + direction[2] * speed * delta_time
            ]
            
            # Update enemy position (would need scene_set_object_position)
            # scene_set_object_position(enemy_obj, new_pos)
    
    def calculate_distance(self, pos1, pos2):
        """Calculate 3D distance between two positions"""
        dx = pos1[0] - pos2[0]
        dy = pos1[1] - pos2[1] 
        dz = pos1[2] - pos2[2]
        return math.sqrt(dx*dx + dy*dy + dz*dz)
    
    def on_collision(self, collision_data: dict):
        """Handle collision with player or environment"""
        object_id = collision_data.get('object_id', 0)
        
        if self.state == "chase":
            # Attack player on collision
            print(f"{self.enemy_name} attacks player!")
            self.state = "attack"

class InventorySystem(GameBehaviour):
    """Simple inventory management"""
    
    def __init__(self):
        super().__init__("InventorySystem")
        self.items = {}
        self.max_items = 10
    
    def start(self):
        """Initialize inventory"""
        print("Inventory system initialized")
    
    def add_item(self, item_name: str, quantity: int = 1):
        """Add item to inventory"""
        if item_name in self.items:
            self.items[item_name] += quantity
        else:
            if len(self.items) < self.max_items:
                self.items[item_name] = quantity
                print(f"Added {quantity}x {item_name} to inventory")
            else:
                print("Inventory full!")
    
    def use_item(self, item_name: str, quantity: int = 1):
        """Use item from inventory"""
        if item_name in self.items and self.items[item_name] >= quantity:
            self.items[item_name] -= quantity
            if self.items[item_name] == 0:
                del self.items[item_name]
            print(f"Used {quantity}x {item_name}")
            return True
        return False
    
    def on_input(self, input_data: dict):
        """Handle inventory shortcuts"""
        key_code = input_data.get('key_code', 0)
        pressed = input_data.get('pressed', False)
        
        if pressed and key_code == 73:  # 'I' key
            self.print_inventory()
    
    def print_inventory(self):
        """Print current inventory"""
        print("=== Inventory ===")
        if not self.items:
            print("Empty")
        else:
            for item, quantity in self.items.items():
                print(f"  {item}: {quantity}")
        print("================")

# Example usage combining multiple behaviours
def create_game_with_ai():
    """Create a game with AI enemies and inventory"""
    from interlope.engine import engine
    
    # Create AI enemies
    enemy1 = EnemyAI("enemy_guard_1")
    enemy2 = EnemyAI("enemy_guard_2")
    
    # Create inventory system
    inventory = InventorySystem()
    
    # Add some initial items
    inventory.add_item("Health Potion", 3)
    inventory.add_item("Key Card", 1)
    
    # Add all behaviours to engine
    engine.add_behaviour(enemy1)
    engine.add_behaviour(enemy2)
    engine.add_behaviour(inventory)
    
    return engine
```

## Build System Integration

### 1. Updated Build Scripts

**`build_all.sh`**
```bash
#!/bin/sh
set -e

echo "Building C shared library..."
./build_lib.sh

echo "Building Python extension..."
cd python
python setup.py build_ext --inplace
cd ..

echo "Installing Python package..."
pip install -e python/

echo "Build complete!"
```

**`Makefile`**
```makefile
.PHONY: all clean lib python install

CC = gcc
CFLAGS = -std=c23 -fPIC -g -O2
LDFLAGS = -lm -lGL -lGLEW -lglfw -lcglm -lode -lpython3.11

SRC_DIR = src
LIB_DIR = lib
PYTHON_DIR = python

# Core engine sources (excluding main.c and python integration)
CORE_SOURCES = $(shell find $(SRC_DIR) -name "*.c" ! -name "main.c" ! -path "$(SRC_DIR)/python/*")
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)

# Python integration sources
PYTHON_SOURCES = $(shell find $(SRC_DIR)/python -name "*.c")
PYTHON_OBJECTS = $(PYTHON_SOURCES:.c=.o)

all: lib python

lib: $(LIB_DIR)/libinterlope.so

$(LIB_DIR)/libinterlope.so: $(CORE_OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -I/usr/include/python3.11 -c $< -o $@

python: lib
	python setup.py build_ext --inplace

install: python
	pip install -e ./

clean:
	rm -f $(CORE_OBJECTS) $(PYTHON_OBJECTS)
	rm -rf $(LIB_DIR)
	rm -rf build/
	rm -f interlope_engine*.so

test: install
	python examples/simple_game.py

# Development targets
dev-build: clean all

# Performance build
release: CFLAGS = -std=c23 -fPIC -O3 -DNDEBUG -march=native
release: clean all
```

## Memory Management & Safety

### 1. Resource Management
```c
// Resource tracking in engine context
typedef struct ResourcePool {
    void** resources;
    bool* active_flags;
    size_t capacity;
    size_t count;
    pthread_mutex_t mutex;
} ResourcePool;

// Safe resource allocation
uint32_t resource_pool_alloc(ResourcePool* pool, void* resource) {
    pthread_mutex_lock(&pool->mutex);
    
    // Find free slot
    for (size_t i = 0; i < pool->capacity; i++) {
        if (!pool->active_flags[i]) {
            pool->resources[i] = resource;
            pool->active_flags[i] = true;
            pool->count++;
            
            pthread_mutex_unlock(&pool->mutex);
            return i;
        }
    }
    
    // Expand pool if needed
    // ... resize logic ...
    
    pthread_mutex_unlock(&pool->mutex);
    return INVALID_HANDLE;
}
```

### 2. Python Reference Counting
```cpp
// Python object wrapper with automatic cleanup
class PythonResourceWrapper {
    uint32_t handle;
    EngineContext* ctx;
    
public:
    PythonResourceWrapper(EngineContext* c, uint32_t h) : ctx(c), handle(h) {}
    
    ~PythonResourceWrapper() {
        if (ctx && handle != INVALID_HANDLE) {
            engine_unload_resource(ctx, {handle, ""});
        }
    }
    
    uint32_t get_handle() const { return handle; }
};
```

## Error Handling & Debugging

### 1. C Error Codes
```c
typedef enum {
    ENGINE_OK = 0,
    ENGINE_ERROR_INIT_FAILED,
    ENGINE_ERROR_INVALID_CONTEXT,
    ENGINE_ERROR_RESOURCE_NOT_FOUND,
    ENGINE_ERROR_MEMORY_ALLOCATION,
    ENGINE_ERROR_FILE_IO,
    ENGINE_ERROR_OPENGL,
    ENGINE_ERROR_PHYSICS
} EngineResult;

const char* engine_get_error_string(EngineResult result);
```

### 2. Python Exception Mapping
```cpp
void throw_engine_exception(EngineResult result) {
    switch (result) {
        case ENGINE_ERROR_INIT_FAILED:
            throw std::runtime_error("Engine initialization failed");
        case ENGINE_ERROR_INVALID_CONTEXT:
            throw std::invalid_argument("Invalid engine context");
        case ENGINE_ERROR_RESOURCE_NOT_FOUND:
            throw std::runtime_error("Resource not found");
        // ... more cases
        default:
            throw std::runtime_error("Unknown engine error");
    }
}
```

## Performance Comparison: Native C API vs Alternatives

### 1. Performance Characteristics

**Native Python C API (Recommended)**
- **Call Overhead**: ~50-100ns per function call
- **GIL Management**: Explicit control, releases during C main loop
- **Memory**: Zero-copy for most operations, direct C struct access
- **Threading**: Full control over Python threading and GIL
- **Binary Size**: Smallest footprint

**Pybind11**  
- **Call Overhead**: ~200-500ns per function call
- **GIL Management**: Automatic but less control
- **Memory**: Additional wrapper overhead
- **Threading**: Good but less control
- **Binary Size**: Larger due to C++ templates

**ctypes**
- **Call Overhead**: ~1-5μs per function call  
- **GIL Management**: Automatic release/acquire
- **Memory**: Additional marshalling overhead
- **Threading**: Limited control
- **Binary Size**: No compilation needed

### 2. Lock-Free Message Queue Implementation

For maximum performance in the multithreaded architecture:

**`src/core/lockfree_queue.h`**
```c
#ifndef LOCKFREE_QUEUE_H
#define LOCKFREE_QUEUE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

// Single-producer, single-consumer lock-free queue
typedef struct {
    void** buffer;
    atomic_size_t head;
    atomic_size_t tail;
    size_t capacity;
    size_t mask;  // capacity - 1 (for power-of-2 sizes)
} SPSCQueue;

SPSCQueue* spsc_queue_create(size_t capacity);
void spsc_queue_destroy(SPSCQueue* queue);
bool spsc_queue_push(SPSCQueue* queue, void* item);
bool spsc_queue_pop(SPSCQueue* queue, void** item);
size_t spsc_queue_size(SPSCQueue* queue);
bool spsc_queue_empty(SPSCQueue* queue);

#endif
```

**`src/core/lockfree_queue.c`**
```c
#include "lockfree_queue.h"
#include <stdlib.h>

SPSCQueue* spsc_queue_create(size_t capacity) {
    // Ensure capacity is power of 2
    size_t actual_capacity = 1;
    while (actual_capacity < capacity) {
        actual_capacity <<= 1;
    }
    
    SPSCQueue* queue = malloc(sizeof(SPSCQueue));
    if (!queue) return NULL;
    
    queue->buffer = calloc(actual_capacity, sizeof(void*));
    if (!queue->buffer) {
        free(queue);
        return NULL;
    }
    
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    queue->capacity = actual_capacity;
    queue->mask = actual_capacity - 1;
    
    return queue;
}

void spsc_queue_destroy(SPSCQueue* queue) {
    if (queue) {
        free(queue->buffer);
        free(queue);
    }
}

bool spsc_queue_push(SPSCQueue* queue, void* item) {
    size_t head = atomic_load_explicit(&queue->head, memory_order_relaxed);
    size_t next_head = (head + 1) & queue->mask;
    
    if (next_head == atomic_load_explicit(&queue->tail, memory_order_acquire)) {
        return false; // Queue full
    }
    
    queue->buffer[head] = item;
    atomic_store_explicit(&queue->head, next_head, memory_order_release);
    return true;
}

bool spsc_queue_pop(SPSCQueue* queue, void** item) {
    size_t tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);
    
    if (tail == atomic_load_explicit(&queue->head, memory_order_acquire)) {
        return false; // Queue empty
    }
    
    *item = queue->buffer[tail];
    atomic_store_explicit(&queue->tail, (tail + 1) & queue->mask, memory_order_release);
    return true;
}

size_t spsc_queue_size(SPSCQueue* queue) {
    size_t head = atomic_load_explicit(&queue->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&queue->tail, memory_order_acquire);
    return (head - tail) & queue->mask;
}

bool spsc_queue_empty(SPSCQueue* queue) {
    size_t head = atomic_load_explicit(&queue->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&queue->tail, memory_order_acquire);
    return head == tail;
}
```

### 3. High-Performance Hook System

**`src/api/fast_hooks.h`**
```c
#ifndef FAST_HOOKS_H
#define FAST_HOOKS_H

#include <stdint.h>
#include <stdbool.h>

// Pre-allocated hook slots for maximum performance
#define MAX_HOOKS_PER_TYPE 4
#define HOOK_INVALID_ID UINT32_MAX

typedef struct {
    void (*callback)(const void* data, size_t size, void* userdata);
    void* userdata;
    bool enabled;
    uint32_t id;
} FastHook;

typedef struct {
    FastHook hooks[MAX_HOOKS_PER_TYPE];
    uint32_t count;
    uint32_t next_id;
} FastHookRegistry;

// Global hook registries (one per hook type)
extern FastHookRegistry g_hook_registries[HOOK_CUSTOM + 1];

// Hook management (thread-safe)
uint32_t fast_hook_register(HookType type, void (*callback)(const void*, size_t, void*), void* userdata);
bool fast_hook_unregister(HookType type, uint32_t hook_id);
void fast_hook_trigger(HookType type, const void* data, size_t size);

// Initialize/cleanup
bool fast_hooks_init(void);
void fast_hooks_destroy(void);

#endif
```

**`src/api/fast_hooks.c`**
```c
#include "fast_hooks.h"
#include "hooks.h"
#include <string.h>
#include <pthread.h>

// Global registries
FastHookRegistry g_hook_registries[HOOK_CUSTOM + 1];
static pthread_mutex_t hook_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool hooks_initialized = false;

bool fast_hooks_init(void) {
    if (hooks_initialized) return true;
    
    memset(g_hook_registries, 0, sizeof(g_hook_registries));
    hooks_initialized = true;
    return true;
}

void fast_hooks_destroy(void) {
    pthread_mutex_lock(&hook_mutex);
    memset(g_hook_registries, 0, sizeof(g_hook_registries));
    hooks_initialized = false;
    pthread_mutex_unlock(&hook_mutex);
}

uint32_t fast_hook_register(HookType type, void (*callback)(const void*, size_t, void*), void* userdata) {
    if (type > HOOK_CUSTOM || !callback) return HOOK_INVALID_ID;
    
    pthread_mutex_lock(&hook_mutex);
    
    FastHookRegistry* registry = &g_hook_registries[type];
    
    if (registry->count >= MAX_HOOKS_PER_TYPE) {
        pthread_mutex_unlock(&hook_mutex);
        return HOOK_INVALID_ID;
    }
    
    // Find empty slot
    for (uint32_t i = 0; i < MAX_HOOKS_PER_TYPE; i++) {
        if (!registry->hooks[i].enabled) {
            registry->hooks[i].callback = callback;
            registry->hooks[i].userdata = userdata;
            registry->hooks[i].enabled = true;
            registry->hooks[i].id = registry->next_id++;
            registry->count++;
            
            uint32_t hook_id = registry->hooks[i].id;
            pthread_mutex_unlock(&hook_mutex);
            return hook_id;
        }
    }
    
    pthread_mutex_unlock(&hook_mutex);
    return HOOK_INVALID_ID;
}

bool fast_hook_unregister(HookType type, uint32_t hook_id) {
    if (type > HOOK_CUSTOM) return false;
    
    pthread_mutex_lock(&hook_mutex);
    
    FastHookRegistry* registry = &g_hook_registries[type];
    
    for (uint32_t i = 0; i < MAX_HOOKS_PER_TYPE; i++) {
        if (registry->hooks[i].enabled && registry->hooks[i].id == hook_id) {
            registry->hooks[i].enabled = false;
            registry->hooks[i].callback = NULL;
            registry->hooks[i].userdata = NULL;
            registry->count--;
            
            pthread_mutex_unlock(&hook_mutex);
            return true;
        }
    }
    
    pthread_mutex_unlock(&hook_mutex);
    return false;
}

void fast_hook_trigger(HookType type, const void* data, size_t size) {
    if (type > HOOK_CUSTOM) return;
    
    FastHookRegistry* registry = &g_hook_registries[type];
    
    // Read-only access, no lock needed for triggering
    for (uint32_t i = 0; i < MAX_HOOKS_PER_TYPE; i++) {
        if (registry->hooks[i].enabled && registry->hooks[i].callback) {
            registry->hooks[i].callback(data, size, registry->hooks[i].userdata);
        }
    }
}
```

## Key Performance Benefits of Native C API:

✅ **Minimal Overhead**: Direct function calls, no C++ template overhead  
✅ **GIL Control**: Explicit GIL release during C main loop for maximum concurrency  
✅ **Zero-Copy**: Direct access to C structures without marshalling  
✅ **Lock-Free Queues**: SPSC queues for optimal thread communication  
✅ **Fast Hooks**: Pre-allocated hook slots, minimal branching  
✅ **Memory Efficient**: No additional wrapper layers  
✅ **Predictable**: No hidden allocations or exceptions  

The native C API approach provides maximum performance while maintaining the multithreaded hook-based architecture. This gives you the speed of C with the flexibility of Python scripting.
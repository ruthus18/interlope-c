# Interlope Game Engine

A lightweight 3D game engine written in C, featuring modern OpenGL rendering, physics simulation, and scene management capabilities.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/badge/version-0.0.1a-green.svg)

## Features

- Modern OpenGL-based rendering pipeline
- Physics simulation using ODE (Open Dynamics Engine)
- GLTF model loading with texture support (PNG/DDS)
- Scene management with TOML-based configuration
- Built-in editor with object manipulation
- First-person camera controller
- Cross-platform support *(at now primarly supporting Linux, 3rd party libraries and build system should be adjusted for Windows)*

## Dependencies

- OpenGL 4.x
- GLFW
- ODE (Open Dynamics Engine)
- cglm

## Building

```bash
# Clone the repository
git clone https://github.com/ruthus18/interlope-c.git

# Build the engine
./build.sh
```

## Quick Start

**IMPORTANT:** I cannot share assets due to they non-CC0 licensing, so you should add your own assets to corresponding directory and define they usage in `objects.toml`

1. Run the engine:
```bash
./run.sh
```

2. Controls:
- WASD - Movement
- Mouse - Look around
- F1 - Toggle editor
- ESC - Exit
- ~ - Toggle cursor


## Project Structure
```
/assets    - Models, textures, fonts, etc.
/data      - Scene and game objects configurations
/shaders   - GLSL shaders source
/src       - Engine source code
/vendor    - Third-party libraries
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


## Acknowledgments

- GLFW for window management
- CGLM for mathematics
- ODE for physics simulation
- STB for image loading
- TOML for configuration

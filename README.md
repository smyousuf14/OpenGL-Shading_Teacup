## OpenGL Teacup With Various Shading techniques

# 3D Teapot with Dynamic Lighting

![Teapot Demo](https://i.imgur.com/3YVJk0L.png)

A real-time 3D teapot visualization with multiple shading techniques and interactive controls. Implements Phong, Blinn-Phong, and Gouraud shading models with dynamic color control and camera manipulation.

## Features

- Three shading models:
  - Phong Shading (per-pixel lighting)
  - Blinn-Phong Shading (optimized specular)
  - Gouraud Shading (per-vertex lighting)
- Interactive controls:
  - Camera rotation & zoom
  - Real-time color manipulation
  - Shading model switching
- Dynamic lighting system with:
  - Adjustable specular highlights
  - Directional lighting
  - Ambient/diffuse/specular components

## Requirements

- C++17 compatible compiler
- OpenGL 3.3+
- GLFW 3.3+
- GLM 0.9.9+
- [GLAD](https://glad.dav1d.de/) (included in source)
- OBJ model file (teapot.obj)

## Installation

1. **Install Dependencies**:
   ```bash
   # Using vcpkg (recommended)
   vcpkg install glfw3 glm

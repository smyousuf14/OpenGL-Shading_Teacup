## OpenGL Teacup With Various Shading techniques

# 3D Teapot with Dynamic Lighting

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
2. **Place teapot.obj in project root**
3. Build project:
   ```bash
   mkdir build && cd build
   cmake .. && make

Controls
| **Key**   | **Action**               |
|-----------|--------------------------|
| W/S       | Rotate up/down           |
| A/D       | Rotate left/right        |
| Q/E       | Zoom in/out              |
| R/G/B     | Select color channel     |
| T/Y       | Adjust color intensity   |
| V         | Phong Shading            |
| B         | Blinn-Phong Shading      |
| N         | Gouraud Shading          |
| ESC       | Quit                     |

# Technical Implementation

## Shading Models

### Phong Shading
- Per-pixel lighting
- Classic reflection model
- **GLSL Implementation**:
  ```glsl
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

## Blinn-Phong
- **Optimized specular calculation**  
- **Uses halfway vector**  

**GLSL Implementation:**  
```glsl
vec3 halfwayDir = normalize(lightDir + viewDir);
float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
```

## Gouraud

- **Per-vertex lighting**
- **Faster** but **less precise**
- Lighting calculations in **vertex shader**

```glsl
// Example vertex shader code
void main() {
    // Lighting calculations performed here
    gl_Position = projection * view * model * vec4(position, 1.0);
}
```

## Matrix Pipeline

```cpp
glm::mat4 model = /* rotation transforms */;
glm::mat4 view = glm::lookAt(eye, center, up);
glm::mat4 projection = glm::perspective(fov, aspect, near, far);
glm::mat4 mvp = projection * view * model;
```

## Known Issues

- Requires OBJ file with proper normals  
- Color values can exceed `[0,1]` range  
- High-poly models may impact performance

## License

To use:
1. Copy this entire text
2. Save as `README.md` in your project root
3. Add a `teapot.jpg` screenshot (optional)
4. Create a matching `LICENSE` file
🚀

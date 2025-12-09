# Fractal

C++ OpenGL application for exploring the Mandelbox fractal with true infinite zoom using log-domain coordinates and adaptive detail scaling.

![Fractal <img width="1183" height="1075" alt="Screenshot 2025-12-02 142428" src="https://github.com/user-attachments/assets/6712817f-5a91-49f6-a116-e4b8179999c9" />]
<img width="1886" height="1007" alt="image" src="https://github.com/user-attachments/assets/5372030f-f00c-4c4c-aca7-1f94fb266603" />


## Features

- **True Infinite Zoom**: Log-domain coordinate system (mantissa + exponent) enables exploration at any scale
- **Adaptive Detail**: Iteration count automatically increases as you approach the fractal surface
- **Real-time Ray Marching**: GPU-accelerated distance estimation rendering
- **Smooth Navigation**: Camera speed automatically adjusts based on zoom level
- **Beautiful Rendering**: 2x2 anti-aliasing, orbit trap coloring, soft shadows, ambient occlusion
- **Fixed Starfield**: Non-rotating background for consistent spatial reference

### Prerequisites

- **CMake** 3.15 or higher
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **OpenGL 4.6** capable GPU and drivers
- **GLFW 3.3+** (must be installed on your system)

**GLM and GLAD are already included in the repository** in the `include/` directory.

### Installing GLFW

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libglfw3-dev
```

#### macOS
```bash
brew install glfw
```

#### Windows
Using vcpkg:
```cmd
vcpkg install glfw3:x64-windows
```

Or download pre-compiled binaries from https://www.glfw.org/download.html

### Building

```bash
# Clone the repository
git clone https://github.com/yourusername/fractal-explorer.git
cd fractal-explorer

# Build
mkdir build
cd build
cmake ..
make  # or 'cmake --build .'

# Run
./FractalExplorer
```

#### Windows (Visual Studio)
```cmd
git clone https://github.com/yourusername/fractal-explorer.git
cd fractal-explorer
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
Release\FractalExplorer.exe
```

If using vcpkg on Windows:
```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
```

### System-Specific Notes

**Linux**: You may need OpenGL development files:
```bash
sudo apt-get install libgl1-mesa-dev  # Ubuntu/Debian
sudo dnf install mesa-libGL-devel     # Fedora
```

**macOS**: Ensure Xcode Command Line Tools are installed:
```bash
xcode-select --install
```

**Windows**: OpenGL should be included with your graphics drivers. Make sure they're up to date.

## Controls

### Movement
- **W/A/S/D** - Move forward/left/backward/right (speed auto-adjusts with zoom)
- **Space** - Move up
- **Left Shift** - Move down
- **Mouse** - Look around
- **Mouse Scroll** - Adjust base movement speed multiplier

### Zoom
- **Q** - Zoom OUT
- **E** - Zoom IN

### Quality
- **1** - Decrease base iteration count (faster, less detail)
- **2** - Increase base iteration count (slower, more detail)

### Other
- **R** - Toggle auto-rotation
- **ESC** - Exit

### On-Screen Display
The title bar shows real-time stats:
- **FPS**: Current frame rate
- **Zoom Level**: Current exponent (2^N scale)
- **Max Iterations**: Current base iteration count
- **Speed**: Current movement speed

## How to Explore

1. **Start**: Launch the program - you'll see the Mandelbox from a distance
2. **Navigate**: Use WASD to fly around, mouse to look
3. **Zoom In**: Press E repeatedly to zoom into interesting structures
4. **Adjust Speed**: If moving too fast/slow, use mouse scroll wheel, it should automatically adjust as you increase detail
5. **Fine-tune Quality**: Press 1/2 to adjust base iteration count (start at 12-24)

### Interesting Features to Find
- **Cubic structures** - The Mandelbox has distinct box-like formations
- **Spherical inversions** - Look for rounded bulges and indentations
- **Self-similarity** - Structures repeat at different scales
- **Fine tendrils** - Zoom in close to see intricate details
- **Colorful patterns** - Orbit trap coloring reveals beautiful gradients

### Limitations/To Do
- Performance is still not great- requires a pretty good GPU, looking into a number of optimization techniques
- Floating point precision means that infinite zoom is impossible, could be fixed using Perturbation Theory techniques
- It would be nice to have the zoom automatically adjust, still brainstorming

## Performance Tips

- **Start with 12-24 base iterations** for smooth exploration
- **Adjust Iterations** some areas look better or worse based on the iterations, play around with them!
- **Lower your screen resolution** if framerate drops
- Anti-aliasing can be disabled in `shaders/fragment.glsl` for ~4× performance boost (change AA loops from 2 to 1)

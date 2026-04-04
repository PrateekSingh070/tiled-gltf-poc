# glTF Export Proof-of-Concept

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)
![glTF 2.0](https://img.shields.io/badge/glTF-2.0-86CEF9)

Standalone C++ program that writes a **valid** glTF 2.0 (`.gltf` + `.bin`) from a small orthogonal tile grid, using the same tile-to-screen and atlas UV ideas as [Tiled](https://www.mapeditor.org/). This is the technical foundation for a Tiled **glTF export** plugin; see [issue #2741](https://github.com/mapeditor/tiled/issues/2741) for upstream context.

**Author:** [PrateekSingh070](https://github.com/PrateekSingh070) · **Related upstream work:** [Tiled PR #4419](https://github.com/mapeditor/tiled/pull/4419), [PR #4465](https://github.com/mapeditor/tiled/pull/4465)

## What it does

- Creates a 2x2 orthogonal tile grid (4 tiles)
- Computes vertex positions from tile grid coordinates (same math as Tiled's `OrthogonalRenderer::tileToScreenCoords`)
- Computes UV coordinates from tile IDs + tileset atlas parameters (margin, spacing, column count)
- Packs geometry into a binary buffer with proper alignment
- Outputs a valid `.gltf` + `.bin` using the [cgltf](https://github.com/jkuhlmann/cgltf) library
- Uses `GL_NEAREST` sampling for pixel-art fidelity
- Uses `KHR_materials_unlit` extension for flat 2D rendering

## Build

```bash
g++ -std=c++17 -o gltf_poc main.cpp
```

## Run

```bash
./gltf_poc
```

Generates `output.gltf` and `output.bin`.

## Validate

Open `output.gltf` in https://gltf-viewer.donmccurdy.com/ (place a 64x64 `tileset.png` in the same directory).

## Relevance to Tiled

This program demonstrates:
- The **tile-to-quad** conversion that the Tiled glTF exporter will perform for every cell
- The **UV computation** from tileset atlas parameters (`margin`, `spacing`, `columnCount`, `imageWidth`)
- The **cgltf write API** workflow: build data structure in memory, write binary buffer, write .gltf JSON
- The **glTF buffer/bufferView/accessor** hierarchy for packing geometry data

## Repository topics (for GitHub)

Suggested tags: `gltf` · `gltf2` · `tiled` · `map-editor` · `cgltf` · `c++` · `game-tools` · `proof-of-concept`

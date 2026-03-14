# glTF Export Proof-of-Concept

A standalone C++ program that generates a valid glTF 2.0 file from a simulated 2x2 tile map, demonstrating the core building block for the Tiled glTF export plugin.

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

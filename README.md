# glTF Export Proof-of-Concept

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=c%2B%2B&logoColor=white)
![glTF 2.0](https://img.shields.io/badge/glTF-2.0-86CEF9)

This repo shows **valid glTF 2.0** output with [cgltf](https://github.com/jkuhlmann/cgltf), as groundwork for a Tiled **glTF export** plugin ([issue #2741](https://github.com/mapeditor/tiled/issues/2741)).

**Author:** [PrateekSingh070](https://github.com/PrateekSingh070) · **Tiled upstream:** [PR #4419](https://github.com/mapeditor/tiled/pull/4419), [PR #4465](https://github.com/mapeditor/tiled/pull/4465)

## Demos in this repository

### 1. Tile grid (C++) — `main.cpp`

Standalone program that writes a **valid** `.gltf` + `.bin` from a small **orthogonal tile grid** (2×2), using the same tile-to-screen and atlas UV ideas as [Tiled](https://www.mapeditor.org/).

- Vertex positions from grid coordinates (aligned with `OrthogonalRenderer::tileToScreenCoords`)
- UVs from tile IDs + atlas parameters (margin, spacing, column count)
- `GL_NEAREST` sampling · `KHR_materials_unlit` for flat 2D

```bash
g++ -std=c++17 -o gltf_poc main.cpp
./gltf_poc
```

Generates `output.gltf` and `output.bin`. Use a 64×64 `tileset.png` beside them for the viewer.

### 2. Single quad (C) — `gltf_quad_poc.c`

Minimal program: one **textured quad** (two triangles), with an embedded checkerboard `quad.png`. Simpler slice of what the exporter does **per tile**.

If needed, refresh cgltf headers:

```bash
curl -O https://raw.githubusercontent.com/jkuhlmann/cgltf/master/cgltf.h
curl -O https://raw.githubusercontent.com/jkuhlmann/cgltf/master/cgltf_write.h
```

```bash
gcc -o gltf_quad_poc gltf_quad_poc.c -lm
./gltf_quad_poc
```

Produces `quad.gltf`, `quad.bin`, and `quad.png`.

## Validate

- Viewer: https://gltf-viewer.donmccurdy.com/
- CLI: `npx gltf-validator quad.gltf` (or your generated `.gltf`)

## Relevance to Tiled

- **Tile-to-quad** conversion the glTF exporter will apply per map cell
- **UV computation** from tileset atlas parameters (`margin`, `spacing`, `columnCount`, `imageWidth`)
- **cgltf write path**: in-memory scene → binary buffer → `.gltf` JSON
- **glTF** `buffer` / `bufferView` / `accessor` layout for mesh data

## Repository topics (GitHub)

Suggested tags: `gltf` · `gltf2` · `tiled` · `map-editor` · `cgltf` · `c++` · `game-tools` · `proof-of-concept`

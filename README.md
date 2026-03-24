# glTF Quad Proof-of-Concept

Minimal C program that uses [cgltf](https://github.com/jkuhlmann/cgltf) to generate a valid `.gltf` + `.bin` file containing a single textured quad (two triangles). This is a simplified version of what the Tiled glTF export plugin will produce per tile.

## Setup

Download cgltf headers into this directory:

```bash
curl -O https://raw.githubusercontent.com/jkuhlmann/cgltf/master/cgltf.h
curl -O https://raw.githubusercontent.com/jkuhlmann/cgltf/master/cgltf_write.h
```

## Build

```bash
gcc -o gltf_quad_poc gltf_quad_poc.c -lm
```

Or with MSVC:

```bash
cl gltf_quad_poc.c
```

## Run

```bash
./gltf_quad_poc
```

Produces `quad.gltf` and `quad.bin`.

## Validate

Open `quad.gltf` in https://gltf-viewer.donmccurdy.com/ or run:

```bash
npx gltf-validator quad.gltf
```

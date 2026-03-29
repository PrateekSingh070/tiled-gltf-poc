/*
 * gltf_quad_poc.c
 *
 * Proof-of-concept: generates a valid .gltf + .bin file containing a
 * single textured quad (two triangles), similar to what one Tiled tile
 * would look like in the final exporter.
 *
 * Uses cgltf (https://github.com/jkuhlmann/cgltf) for writing.
 *
 * Build:
 *   gcc -o gltf_quad_poc gltf_quad_poc.c -lm
 *
 * Validate output:
 *   Open quad.gltf in https://gltf-viewer.donmccurdy.com/
 *   or run: npx gltf-validator quad.gltf
 */

#define CGLTF_WRITE_IMPLEMENTATION
#include "cgltf_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Quad vertex data: position (xyz) + texcoord (uv) */
static const float positions[] = {
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
};

static const float texcoords[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
};

static const unsigned short indices[] = {
    0, 1, 2,
    0, 2, 3,
};

/* 128x128 RGBA checkerboard (16px cells); high enough res for sharp preview. */
static const unsigned char quad_png[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x08, 0x06, 0x00, 0x00, 0x00, 0xc3, 0x3e, 0x61,
    0xcb, 0x00, 0x00, 0x01, 0x73, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0xed, 0xdc, 0xb1, 0x0d, 0x00,
    0x31, 0x08, 0x04, 0x41, 0x4a, 0xa3, 0xff, 0x1a, 0xdc, 0x0b, 0x14, 0x41, 0x60, 0x09, 0x26, 0xf8,
    0xf8, 0xa4, 0xdf, 0x09, 0x91, 0xa3, 0xea, 0xd5, 0xe4, 0xcb, 0x9c, 0x7d, 0xf6, 0xff, 0xee, 0x87,
    0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00,
    0x00, 0x08, 0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x00,
    0x08, 0x00, 0x80, 0x00, 0x2b, 0x01, 0xf8, 0x81, 0xb7, 0xf7, 0x01, 0x00, 0x40, 0x00, 0x00, 0x04,
    0x00, 0x40, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x00, 0x04, 0x00,
    0x40, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x00, 0x04, 0xd8, 0x09,
    0xc0, 0x0f, 0x74, 0x11, 0x24, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00,
    0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00,
    0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x80, 0xf7, 0x01, 0xfc, 0x40, 0x17, 0x41, 0x02,
    0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00,
    0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20,
    0x00, 0x00, 0x02, 0x78, 0x1f, 0x40, 0x40, 0x17, 0x41, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00,
    0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20,
    0x00, 0x00, 0xf6, 0x01, 0xb0, 0x0f, 0x80, 0x7d, 0x00, 0xec, 0x03, 0x60, 0xdf, 0xfb, 0x00, 0xf6,
    0x5d, 0x04, 0xd9, 0x07, 0xc0, 0x3e, 0x00, 0xf6, 0x01, 0xb0, 0x0f, 0x80, 0x7d, 0x00, 0xec, 0x03,
    0x60, 0x1f, 0x00, 0xfb, 0x00, 0xd8, 0x07, 0xc0, 0x3e, 0x00, 0xf6, 0x01, 0xb0, 0x0f, 0x80, 0x7d,
    0x00, 0xec, 0x03, 0x60, 0xdf, 0xfb, 0x00, 0xf6, 0x5d, 0x04, 0xd9, 0x07, 0xc0, 0x3e, 0x00, 0xf6,
    0x01, 0xb0, 0x0f, 0x80, 0x7d, 0x00, 0xec, 0x03, 0x60, 0x1f, 0x00, 0xfb, 0x00, 0xd8, 0x07, 0xc0,
    0x3e, 0x00, 0xf6, 0x01, 0xb0, 0x0f, 0x80, 0x7d, 0x00, 0xec, 0x03, 0x60, 0xdf, 0xfb, 0x00, 0xf6,
    0x5d, 0x04, 0xd9, 0x07, 0xc0, 0x3e, 0x00, 0xf6, 0x01, 0xb0, 0x0f, 0x00, 0x00, 0x02, 0x00, 0x20,
    0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00,
    0x00, 0x02, 0x00, 0x20, 0xc0, 0x81, 0xfd, 0x06, 0x9d, 0xc7, 0x8b, 0x9b, 0xb2, 0x4b, 0xd8, 0x27,
    0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static int write_png_file(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(quad_png, 1, sizeof(quad_png), f);
    fclose(f);
    return 1;
}

static int write_bin_file(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    fwrite(positions, sizeof(positions), 1, f);
    fwrite(texcoords, sizeof(texcoords), 1, f);
    fwrite(indices, sizeof(indices), 1, f);

    fclose(f);
    return 1;
}

int main(void) {
    cgltf_size pos_size = sizeof(positions);
    cgltf_size uv_size = sizeof(texcoords);
    cgltf_size idx_size = sizeof(indices);

    /* Buffer */
    cgltf_buffer buffer = {0};
    buffer.uri = "quad.bin";
    buffer.size = pos_size + uv_size + idx_size;

    /* Buffer views */
    cgltf_buffer_view views[3] = {0};

    views[0].buffer = &buffer;
    views[0].offset = 0;
    views[0].size = pos_size;
    views[0].stride = 3 * sizeof(float);
    views[0].type = cgltf_buffer_view_type_vertices;

    views[1].buffer = &buffer;
    views[1].offset = pos_size;
    views[1].size = uv_size;
    views[1].stride = 2 * sizeof(float);
    views[1].type = cgltf_buffer_view_type_vertices;

    views[2].buffer = &buffer;
    views[2].offset = pos_size + uv_size;
    views[2].size = idx_size;
    views[2].type = cgltf_buffer_view_type_indices;

    /* Accessors */
    cgltf_accessor accessors[3] = {0};

    accessors[0].buffer_view = &views[0];
    accessors[0].component_type = cgltf_component_type_r_32f;
    accessors[0].type = cgltf_type_vec3;
    accessors[0].count = 4;
    accessors[0].has_min = 1;
    accessors[0].min[0] = 0.0f; accessors[0].min[1] = 0.0f; accessors[0].min[2] = 0.0f;
    accessors[0].has_max = 1;
    accessors[0].max[0] = 1.0f; accessors[0].max[1] = 1.0f; accessors[0].max[2] = 0.0f;

    accessors[1].buffer_view = &views[1];
    accessors[1].component_type = cgltf_component_type_r_32f;
    accessors[1].type = cgltf_type_vec2;
    accessors[1].count = 4;

    accessors[2].buffer_view = &views[2];
    accessors[2].component_type = cgltf_component_type_r_16u;
    accessors[2].type = cgltf_type_scalar;
    accessors[2].count = 6;

    static char tex_uri[] = "quad.png";

    cgltf_image image = {0};
    image.uri = tex_uri;

    cgltf_sampler sampler = {0};
    sampler.mag_filter = cgltf_filter_type_linear;
    sampler.min_filter = cgltf_filter_type_linear;
    sampler.wrap_s = cgltf_wrap_mode_repeat;
    sampler.wrap_t = cgltf_wrap_mode_repeat;

    cgltf_texture texture = {0};
    texture.image = &image;
    texture.sampler = &sampler;

    /* PBR + baseColorTexture (checkerboard PNG) */
    cgltf_material material = {0};
    material.has_pbr_metallic_roughness = 1;
    material.pbr_metallic_roughness.base_color_texture.texture = &texture;
    material.pbr_metallic_roughness.base_color_texture.texcoord = 0;
    material.pbr_metallic_roughness.base_color_factor[0] = 1.0f;
    material.pbr_metallic_roughness.base_color_factor[1] = 1.0f;
    material.pbr_metallic_roughness.base_color_factor[2] = 1.0f;
    material.pbr_metallic_roughness.base_color_factor[3] = 1.0f;
    material.pbr_metallic_roughness.metallic_factor = 0.0f;
    material.pbr_metallic_roughness.roughness_factor = 1.0f;

    /* Mesh primitive */
    cgltf_attribute attributes[2] = {0};
    attributes[0].name = "POSITION";
    attributes[0].type = cgltf_attribute_type_position;
    attributes[0].data = &accessors[0];
    attributes[1].name = "TEXCOORD_0";
    attributes[1].type = cgltf_attribute_type_texcoord;
    attributes[1].data = &accessors[1];

    cgltf_primitive primitive = {0};
    primitive.type = cgltf_primitive_type_triangles;
    primitive.indices = &accessors[2];
    primitive.material = &material;
    primitive.attributes = attributes;
    primitive.attributes_count = 2;

    cgltf_mesh mesh = {0};
    mesh.name = "TileQuad";
    mesh.primitives = &primitive;
    mesh.primitives_count = 1;

    /* Node */
    cgltf_node node = {0};
    node.name = "Tile_0_0";
    node.mesh = &mesh;
    node.has_translation = 0;

    /* Scene */
    cgltf_node *scene_nodes[] = { &node };
    cgltf_scene scene = {0};
    scene.name = "TileMap";
    scene.nodes = scene_nodes;
    scene.nodes_count = 1;

    /* Asset */
    cgltf_asset asset = {0};
    asset.version = "2.0";
    asset.generator = "tiled-gltf-poc";

    /* Root data */
    cgltf_data data = {0};
    data.asset = asset;
    data.meshes = &mesh;
    data.meshes_count = 1;
    data.materials = &material;
    data.materials_count = 1;
    data.images = &image;
    data.images_count = 1;
    data.textures = &texture;
    data.textures_count = 1;
    data.samplers = &sampler;
    data.samplers_count = 1;
    data.accessors = accessors;
    data.accessors_count = 3;
    data.buffer_views = views;
    data.buffer_views_count = 3;
    data.buffers = &buffer;
    data.buffers_count = 1;
    data.nodes = &node;
    data.nodes_count = 1;
    data.scenes = &scene;
    data.scenes_count = 1;
    data.scene = &scene;

    if (!write_png_file("quad.png")) {
        fprintf(stderr, "Failed to write quad.png\n");
        return 1;
    }

    /* Write .bin */
    if (!write_bin_file("quad.bin")) {
        fprintf(stderr, "Failed to write quad.bin\n");
        return 1;
    }

    /* Write .gltf */
    cgltf_options options = {0};
    cgltf_result result = cgltf_write_file(&options, "quad.gltf", &data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to write quad.gltf (error %d)\n", result);
        return 1;
    }

    printf("Written quad.gltf + quad.bin + quad.png\n");
    printf("Open in https://gltf-viewer.donmccurdy.com/ to verify.\n");
    return 0;
}

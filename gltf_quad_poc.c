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

    /* Material (unlit, white) */
    cgltf_material material = {0};
    material.has_pbr_metallic_roughness = 1;
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

    printf("Written quad.gltf + quad.bin\n");
    printf("Open in https://gltf-viewer.donmccurdy.com/ to verify.\n");
    return 0;
}

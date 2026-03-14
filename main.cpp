/*
 * glTF Proof-of-Concept: Generates a textured quad as a valid .gltf file.
 *
 * This demonstrates the core building block for a Tiled map exporter:
 * a single tile rendered as a textured quad with UV coordinates pointing
 * into a tileset atlas.
 *
 * Build:  g++ -std=c++17 -o gltf_poc main.cpp
 * Run:    ./gltf_poc
 * Output: output.gltf + output.bin (validate at https://gltf-viewer.donmccurdy.com/)
 */

#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include "cgltf_write.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

static bool write_binary_buffer(const char *path,
                                const void *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    return written == size;
}

/*
 * Simulate reading a 2x2 tile region from a tileset atlas.
 *
 * Atlas layout (4 tiles in a 2x2 grid, 32x32 each, no margin/spacing):
 *   +----+----+
 *   | T0 | T1 |
 *   +----+----+
 *   | T2 | T3 |
 *   +----+----+
 *
 * We'll place tiles T0, T1, T2, T3 in a 2x2 map grid,
 * demonstrating UV coordinate computation from tile IDs.
 */

struct TileQuad {
    float positions[4][3]; // 4 vertices, xyz
    float uvs[4][2];       // 4 vertices, uv
    unsigned short indices[6];
};

static TileQuad make_tile_quad(int col, int row,
                               float tile_w, float tile_h,
                               int tile_id,
                               int atlas_cols, int atlas_rows,
                               float atlas_w, float atlas_h,
                               int margin, int spacing)
{
    TileQuad q;

    // Screen position (orthogonal: col * tileWidth, row * tileHeight)
    float x0 = col * tile_w;
    float y0 = row * tile_h;
    float x1 = x0 + tile_w;
    float y1 = y0 + tile_h;

    // glTF uses Y-up, so flip the Y axis
    q.positions[0][0] = x0; q.positions[0][1] = -y0; q.positions[0][2] = 0;
    q.positions[1][0] = x1; q.positions[1][1] = -y0; q.positions[1][2] = 0;
    q.positions[2][0] = x1; q.positions[2][1] = -y1; q.positions[2][2] = 0;
    q.positions[3][0] = x0; q.positions[3][1] = -y1; q.positions[3][2] = 0;

    // UV coordinates from tileset atlas
    int tcol = tile_id % atlas_cols;
    int trow = tile_id / atlas_cols;

    float u0 = (float)(margin + tcol * ((int)tile_w + spacing)) / atlas_w;
    float v0 = (float)(margin + trow * ((int)tile_h + spacing)) / atlas_h;
    float u1 = u0 + tile_w / atlas_w;
    float v1 = v0 + tile_h / atlas_h;

    q.uvs[0][0] = u0; q.uvs[0][1] = v0;
    q.uvs[1][0] = u1; q.uvs[1][1] = v0;
    q.uvs[2][0] = u1; q.uvs[2][1] = v1;
    q.uvs[3][0] = u0; q.uvs[3][1] = v1;

    q.indices[0] = 0; q.indices[1] = 1; q.indices[2] = 2;
    q.indices[3] = 0; q.indices[4] = 2; q.indices[5] = 3;

    return q;
}

int main()
{
    // --- Map parameters (simulating a 2x2 orthogonal Tiled map) ---
    const int map_cols = 2, map_rows = 2;
    const float tile_w = 32.0f, tile_h = 32.0f;
    const int atlas_cols = 2, atlas_rows = 2;
    const float atlas_w = 64.0f, atlas_h = 64.0f;
    const int margin = 0, spacing = 0;

    // Tile IDs for each cell: T0, T1, T2, T3
    int tile_ids[map_rows][map_cols] = {
        {0, 1},
        {2, 3}
    };

    // --- Generate geometry for all tiles ---
    const int total_tiles = map_cols * map_rows;
    const int verts_per_tile = 4;
    const int indices_per_tile = 6;
    const int total_verts = total_tiles * verts_per_tile;
    const int total_indices = total_tiles * indices_per_tile;

    std::vector<float> positions(total_verts * 3);
    std::vector<float> texcoords(total_verts * 2);
    std::vector<unsigned short> indices(total_indices);

    float min_pos[3] = { 1e9f,  1e9f, 0.0f};
    float max_pos[3] = {-1e9f, -1e9f, 0.0f};

    int vi = 0; // vertex index offset
    int ii = 0; // index offset

    for (int row = 0; row < map_rows; row++) {
        for (int col = 0; col < map_cols; col++) {
            TileQuad q = make_tile_quad(col, row, tile_w, tile_h,
                                        tile_ids[row][col],
                                        atlas_cols, atlas_rows,
                                        atlas_w, atlas_h,
                                        margin, spacing);

            for (int v = 0; v < 4; v++) {
                positions[(vi + v) * 3 + 0] = q.positions[v][0];
                positions[(vi + v) * 3 + 1] = q.positions[v][1];
                positions[(vi + v) * 3 + 2] = q.positions[v][2];

                texcoords[(vi + v) * 2 + 0] = q.uvs[v][0];
                texcoords[(vi + v) * 2 + 1] = q.uvs[v][1];

                for (int a = 0; a < 3; a++) {
                    if (q.positions[v][a] < min_pos[a]) min_pos[a] = q.positions[v][a];
                    if (q.positions[v][a] > max_pos[a]) max_pos[a] = q.positions[v][a];
                }
            }

            for (int i = 0; i < 6; i++) {
                indices[ii + i] = (unsigned short)(vi + q.indices[i]);
            }

            vi += 4;
            ii += 6;
        }
    }

    // --- Pack binary buffer: [indices | positions | texcoords] ---
    size_t idx_size = total_indices * sizeof(unsigned short);
    size_t pos_size = total_verts * 3 * sizeof(float);
    size_t uv_size  = total_verts * 2 * sizeof(float);

    // Align position data to 4 bytes
    size_t idx_padded = (idx_size + 3) & ~(size_t)3;
    size_t total_buf  = idx_padded + pos_size + uv_size;

    std::vector<unsigned char> buffer(total_buf, 0);
    memcpy(buffer.data(), indices.data(), idx_size);
    memcpy(buffer.data() + idx_padded, positions.data(), pos_size);
    memcpy(buffer.data() + idx_padded + pos_size, texcoords.data(), uv_size);

    if (!write_binary_buffer("output.bin", buffer.data(), total_buf)) {
        fprintf(stderr, "Failed to write output.bin\n");
        return 1;
    }

    // --- Build cgltf data structure ---
    cgltf_data data = {};
    data.asset.generator = (char*)"gltf-poc (Tiled GSoC proof-of-concept)";
    data.asset.version = (char*)"2.0";

    // Buffer
    cgltf_buffer buf = {};
    buf.uri = (char*)"output.bin";
    buf.size = total_buf;
    data.buffers = &buf;
    data.buffers_count = 1;

    // Buffer views: [indices, positions, texcoords]
    cgltf_buffer_view views[3] = {};

    views[0].buffer = &buf;
    views[0].offset = 0;
    views[0].size = idx_size;
    views[0].type = cgltf_buffer_view_type_indices;

    views[1].buffer = &buf;
    views[1].offset = idx_padded;
    views[1].size = pos_size;
    views[1].stride = 3 * sizeof(float);
    views[1].type = cgltf_buffer_view_type_vertices;

    views[2].buffer = &buf;
    views[2].offset = idx_padded + pos_size;
    views[2].size = uv_size;
    views[2].stride = 2 * sizeof(float);
    views[2].type = cgltf_buffer_view_type_vertices;

    data.buffer_views = views;
    data.buffer_views_count = 3;

    // Accessors: [indices, positions, texcoords]
    cgltf_accessor accessors[3] = {};

    accessors[0].buffer_view = &views[0];
    accessors[0].offset = 0;
    accessors[0].component_type = cgltf_component_type_r_16u;
    accessors[0].type = cgltf_type_scalar;
    accessors[0].count = total_indices;

    accessors[1].buffer_view = &views[1];
    accessors[1].offset = 0;
    accessors[1].component_type = cgltf_component_type_r_32f;
    accessors[1].type = cgltf_type_vec3;
    accessors[1].count = total_verts;
    accessors[1].has_min = true;
    accessors[1].has_max = true;
    memcpy(accessors[1].min, min_pos, sizeof(min_pos));
    memcpy(accessors[1].max, max_pos, sizeof(max_pos));

    accessors[2].buffer_view = &views[2];
    accessors[2].offset = 0;
    accessors[2].component_type = cgltf_component_type_r_32f;
    accessors[2].type = cgltf_type_vec2;
    accessors[2].count = total_verts;

    data.accessors = accessors;
    data.accessors_count = 3;

    // Image (reference to external PNG)
    cgltf_image image = {};
    image.uri = (char*)"tileset.png";
    data.images = &image;
    data.images_count = 1;

    // Sampler (nearest-neighbor for pixel art)
    cgltf_sampler sampler = {};
    sampler.mag_filter = cgltf_filter_type_nearest;
    sampler.min_filter = cgltf_filter_type_nearest;
    sampler.wrap_s = cgltf_wrap_mode_clamp_to_edge;
    sampler.wrap_t = cgltf_wrap_mode_clamp_to_edge;
    data.samplers = &sampler;
    data.samplers_count = 1;

    // Texture
    cgltf_texture texture = {};
    texture.image = &image;
    texture.sampler = &sampler;
    data.textures = &texture;
    data.textures_count = 1;

    // Material (unlit with alpha masking for tile transparency)
    cgltf_material material = {};
    material.has_pbr_metallic_roughness = true;
    material.pbr_metallic_roughness.base_color_factor[0] = 1.0f;
    material.pbr_metallic_roughness.base_color_factor[1] = 1.0f;
    material.pbr_metallic_roughness.base_color_factor[2] = 1.0f;
    material.pbr_metallic_roughness.base_color_factor[3] = 1.0f;
    material.pbr_metallic_roughness.base_color_texture.texture = &texture;
    material.pbr_metallic_roughness.metallic_factor = 0.0f;
    material.pbr_metallic_roughness.roughness_factor = 1.0f;
    material.alpha_mode = cgltf_alpha_mode_mask;
    material.alpha_cutoff = 0.5f;
    material.unlit = true;
    data.materials = &material;
    data.materials_count = 1;

    // Mesh primitive
    cgltf_attribute attrs[2] = {};
    attrs[0].name = (char*)"POSITION";
    attrs[0].type = cgltf_attribute_type_position;
    attrs[0].data = &accessors[1];
    attrs[1].name = (char*)"TEXCOORD_0";
    attrs[1].type = cgltf_attribute_type_texcoord;
    attrs[1].data = &accessors[2];

    cgltf_primitive primitive = {};
    primitive.type = cgltf_primitive_type_triangles;
    primitive.indices = &accessors[0];
    primitive.attributes = attrs;
    primitive.attributes_count = 2;
    primitive.material = &material;

    cgltf_mesh mesh = {};
    mesh.name = (char*)"TileLayer_Ground";
    mesh.primitives = &primitive;
    mesh.primitives_count = 1;
    data.meshes = &mesh;
    data.meshes_count = 1;

    // Node
    cgltf_node node = {};
    node.name = (char*)"Ground";
    node.mesh = &mesh;
    data.nodes = &node;
    data.nodes_count = 1;

    // Scene
    cgltf_node *scene_nodes[] = { &node };
    cgltf_scene scene = {};
    scene.name = (char*)"TiledMap";
    scene.nodes = scene_nodes;
    scene.nodes_count = 1;
    data.scenes = &scene;
    data.scenes_count = 1;
    data.scene = &scene;

    // --- Write .gltf ---
    cgltf_options options = {};
    cgltf_result result = cgltf_write_file(&options, "output.gltf", &data);
    if (result != cgltf_result_success) {
        fprintf(stderr, "Failed to write output.gltf (error %d)\n", result);
        return 1;
    }

    printf("Success! Generated:\n");
    printf("  output.gltf  (scene description)\n");
    printf("  output.bin   (binary geometry data, %zu bytes)\n", total_buf);
    printf("\nMap: %dx%d tiles, tile size: %.0fx%.0f\n",
           map_cols, map_rows, tile_w, tile_h);
    printf("Atlas: %.0fx%.0f, %dx%d tiles\n",
           atlas_w, atlas_h, atlas_cols, atlas_rows);
    printf("Geometry: %d vertices, %d triangles\n",
           total_verts, total_indices / 3);
    printf("\nValidate at: https://gltf-viewer.donmccurdy.com/\n");
    printf("  (you'll need a tileset.png image, 64x64, with 4 tiles in a 2x2 grid)\n");

    return 0;
}

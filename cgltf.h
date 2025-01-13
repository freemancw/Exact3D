/*
 * cgltf.h - C glTF loader
 * Version 1.9
 * https://github.com/jkuhlmann/cgltf
 *
 * Distributed under the MIT License, see notice at the end of this file.
 */

#ifndef CGLTF_H_INCLUDED__
#define CGLTF_H_INCLUDED__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cgltf_result {
	cgltf_result_success,
	cgltf_result_data_too_short,
	cgltf_result_unknown_format,
	cgltf_result_invalid_json,
	cgltf_result_invalid_gltf,
	cgltf_result_invalid_options,
	cgltf_result_file_not_found,
	cgltf_result_io_error,
	cgltf_result_out_of_memory,
	cgltf_result_legacy_gltf,
	cgltf_result_data_too_long
} cgltf_result;

typedef enum cgltf_file_type {
	cgltf_file_type_invalid,
	cgltf_file_type_gltf,
	cgltf_file_type_glb
} cgltf_file_type;

typedef enum cgltf_buffer_view_type {
	cgltf_buffer_view_type_invalid,
	cgltf_buffer_view_type_indices,
	cgltf_buffer_view_type_vertices
} cgltf_buffer_view_type;

typedef enum cgltf_attribute_type {
	cgltf_attribute_type_invalid,
	cgltf_attribute_type_position,
	cgltf_attribute_type_normal,
	cgltf_attribute_type_tangent,
	cgltf_attribute_type_texcoord,
	cgltf_attribute_type_color,
	cgltf_attribute_type_joints,
	cgltf_attribute_type_weights
} cgltf_attribute_type;

typedef enum cgltf_component_type {
	cgltf_component_type_invalid,
	cgltf_component_type_r_8,
	cgltf_component_type_r_8u,
	cgltf_component_type_r_16,
	cgltf_component_type_r_16u,
	cgltf_component_type_r_32u,
	cgltf_component_type_r_32f
} cgltf_component_type;

typedef enum cgltf_type {
	cgltf_type_invalid,
	cgltf_type_scalar,
	cgltf_type_vec2,
	cgltf_type_vec3,
	cgltf_type_vec4,
	cgltf_type_mat2,
	cgltf_type_mat3,
	cgltf_type_mat4
} cgltf_type;

typedef enum cgltf_primitive_type {
	cgltf_primitive_type_points,
	cgltf_primitive_type_lines,
	cgltf_primitive_type_line_loop,
	cgltf_primitive_type_line_strip,
	cgltf_primitive_type_triangles,
	cgltf_primitive_type_triangle_strip,
	cgltf_primitive_type_triangle_fan
} cgltf_primitive_type;

typedef enum cgltf_alpha_mode {
	cgltf_alpha_mode_opaque,
	cgltf_alpha_mode_mask,
	cgltf_alpha_mode_blend
} cgltf_alpha_mode;

typedef enum cgltf_animation_path_type {
	cgltf_animation_path_type_invalid,
	cgltf_animation_path_type_translation,
	cgltf_animation_path_type_rotation,
	cgltf_animation_path_type_scale,
	cgltf_animation_path_type_weights
} cgltf_animation_path_type;

typedef enum cgltf_interpolation_type {
	cgltf_interpolation_type_linear,
	cgltf_interpolation_type_step,
	cgltf_interpolation_type_cubic_spline
} cgltf_interpolation_type;

typedef enum cgltf_camera_type {
	cgltf_camera_type_invalid,
	cgltf_camera_type_perspective,
	cgltf_camera_type_orthographic
} cgltf_camera_type;

typedef enum cgltf_light_type {
	cgltf_light_type_invalid,
	cgltf_light_type_directional,
	cgltf_light_type_point,
	cgltf_light_type_spot
} cgltf_light_type;

typedef enum cgltf_data_free_method {
	cgltf_data_free_method_none,
	cgltf_data_free_method_file_release,
	cgltf_data_free_method_buffer_release
} cgltf_data_free_method;

typedef struct cgltf_options {
	cgltf_data_free_method data_free_method;
	void* (*file_read)(const struct cgltf_options* options, const char* path, size_t* size);
	void (*file_release)(const struct cgltf_options* options, void* data);
	void* (*buffer_read)(const struct cgltf_options* options, const struct cgltf_buffer* buffer, size_t* size);
	void (*buffer_release)(const struct cgltf_options* options, void* data);
	void* user_data;
} cgltf_options;

typedef struct cgltf_extras {
	void* data;
} cgltf_extras;

typedef struct cgltf_buffer {
	cgltf_size size;
	void* data;
	cgltf_extras extras;
	char* uri;
} cgltf_buffer;

typedef struct cgltf_buffer_view {
	cgltf_size offset;
	cgltf_size size;
	cgltf_size stride;
	cgltf_buffer* buffer;
	cgltf_buffer_view_type type;
	cgltf_extras extras;
} cgltf_buffer_view;

typedef struct cgltf_accessor {
	cgltf_size offset;
	cgltf_size count;
	cgltf_size stride;
	cgltf_buffer_view* buffer_view;
	cgltf_component_type component_type;
	cgltf_type type;
	cgltf_bool normalized;
	cgltf_extras extras;
} cgltf_accessor;

typedef struct cgltf_attribute {
	cgltf_attribute_type type;
	cgltf_accessor* data;
	cgltf_size index;
	cgltf_extras extras;
} cgltf_attribute;

typedef struct cgltf_primitive {
	cgltf_primitive_type type;
	cgltf_accessor* indices;
	cgltf_material* material;
	cgltf_size attributes_count;
	cgltf_attribute* attributes;
	cgltf_size targets_count;
	cgltf_morph_target* targets;
	cgltf_extras extras;
} cgltf_primitive;

typedef struct cgltf_mesh {
	cgltf_size primitives_count;
	cgltf_primitive* primitives;
	cgltf_size weights_count;
	float* weights;
	cgltf_extras extras;
} cgltf_mesh;

typedef struct cgltf_node {
	cgltf_size children_count;
	struct cgltf_node** children;
	cgltf_mesh* mesh;
	cgltf_skin* skin;
	cgltf_camera* camera;
	cgltf_light* light;
	cgltf_size weights_count;
	float* weights;
	cgltf_bool has_translation;
	float translation[3];
	cgltf_bool has_rotation;
	float rotation[4];
	cgltf_bool has_scale;
	float scale[3];
	cgltf_bool has_matrix;
	float matrix[16];
	cgltf_extras extras;
} cgltf_node;

typedef struct cgltf_scene {
	cgltf_size nodes_count;
	cgltf_node** nodes;
	cgltf_extras extras;
} cgltf_scene;

typedef struct cgltf_data {
	cgltf_file_type file_type;
	cgltf_size buffers_count;
	cgltf_buffer* buffers;
	cgltf_size buffer_views_count;
	cgltf_buffer_view* buffer_views;
	cgltf_size accessors_count;
	cgltf_accessor* accessors;
	cgltf_size images_count;
	cgltf_image* images;
	cgltf_size textures_count;
	cgltf_texture* textures;
	cgltf_size materials_count;
	cgltf_material* materials;
	cgltf_size meshes_count;
	cgltf_mesh* meshes;
	cgltf_size nodes_count;
	cgltf_node* nodes;
	cgltf_size scenes_count;
	cgltf_scene* scenes;
	cgltf_scene* scene;
	cgltf_extras extras;
} cgltf_data;

cgltf_result cgltf_parse(const cgltf_options* options, const void* data, cgltf_size size, cgltf_data** out_data);
cgltf_result cgltf_parse_file(const cgltf_options* options, const char* path, cgltf_data** out_data);
cgltf_result cgltf_load_buffers(const cgltf_options* options, cgltf_data* data, const char* gltf_path);
void cgltf_free(cgltf_data* data);

#ifdef __cplusplus
}
#endif

#endif /* CGLTF_H_INCLUDED__ */

/*
 * MIT License
 * 
 * Copyright (c) 2018-2021 Johannes Kuhlmann
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

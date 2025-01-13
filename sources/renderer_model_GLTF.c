#include "cgltf.h"
#include "headers/common.h"
#include "headers/renderer_models.h"
#include "headers/renderer_materials.h"

typedef struct {
    cgltf_data* data;
    GLuint* vbo;
    GLuint* vao;
    int numMeshes;
} gltf_model_t;

static gltf_model_t modelList[MAX_MODELS];
static modelHandle_t lastPtr = 0;

modelHandle_t renderer_model_loadGLTF(char *name, eboolean collidable, eboolean clamp) {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, name, &data);

    if (result != cgltf_result_success) {
        printf("Loading GLTF: %s, failed. Error parsing file.\n", name);
        return -1;
    }

    result = cgltf_load_buffers(&options, data, name);
    if (result != cgltf_result_success) {
        printf("Loading GLTF: %s, failed. Error loading buffers.\n", name);
        cgltf_free(data);
        return -1;
    }

    gltf_model_t* model = &modelList[lastPtr];
    model->data = data;
    model->numMeshes = data->meshes_count;
    model->vbo = (GLuint*)malloc(sizeof(GLuint) * model->numMeshes);
    model->vao = (GLuint*)malloc(sizeof(GLuint) * model->numMeshes);

    glGenBuffers(model->numMeshes, model->vbo);
    glGenVertexArrays(model->numMeshes, model->vao);

    for (int i = 0; i < model->numMeshes; ++i) {
        cgltf_mesh* mesh = &data->meshes[i];
        cgltf_primitive* primitive = &mesh->primitives[0];

        glBindVertexArray(model->vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, model->vbo[i]);

        cgltf_accessor* accessor = primitive->attributes[0].data;
        glBufferData(GL_ARRAY_BUFFER, accessor->buffer_view->size, accessor->buffer_view->buffer->data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, accessor->type, accessor->component_type, GL_FALSE, accessor->stride, (void*)accessor->offset);

        glBindVertexArray(0);
    }

    return lastPtr++;
}

void renderer_model_drawGLTF(modelHandle_t index) {
    gltf_model_t* model = &modelList[index];

    for (int i = 0; i < model->numMeshes; ++i) {
        glBindVertexArray(model->vao[i]);
        glDrawArrays(GL_TRIANGLES, 0, model->data->meshes[i].primitives[0].attributes[0].data->count);
        glBindVertexArray(0);
    }
}

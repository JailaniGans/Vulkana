#version 450

/*
 * scene.vert - Vertex shader untuk segitiga demo.
 * Input:
 *   location 0: posisi (vec3)
 *   location 1: warna   (vec3)
 * Uniform:
 *   binding 0: UBO (model, view, proj)
 * Output:
 *   location 0: warna terinterpolasi ke fragment shader
 */

layout(location = 0) in vec3 inPosisi;
layout(location = 1) in vec3 inWarna;

layout(location = 0) out vec3 outWarna;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosisi, 1.0);
    outWarna = inWarna;
}

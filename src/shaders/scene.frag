#version 450

/*
 * scene.frag - Fragment shader untuk segitiga demo.
 * Input:
 *   location 0: warna terinterpolasi dari vertex shader
 * Output:
 *   location 0: warna final ke framebuffer
 */

layout(location = 0) in vec3 inWarna;

layout(location = 0) out vec4 outWarna;

void main() {
    outWarna = vec4(inWarna, 1.0);
}

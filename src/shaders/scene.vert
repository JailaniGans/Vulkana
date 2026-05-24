#version 450

// Vertex shader - terima posisi & warna, output ke fragment
// MVP dikirim via push constant ( sizeof(mat4) * 3 = 144 byte )

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConst {
    mat4 model;
    mat4 view;
    mat4 proj;
} push;

void main()
{
    gl_Position = push.proj * push.view * push.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}

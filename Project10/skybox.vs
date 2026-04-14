#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords; // Direction vector used to sample the cubemap

uniform mat4 projection;
uniform mat4 view;

void main() {
    TexCoords = aPos; // Position used directly as cubemap direction
    // Strip translation by converting view to mat3 then back to mat4
    // This keeps the skybox centered on the camera at all times
    vec4 pos = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
    // Set z = w so the depth is always 1.0 (skybox renders behind everything)
    gl_Position = pos.xyww;
}
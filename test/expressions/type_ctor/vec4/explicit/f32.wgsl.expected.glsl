#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
const vec4 v = vec4(0.0f, 1.0f, 2.0f, 3.0f);
[[group(1), binding(0)]] var arg_0 : texture_storage_1d<r32uint, write>;

fn textureStore_102722() {
  textureStore(arg_0, 1, vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_102722();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_102722();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_102722();
}
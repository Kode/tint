[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

fn textureNumLevels_105988() {
  var res : i32 = textureNumLevels(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumLevels_105988();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumLevels_105988();
}

[[stage(compute)]]
fn compute_main() {
  textureNumLevels_105988();
}
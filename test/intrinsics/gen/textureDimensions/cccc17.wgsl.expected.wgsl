[[group(1), binding(0)]] var arg_0 : texture_cube<i32>;

fn textureDimensions_cccc17() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_cccc17();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_cccc17();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_cccc17();
}
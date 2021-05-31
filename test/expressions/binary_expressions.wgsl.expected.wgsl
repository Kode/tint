fn vector_scalar_f32() {
  var v : vec3<f32>;
  var s : f32;
  var r : vec3<f32>;
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
}

fn vector_scalar_i32() {
  var v : vec3<i32>;
  var s : i32;
  var r : vec3<i32>;
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

fn vector_scalar_u32() {
  var v : vec3<u32>;
  var s : u32;
  var r : vec3<u32>;
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

fn scalar_vector_f32() {
  var v : vec3<f32>;
  var s : f32;
  var r : vec3<f32>;
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
}

fn scalar_vector_i32() {
  var v : vec3<i32>;
  var s : i32;
  var r : vec3<i32>;
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

fn scalar_vector_u32() {
  var v : vec3<u32>;
  var s : u32;
  var r : vec3<u32>;
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}
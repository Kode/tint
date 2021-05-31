struct tint_symbol {
  float4 value : SV_Target0;
};

void vector_scalar_f32() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  float s = 0.0f;
  float3 r = float3(0.0f, 0.0f, 0.0f);
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
}

void vector_scalar_i32() {
  int3 v = int3(0, 0, 0);
  int s = 0;
  int3 r = int3(0, 0, 0);
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

void vector_scalar_u32() {
  uint3 v = uint3(0u, 0u, 0u);
  uint s = 0u;
  uint3 r = uint3(0u, 0u, 0u);
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

void scalar_vector_f32() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  float s = 0.0f;
  float3 r = float3(0.0f, 0.0f, 0.0f);
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
}

void scalar_vector_i32() {
  int3 v = int3(0, 0, 0);
  int s = 0;
  int3 r = int3(0, 0, 0);
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

void scalar_vector_u32() {
  uint3 v = uint3(0u, 0u, 0u);
  uint s = 0u;
  uint3 r = uint3(0u, 0u, 0u);
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

tint_symbol main() {
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

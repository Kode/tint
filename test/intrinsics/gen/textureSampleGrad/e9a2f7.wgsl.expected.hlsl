struct tint_symbol {
  float4 value : SV_Position;
};

Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleGrad_e9a2f7() {
  float4 res = arg_0.SampleGrad(arg_1, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), int3(0, 0, 0));
}

tint_symbol vertex_main() {
  textureSampleGrad_e9a2f7();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureSampleGrad_e9a2f7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_e9a2f7();
  return;
}

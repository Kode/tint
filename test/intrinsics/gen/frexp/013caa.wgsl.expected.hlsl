struct tint_symbol {
  float4 value : SV_Position;
};

void frexp_013caa() {
  int4 arg_1 = int4(0, 0, 0, 0);
  float4 tint_tmp;
  float4 tint_tmp_1 = frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), tint_tmp);
  arg_1 = int4(tint_tmp);
  float4 res = tint_tmp_1;
}

tint_symbol vertex_main() {
  frexp_013caa();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_013caa();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_013caa();
  return;
}

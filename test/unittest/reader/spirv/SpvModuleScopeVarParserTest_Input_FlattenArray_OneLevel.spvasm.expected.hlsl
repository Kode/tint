static float x_1[3] = (float[3])0;
static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  return;
}

struct main_out {
  float4 x_2_1;
};
struct tint_symbol_1 {
  float x_1_param : TEXCOORD4;
  float x_1_param_1 : TEXCOORD5;
  float x_1_param_2 : TEXCOORD6;
};
struct tint_symbol_2 {
  float4 x_2_1 : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float x_1_param = tint_symbol.x_1_param;
  const float x_1_param_1 = tint_symbol.x_1_param_1;
  const float x_1_param_2 = tint_symbol.x_1_param_2;
  x_1[0] = x_1_param;
  x_1[1] = x_1_param_1;
  x_1[2] = x_1_param_2;
  main_1();
  const main_out tint_symbol_3 = {x_2};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_2_1};
  return tint_symbol_4;
}
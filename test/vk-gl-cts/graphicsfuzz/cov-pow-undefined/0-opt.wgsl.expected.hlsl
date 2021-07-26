static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[2];
};

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  a = -1.0f;
  b = 1.700000048f;
  c = pow(a, b);
  const float x_30 = c;
  x_GLF_color = float4(x_30, x_30, x_30, x_30);
  bool tint_tmp = (a == -1.0f);
  if (tint_tmp) {
    tint_tmp = (b == 1.700000048f);
  }
  if ((tint_tmp)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_41 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
    const float x_43 = asfloat(x_8[1].x);
    const float x_45 = asfloat(x_8[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_47 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(x_41, x_43, x_45, x_47);
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_50 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(x_50, x_50, x_50, x_50);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
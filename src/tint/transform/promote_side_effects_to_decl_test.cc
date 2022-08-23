// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/transform/promote_side_effects_to_decl.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using PromoteSideEffectsToDeclTest = TransformTest;

TEST_F(PromoteSideEffectsToDeclTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Unary_Arith_SE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  let r = -(a(0));
}
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_BothSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn b() -> i32 {
  return 1;
}

fn f() {
  let r = a() + b();
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn b() -> i32 {
  return 1;
}

fn f() {
  let tint_symbol = a();
  let tint_symbol_1 = b();
  let r = (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_LeftSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let r = a() + b;
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol = a();
  let r = (tint_symbol + b);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_RightSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let r = b + a();
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol = b;
  let tint_symbol_1 = a();
  let r = (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_LeftmostSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  var d = 1;
  let r = a() + b + c + d;
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  var d = 1;
  let tint_symbol = a();
  let r = (((tint_symbol + b) + c) + d);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_RightmostSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  var d = 1;
  let r = b + c + d + a();
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  var d = 1;
  let tint_symbol = ((b + c) + d);
  let tint_symbol_1 = a();
  let r = (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_MiddleSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  var d = 1;
  var e = 1;
  let r = b + c + a() + d + e;
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  var d = 1;
  var e = 1;
  let tint_symbol = (b + c);
  let tint_symbol_1 = a();
  let r = (((tint_symbol + tint_symbol_1) + d) + e);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_ThreeSE) {
    auto* src = R"(
fn a(v : i32) -> i32 {
  return v;
}

fn f() {
  let r = a(0) + a(1) + a(2);
}
)";

    auto* expect = R"(
fn a(v : i32) -> i32 {
  return v;
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  let tint_symbol_2 = a(2);
  let r = ((tint_symbol + tint_symbol_1) + tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Constants_NoRecvSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  let r = ((((1 + a(0)) - 2) + 3) - 4);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  let tint_symbol = a(0);
  let r = ((((1 + tint_symbol) - 2) + 3) - 4);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Constants_RecvSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let r = a(0) + 1 + b + a(1) + 2;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = b;
  let tint_symbol_2 = a(1);
  let r = ((((tint_symbol + 1) + tint_symbol_1) + tint_symbol_2) + 2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Constants_ConstAndSEAndVar) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn main() {
  var b = 1;
  var c = 1;
  let r = 1 + a(0) + b;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn main() {
  var b = 1;
  var c = 1;
  let tint_symbol = a(0);
  let r = ((1 + tint_symbol) + b);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Constants_VarAndSEAndConst) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn main() {
  var b = 1;
  let r = b + a(0) + 1;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn main() {
  var b = 1;
  let tint_symbol = b;
  let tint_symbol_1 = a(0);
  let r = ((tint_symbol + tint_symbol_1) + 1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Constants_SEAndVarAndConstAndVar) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn main() {
  var b = 1;
  var c = 1;
  let r = a(0) + b + 1 + c;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn main() {
  var b = 1;
  var c = 1;
  let tint_symbol = a(0);
  let r = (((tint_symbol + b) + 1) + c);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Builtins_WithSE) {
    auto* src = R"(
struct SB {
  a : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

fn f() {
  var b = 0;
  let r = atomicAdd(&sb.a, 123) + b;
}
)";

    auto* expect = R"(
struct SB {
  a : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

fn f() {
  var b = 0;
  let tint_symbol = atomicAdd(&(sb.a), 123);
  let r = (tint_symbol + b);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Builtins_NoSEAndVar) {
    auto* src = R"(
struct SB {
  a : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

fn f() {
  var b = 0;
  let r = (atomicLoad(&(sb.a)) + b);
}
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Builtins_NoSEAndSE) {
    auto* src = R"(
struct SB {
  a : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 0;
  let r = (atomicLoad(&(sb.a)) + a(0));
}
)";

    auto* expect = R"(
struct SB {
  a : atomic<i32>,
}

@group(0) @binding(0) var<storage, read_write> sb : SB;

fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 0;
  let tint_symbol = atomicLoad(&(sb.a));
  let tint_symbol_1 = a(0);
  let r = (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_Vector_RightSE) {
    auto* src = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b : vec3<i32>;
  var c : i32;
  let r = b[c] + a();
}
)";

    auto* expect = R"(
fn a() -> i32 {
  return 1;
}

fn f() {
  var b : vec3<i32>;
  var c : i32;
  let tint_symbol = c;
  let tint_symbol_1 = b[tint_symbol];
  let tint_symbol_2 = a();
  let r = (tint_symbol_1 + tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InCall) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn g(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let r = g(a(0)) - g(b + a(1));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn g(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = g(tint_symbol);
  let tint_symbol_2 = b;
  let tint_symbol_3 = a(1);
  let tint_symbol_4 = g((tint_symbol_2 + tint_symbol_3));
  let r = (tint_symbol_1 - tint_symbol_4);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InTypeCtor) {
    auto* src = R"(

fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let r = i32(a(0)) + i32(a(1) + b) - i32(a(2) - a(3));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = i32(tint_symbol);
  let tint_symbol_2 = a(1);
  let tint_symbol_3 = i32((tint_symbol_2 + b));
  let tint_symbol_4 = a(2);
  let tint_symbol_5 = a(3);
  let tint_symbol_6 = i32((tint_symbol_4 - tint_symbol_5));
  let r = ((tint_symbol_1 + tint_symbol_3) - tint_symbol_6);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InTypeConversion) {
    auto* src = R"(

fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1u;
  let r = u32(a(0)) + u32(a(1)) - b;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1u;
  let tint_symbol = a(0);
  let tint_symbol_1 = u32(tint_symbol);
  let tint_symbol_2 = a(1);
  let tint_symbol_3 = u32(tint_symbol_2);
  let r = ((tint_symbol_1 + tint_symbol_3) - b);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InIntrinsic) {
    auto* src = R"(

fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let r = abs(a(0)) + abs(a(1) + b) - abs(a(2) + a(3));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = abs(tint_symbol);
  let tint_symbol_2 = a(1);
  let tint_symbol_3 = abs((tint_symbol_2 + b));
  let tint_symbol_4 = a(2);
  let tint_symbol_5 = a(3);
  let tint_symbol_6 = abs((tint_symbol_4 + tint_symbol_5));
  let r = ((tint_symbol_1 + tint_symbol_3) - tint_symbol_6);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InMemberAccessor) {
    auto* src = R"(

struct S {
  v : i32,
}

fn a(i : i32) -> S {
  return S();
}

fn f() {
  var b = 1;
  let r = a(0).v + b + a(1).v;
}
)";

    auto* expect = R"(
struct S {
  v : i32,
}

fn a(i : i32) -> S {
  return S();
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = b;
  let tint_symbol_2 = a(1);
  let r = ((tint_symbol.v + tint_symbol_1) + tint_symbol_2.v);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InUnary) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let r = -a(0) + -(b + a(1));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let tint_symbol = -(a(0));
  let tint_symbol_1 = b;
  let tint_symbol_2 = a(1);
  let r = (tint_symbol + -((tint_symbol_1 + tint_symbol_2)));
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InBitcast) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let r = bitcast<u32>(a(0) + a(1));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  let r = bitcast<u32>((tint_symbol + tint_symbol_1));
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InForLoopInit) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  for(var r = a(0) + b; ; ) {
    var marker = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  for(var r = (tint_symbol + b); ; ) {
    var marker = 0;
    break;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InForLoopCond) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  for(; a(0) + b > 0;) {
    var marker = 0;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  loop {
    let tint_symbol = a(0);
    if (!(((tint_symbol + b) > 0))) {
      break;
    }
    {
      var marker = 0;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InForLoopCont) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  var r = 0;
  for(; ; r = a(0) + b) {
    var marker = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  var r = 0;
  loop {
    {
      var marker = 0;
      break;
    }

    continuing {
      let tint_symbol = a(0);
      r = (tint_symbol + b);
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InForLoopInitCondCont) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  var c = 2;
  var d = 3;
  var r = 0;
  for(var r = a(0) + b; a(1) + c > 0; r = a(2) + d) {
    var marker = 0;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  var c = 2;
  var d = 3;
  var r = 0;
  {
    let tint_symbol = a(0);
    var r = (tint_symbol + b);
    loop {
      let tint_symbol_1 = a(1);
      if (!(((tint_symbol_1 + c) > 0))) {
        break;
      }
      {
        var marker = 0;
      }

      continuing {
        let tint_symbol_2 = a(2);
        r = (tint_symbol_2 + d);
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InWhileCond) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  while(a(0) + b > 0) {
    var marker = 0;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  loop {
    let tint_symbol = a(0);
    if (!(((tint_symbol + b) > 0))) {
      break;
    }
    {
      var marker = 0;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InElseIf) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  if (true) {
    var marker = 0;
  } else if (a(0) + b > 0) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  if (true) {
    var marker = 0;
  } else {
    let tint_symbol = a(0);
    if (((tint_symbol + b) > 0)) {
      var marker = 1;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InElseIfChain) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (a(0) + b > 0) {
    var marker = 2;
  } else if (a(1) + a(2) > 0) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    let tint_symbol = a(0);
    if (((tint_symbol + b) > 0)) {
      var marker = 2;
    } else {
      let tint_symbol_1 = a(1);
      let tint_symbol_2 = a(2);
      if (((tint_symbol_1 + tint_symbol_2) > 0)) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InReturn) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() -> i32 {
  var b = 1;
  return b + a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() -> i32 {
  var b = 1;
  let tint_symbol = b;
  let tint_symbol_1 = a(0);
  return (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Arith_InSwitch) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  switch (b + a(0)) {
    default: {
    }
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var b = 1;
  let tint_symbol = b;
  let tint_symbol_1 = a(0);
  switch((tint_symbol + tint_symbol_1)) {
    default: {
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_LeftSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = a(0) && b;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol = a(0);
  if (tint_symbol) {
    tint_symbol = b;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_RightSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = b && a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol = b;
  if (tint_symbol) {
    tint_symbol = a(0);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_BothSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  let r = a(0) && a(1);
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var tint_symbol = a(0);
  if (tint_symbol) {
    tint_symbol = a(1);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_LeftmostSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  let r = a(0) && b && c && d;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  var tint_symbol_2 = a(0);
  if (tint_symbol_2) {
    tint_symbol_2 = b;
  }
  var tint_symbol_1 = tint_symbol_2;
  if (tint_symbol_1) {
    tint_symbol_1 = c;
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = d;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_RightmostSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  let r = b && c && d && a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  var tint_symbol = ((b && c) && d);
  if (tint_symbol) {
    tint_symbol = a(0);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_MiddleSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  let r = b && c && a(0) && c && d;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  var tint_symbol_2 = (b && c);
  if (tint_symbol_2) {
    tint_symbol_2 = a(0);
  }
  var tint_symbol_1 = tint_symbol_2;
  if (tint_symbol_1) {
    tint_symbol_1 = c;
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = d;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_Constants_NoRecvSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  let r = true && a(0) && false && a(1);
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var tint_symbol_2 = true;
  if (tint_symbol_2) {
    tint_symbol_2 = a(0);
  }
  var tint_symbol_1 = tint_symbol_2;
  if (tint_symbol_1) {
    tint_symbol_1 = false;
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = a(1);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_Constants_RecvSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = b && true && a(0) && false && a(1);
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol_2 = (b && true);
  if (tint_symbol_2) {
    tint_symbol_2 = a(0);
  }
  var tint_symbol_1 = tint_symbol_2;
  if (tint_symbol_1) {
    tint_symbol_1 = false;
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = a(1);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_Constants_ConstAndSEAndVar) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn main() {
  var b = true;
  var c = true;
  let r = true && a(0) && b;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn main() {
  var b = true;
  var c = true;
  var tint_symbol_1 = true;
  if (tint_symbol_1) {
    tint_symbol_1 = a(0);
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = b;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_Constants_VarAndSEAndConst) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn main() {
  var b = true;
  let r = b && a(0) && true;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn main() {
  var b = true;
  var tint_symbol_1 = b;
  if (tint_symbol_1) {
    tint_symbol_1 = a(0);
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = true;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_Constants_SEAndVarAndConstAndVar) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn main() {
  var b = true;
  var c = true;
  let r = a(0) && b && true && c;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn main() {
  var b = true;
  var c = true;
  var tint_symbol_2 = a(0);
  if (tint_symbol_2) {
    tint_symbol_2 = b;
  }
  var tint_symbol_1 = tint_symbol_2;
  if (tint_symbol_1) {
    tint_symbol_1 = true;
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    tint_symbol = c;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_MixedSE) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  let r = (b && a(0)) || (c && a(1) && c && d) || a(2);
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  var tint_symbol_2 = b;
  if (tint_symbol_2) {
    tint_symbol_2 = a(0);
  }
  var tint_symbol_1 = tint_symbol_2;
  if (!(tint_symbol_1)) {
    var tint_symbol_5 = c;
    if (tint_symbol_5) {
      tint_symbol_5 = a(1);
    }
    var tint_symbol_4 = tint_symbol_5;
    if (tint_symbol_4) {
      tint_symbol_4 = c;
    }
    var tint_symbol_3 = tint_symbol_4;
    if (tint_symbol_3) {
      tint_symbol_3 = d;
    }
    tint_symbol_1 = tint_symbol_3;
  }
  var tint_symbol = tint_symbol_1;
  if (!(tint_symbol)) {
    tint_symbol = a(2);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_NestedAnds) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  let r = a(0) && (a(1) && (a(2) && (a(3) && (a(4) && a(5)))));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var tint_symbol = a(0);
  if (tint_symbol) {
    var tint_symbol_1 = a(1);
    if (tint_symbol_1) {
      var tint_symbol_2 = a(2);
      if (tint_symbol_2) {
        var tint_symbol_3 = a(3);
        if (tint_symbol_3) {
          var tint_symbol_4 = a(4);
          if (tint_symbol_4) {
            tint_symbol_4 = a(5);
          }
          tint_symbol_3 = tint_symbol_4;
        }
        tint_symbol_2 = tint_symbol_3;
      }
      tint_symbol_1 = tint_symbol_2;
    }
    tint_symbol = tint_symbol_1;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_NestedOrs) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  let r = a(0) || (a(1) || (a(2) || (a(3) || (a(4) || a(5)))));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var tint_symbol = a(0);
  if (!(tint_symbol)) {
    var tint_symbol_1 = a(1);
    if (!(tint_symbol_1)) {
      var tint_symbol_2 = a(2);
      if (!(tint_symbol_2)) {
        var tint_symbol_3 = a(3);
        if (!(tint_symbol_3)) {
          var tint_symbol_4 = a(4);
          if (!(tint_symbol_4)) {
            tint_symbol_4 = a(5);
          }
          tint_symbol_3 = tint_symbol_4;
        }
        tint_symbol_2 = tint_symbol_3;
      }
      tint_symbol_1 = tint_symbol_2;
    }
    tint_symbol = tint_symbol_1;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_MultipleStatements) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r1 = b && a(0);
  let r2 = a(1) || b;
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol = b;
  if (tint_symbol) {
    tint_symbol = a(0);
  }
  let r1 = tint_symbol;
  var tint_symbol_1 = a(1);
  if (!(tint_symbol_1)) {
    tint_symbol_1 = b;
  }
  let r2 = tint_symbol_1;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InCall) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn g(v : bool) -> bool {
  return v;
}

fn f() {
  var b = true;
  g(b && a(1));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn g(v : bool) -> bool {
  return v;
}

fn f() {
  var b = true;
  var tint_symbol = b;
  if (tint_symbol) {
    tint_symbol = a(1);
  }
  g(tint_symbol);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InTypeCtor) {
    auto* src = R"(

fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = (bool(a(0)) && bool(a(1) && b)) || bool(a(2) && a(3));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let tint_symbol_2 = a(0);
  var tint_symbol_1 = bool(tint_symbol_2);
  if (tint_symbol_1) {
    var tint_symbol_3 = a(1);
    if (tint_symbol_3) {
      tint_symbol_3 = b;
    }
    tint_symbol_1 = bool(tint_symbol_3);
  }
  var tint_symbol = tint_symbol_1;
  if (!(tint_symbol)) {
    var tint_symbol_4 = a(2);
    if (tint_symbol_4) {
      tint_symbol_4 = a(3);
    }
    tint_symbol = bool(tint_symbol_4);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InTypeConversion) {
    auto* src = R"(

fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = true;
  let r = (bool(a(0)) && bool(a(1))) || b;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = true;
  let tint_symbol_2 = a(0);
  var tint_symbol_1 = bool(tint_symbol_2);
  if (tint_symbol_1) {
    let tint_symbol_3 = a(1);
    tint_symbol_1 = bool(tint_symbol_3);
  }
  var tint_symbol = tint_symbol_1;
  if (!(tint_symbol)) {
    tint_symbol = b;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

// Make sure we process logical binary expressions of non-logical binary
// expressions.
TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_OfNonLogical) {
    auto* src = R"(

fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let r = bool(a(0) == b) && bool(a(1) == b);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol_1 = a(0);
  var tint_symbol = bool((tint_symbol_1 == b));
  if (tint_symbol) {
    let tint_symbol_2 = a(1);
    tint_symbol = bool((tint_symbol_2 == b));
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InIntrinsic) {
    auto* src = R"(

fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = (all(a(0)) && all(a(1) && b)) || all(a(2) && a(3));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let tint_symbol_2 = a(0);
  var tint_symbol_1 = all(tint_symbol_2);
  if (tint_symbol_1) {
    var tint_symbol_3 = a(1);
    if (tint_symbol_3) {
      tint_symbol_3 = b;
    }
    tint_symbol_1 = all(tint_symbol_3);
  }
  var tint_symbol = tint_symbol_1;
  if (!(tint_symbol)) {
    var tint_symbol_4 = a(2);
    if (tint_symbol_4) {
      tint_symbol_4 = a(3);
    }
    tint_symbol = all(tint_symbol_4);
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InMemberAccessor) {
    auto* src = R"(

struct S {
  v : bool,
}

fn a(i : i32) -> S {
  return S();
}

fn f() {
  var b = true;
  let r = a(0).v && b && a(1).v;
}
)";

    auto* expect = R"(
struct S {
  v : bool,
}

fn a(i : i32) -> S {
  return S();
}

fn f() {
  var b = true;
  let tint_symbol_2 = a(0);
  var tint_symbol_1 = tint_symbol_2.v;
  if (tint_symbol_1) {
    tint_symbol_1 = b;
  }
  var tint_symbol = tint_symbol_1;
  if (tint_symbol) {
    let tint_symbol_3 = a(1);
    tint_symbol = tint_symbol_3.v;
  }
  let r = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InUnary) {
    auto* src = R"(

fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = !(b || a(1));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol = b;
  if (!(tint_symbol)) {
    tint_symbol = a(1);
  }
  let r = !(tint_symbol);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InBitcast) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  let r = bitcast<u32>(i32(a(0) && a(1)));
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol = a(0);
  if (tint_symbol) {
    tint_symbol = a(1);
  }
  let r = bitcast<u32>(i32(tint_symbol));
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InForLoopInit) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  for(var r = a(0) && b; ; ) {
    var marker = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var tint_symbol = a(0);
  if (tint_symbol) {
    tint_symbol = b;
  }
  for(var r = tint_symbol; ; ) {
    var marker = 0;
    break;
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InForLoopCond) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  for(; a(0) && b;) {
    var marker = 0;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  loop {
    var tint_symbol = a(0);
    if (tint_symbol) {
      tint_symbol = b;
    }
    if (!(tint_symbol)) {
      break;
    }
    {
      var marker = 0;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InForLoopCont) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var r = true;
  for(; ; r = a(0) && b) {
    var marker = 0;
    break;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var r = true;
  loop {
    {
      var marker = 0;
      break;
    }

    continuing {
      var tint_symbol = a(0);
      if (tint_symbol) {
        tint_symbol = b;
      }
      r = tint_symbol;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InForLoopInitCondCont) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  var r = true;
  for(var r = a(0) && b; a(1) && c; r = a(2) && d) {
    var marker = 0;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  var c = true;
  var d = true;
  var r = true;
  {
    var tint_symbol = a(0);
    if (tint_symbol) {
      tint_symbol = b;
    }
    var r = tint_symbol;
    loop {
      var tint_symbol_1 = a(1);
      if (tint_symbol_1) {
        tint_symbol_1 = c;
      }
      if (!(tint_symbol_1)) {
        break;
      }
      {
        var marker = 0;
      }

      continuing {
        var tint_symbol_2 = a(2);
        if (tint_symbol_2) {
          tint_symbol_2 = d;
        }
        r = tint_symbol_2;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InWhileCond) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  while(a(0) && b) {
    var marker = 0;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  loop {
    var tint_symbol = a(0);
    if (tint_symbol) {
      tint_symbol = b;
    }
    if (!(tint_symbol)) {
      break;
    }
    {
      var marker = 0;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InElseIf) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  if (true) {
    var marker = 0;
  } else if (a(0) && b) {
    var marker = 1;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  if (true) {
    var marker = 0;
  } else {
    var tint_symbol = a(0);
    if (tint_symbol) {
      tint_symbol = b;
    }
    if (tint_symbol) {
      var marker = 1;
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Logical_InElseIfChain) {
    auto* src = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else if (a(0) && b) {
    var marker = 2;
  } else if (a(1) && a(2)) {
    var marker = 3;
  } else if (true) {
    var marker = 4;
  } else {
    var marker = 5;
  }
}
)";

    auto* expect = R"(
fn a(i : i32) -> bool {
  return true;
}

fn f() {
  var b = true;
  if (true) {
    var marker = 0;
  } else if (true) {
    var marker = 1;
  } else {
    var tint_symbol = a(0);
    if (tint_symbol) {
      tint_symbol = b;
    }
    if (tint_symbol) {
      var marker = 2;
    } else {
      var tint_symbol_1 = a(1);
      if (tint_symbol_1) {
        tint_symbol_1 = a(2);
      }
      if (tint_symbol_1) {
        var marker = 3;
      } else if (true) {
        var marker = 4;
      } else {
        var marker = 5;
      }
    }
  }
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_NoSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  var c = 1;
  let r = g(b, c, 3);
}
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_OneSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let r = g(a(0), b, 3);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let r = g(tint_symbol, b, 3);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_AllSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}

fn f() {
  let r = g(a(0), a(1), a(2));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  let tint_symbol_2 = a(2);
  let r = g(tint_symbol, tint_symbol_1, tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_MiddleNotSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}


fn f() {
  var b = 1;
  let r = g(a(0), b, a(1));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(a : i32, b : i32, c : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 1;
  let tint_symbol = a(0);
  let tint_symbol_1 = b;
  let tint_symbol_2 = a(1);
  let r = g(tint_symbol, tint_symbol_1, tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_InBinary) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(i : i32, j : i32, k : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 0;
  var c = 0;
  var d = 0;
  let r = b + g(c, a(0), d) + a(1);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn g(i : i32, j : i32, k : i32) -> i32 {
  return 1;
}

fn f() {
  var b = 0;
  var c = 0;
  var d = 0;
  let tint_symbol = b;
  let tint_symbol_1 = c;
  let tint_symbol_2 = a(0);
  let tint_symbol_3 = g(tint_symbol_1, tint_symbol_2, d);
  let tint_symbol_4 = a(1);
  let r = ((tint_symbol + tint_symbol_3) + tint_symbol_4);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor_2D_LeftSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  var c = 1;
  var r = b[a(0)][c];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  var c = 1;
  let tint_symbol = a(0);
  var r = b[tint_symbol][c];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor_2D_RightSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  var c = 1;
  let tint_symbol = c;
  let tint_symbol_1 = a(0);
  var r = b[tint_symbol][tint_symbol_1];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  var c = 1;
  let tint_symbol = c;
  let tint_symbol_1 = a(0);
  var r = b[tint_symbol][tint_symbol_1];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor_2D_BothSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  var r = b[a(0)][a(1)];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  var r = b[tint_symbol][tint_symbol_1];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToPhony) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  _ = a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  _ = a(0);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToArray1D) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<i32, 10>();
  b[a(0)] = a(1);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<i32, 10>();
  let tint_symbol = a(1);
  let tint_symbol_1 = a(0);
  b[tint_symbol_1] = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToArray2D) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  b[a(0)][a(1)] = a(2);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<i32, 10>, 10>();
  let tint_symbol = a(2);
  let tint_symbol_1 = a(0);
  let tint_symbol_2 = a(1);
  b[tint_symbol_1][tint_symbol_2] = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToArray3D) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<array<i32, 10>, 10>, 10>();
  b[a(0)][a(1)][a(2)] = a(3);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<array<array<i32, 10>, 10>, 10>();
  let tint_symbol = a(3);
  let tint_symbol_1 = a(0);
  let tint_symbol_2 = a(1);
  let tint_symbol_3 = a(2);
  b[tint_symbol_1][tint_symbol_2][tint_symbol_3] = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToArray_FromArray) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<i32, 3>();
  var d = array<array<i32, 3>, 3>();
  var a_1 = 0;
  b[a(2)] = d[a(0)][a_1];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = array<i32, 3>();
  var d = array<array<i32, 3>, 3>();
  var a_1 = 0;
  let tint_symbol = a(0);
  let tint_symbol_1 = a_1;
  let tint_symbol_2 = d[tint_symbol][tint_symbol_1];
  let tint_symbol_3 = a(2);
  b[tint_symbol_3] = tint_symbol_2;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToVec_BothSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = vec3<i32>();
  b[a(0)] = a(1);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = vec3<i32>();
  let tint_symbol = a(1);
  let tint_symbol_1 = a(0);
  b[tint_symbol_1] = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToVec_LeftSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = vec3<i32>();
  var c = 0;
  b[a(0)] = c;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = vec3<i32>();
  var c = 0;
  let tint_symbol = c;
  let tint_symbol_1 = a(0);
  b[tint_symbol_1] = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Assignment_ToVec_RightSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = vec3<i32>();
  var c = 0;
  b[c] = a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var b = vec3<i32>();
  var c = 0;
  let tint_symbol = a(0);
  b[c] = tint_symbol;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeConstructor_Struct) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

struct S {
  x : i32,
  y : i32,
  z : i32,
}

fn f() {
  var r = S(a(0), a(1), a(2));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

struct S {
  x : i32,
  y : i32,
  z : i32,
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  let tint_symbol_2 = a(2);
  var r = S(tint_symbol, tint_symbol_1, tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeConstructor_Array1D) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var r = array<i32, 3>(a(0), a(1), a(2));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  let tint_symbol_2 = a(2);
  var r = array<i32, 3>(tint_symbol, tint_symbol_1, tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeConstructor_Array2D) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  var r = array<array<i32, 2>, 2>(array<i32, 2>(a(0), a(1)), array<i32, 2>(a(2), a(3)));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 1;
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  let tint_symbol_2 = array<i32, 2>(tint_symbol, tint_symbol_1);
  let tint_symbol_3 = a(2);
  let tint_symbol_4 = a(3);
  let tint_symbol_5 = array<i32, 2>(tint_symbol_3, tint_symbol_4);
  var r = array<array<i32, 2>, 2>(tint_symbol_2, tint_symbol_5);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, MemberAccessor_Vec) {
    auto* src = R"(
fn a(i : i32) -> vec3<i32> {
  return vec3<i32>();
}

fn f() {
  var r = a(0).x + a(1).y;
}
)";

    auto* expect = R"(
fn a(i : i32) -> vec3<i32> {
  return vec3<i32>();
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  var r = (tint_symbol.x + tint_symbol_1.y);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, MemberAccessor_Struct) {
    auto* src = R"(
struct S {
  x : i32,
  y : i32,
}

fn a(i : i32) -> S {
  return S();
}

fn f() {
  var r = a(0).x + a(1).y;
}
)";

    auto* expect = R"(
struct S {
  x : i32,
  y : i32,
}

fn a(i : i32) -> S {
  return S();
}

fn f() {
  let tint_symbol = a(0);
  let tint_symbol_1 = a(1);
  var r = (tint_symbol.x + tint_symbol_1.y);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, MemberAccessor_Struct_Mixed) {
    auto* src = R"(
struct S {
  x : i32,
  y : i32,
  arr : array<i32, 10>,
}

fn a(i : i32) -> S {
  return S();
}

fn b(i : i32) -> i32 {
  return 0;
}

fn f() {
  var i = 0;
  var j = 0;
  var k = 0;
  var l = 0;
  var m = 0;
  var r = a(0).x + i + a(1).y + j + a(2).arr[k + b(3) + l] + m;
}
)";

    auto* expect = R"(
struct S {
  x : i32,
  y : i32,
  arr : array<i32, 10>,
}

fn a(i : i32) -> S {
  return S();
}

fn b(i : i32) -> i32 {
  return 0;
}

fn f() {
  var i = 0;
  var j = 0;
  var k = 0;
  var l = 0;
  var m = 0;
  let tint_symbol = a(0);
  let tint_symbol_1 = i;
  let tint_symbol_2 = a(1);
  let tint_symbol_3 = j;
  let tint_symbol_4 = a(2);
  let tint_symbol_5 = k;
  let tint_symbol_6 = b(3);
  var r = (((((tint_symbol.x + tint_symbol_1) + tint_symbol_2.y) + tint_symbol_3) + tint_symbol_4.arr[((tint_symbol_5 + tint_symbol_6) + l)]) + m);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor_Plus_SE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<i32, 10>();
    let r = v[0] + a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  let tint_symbol = v[0];
  let tint_symbol_1 = a(0);
  let r = (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor_Of_SE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<i32, 10>();
    let r = v[a(0)];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  let tint_symbol = a(0);
  let r = v[tint_symbol];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor2_Of_LeftSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<array<i32, 10>, 10>();
    let r = v[a(0)][0];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<array<i32, 10>, 10>();
  let tint_symbol = a(0);
  let r = v[tint_symbol][0];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor2_Of_RightSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<array<i32, 10>, 10>();
    let r = v[0][a(0)];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<array<i32, 10>, 10>();
  let tint_symbol = a(0);
  let r = v[0][tint_symbol];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor2_Of_SEAndVar) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<array<i32, 10>, 10>();
    var b : i32;
    let r = v[a(0)][b];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<array<i32, 10>, 10>();
  var b : i32;
  let tint_symbol = a(0);
  let r = v[tint_symbol][b];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor2_Of_VarAndSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<array<i32, 10>, 10>();
    var b : i32;
    let r = v[b][a(0)];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<array<i32, 10>, 10>();
  var b : i32;
  let tint_symbol = b;
  let tint_symbol_1 = a(0);
  let r = v[tint_symbol][tint_symbol_1];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessorOfVar_Plus_SE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  var b = 0;
  let r = v[b] + a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  var b = 0;
  let tint_symbol = b;
  let tint_symbol_1 = v[tint_symbol];
  let tint_symbol_2 = a(0);
  let r = (tint_symbol_1 + tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessor_Plus_IndexAccessorOfSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<i32, 10>();
    let r = v[0] + v[a(0)];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  let tint_symbol = v[0];
  let tint_symbol_1 = a(0);
  let r = (tint_symbol + v[tint_symbol_1]);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, AssignTo_IndexAccessorOfIndexAccessorOfSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<i32, 10>();
    var w = array<i32, 10>();
    v[w[a(0)]] = 1;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  var w = array<i32, 10>();
  let tint_symbol = a(0);
  v[w[tint_symbol]] = 1;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, AssignTo_IndexAccessorOfIndexAccessorOfLiteralPlusSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<i32, 10>();
    var w = array<i32, 10>();
    v[w[0] + a(0)] = 1;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  var w = array<i32, 10>();
  let tint_symbol = w[0];
  let tint_symbol_1 = a(0);
  v[(tint_symbol + tint_symbol_1)] = 1;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest,
       AssignTo_IndexAccessorOfIndexAccessorOfLiteralPlusIndexAccessorOfSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
    var v = array<i32, 10>();
    var w = array<i32, 10>();
    v[w[0] + w[a(0)]] = 1;
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn f() {
  var v = array<i32, 10>();
  var w = array<i32, 10>();
  let tint_symbol = w[0];
  let tint_symbol_1 = a(0);
  v[(tint_symbol + w[tint_symbol_1])] = 1;
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, IndexAccessorOfLhsSERhsSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn b() -> array<i32, 10> {
    return array<i32, 10>();
}

fn f() {
    let r = b()[a(0)];
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn b() -> array<i32, 10> {
  return array<i32, 10>();
}

fn f() {
  let tint_symbol = b();
  let tint_symbol_1 = a(0);
  let r = tint_symbol[tint_symbol_1];
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, BinaryIndexAccessorOfLhsSERhsSE) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn b() -> array<i32, 10> {
    return array<i32, 10>();
}

fn f() {
    let r = b()[0] + a(0);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return i;
}

fn b() -> array<i32, 10> {
  return array<i32, 10>();
}

fn f() {
  let tint_symbol = b();
  let tint_symbol_1 = a(0);
  let r = (tint_symbol[0] + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, BinaryMemberAccessorPlusSE) {
    // bclayton@'s example:
    // https://dawn-review.googlesource.com/c/tint/+/78620/6..8/src/transform/promote_side_effects_to_decl.cc#b490
    auto* src = R"(
fn modify_vec(p : ptr<function, vec4<i32>>) -> i32 {
  (*p).x = 42;
  return 0;
}

fn f() {
  var v = vec4<i32>();
  let l = v.x + modify_vec(&v);
  // l should be 0, not 42
}
)";

    auto* expect = R"(
fn modify_vec(p : ptr<function, vec4<i32>>) -> i32 {
  (*(p)).x = 42;
  return 0;
}

fn f() {
  var v = vec4<i32>();
  let tint_symbol = v.x;
  let tint_symbol_1 = modify_vec(&(v));
  let l = (tint_symbol + tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_ReadOnlyArgAndSE) {
    // Make sure that read-only args don't get hoisted (tex and samp)
    auto* src = R"(
@group(1) @binding(1) var tex: texture_2d_array<u32>;
@group(1) @binding(2) var samp: sampler;

fn get_uv() -> vec2<f32> {
  return vec2<f32>(1.0, 2.0);
}

fn f() {
  let r = textureGather(1, tex, samp, get_uv(), 1);
}
)";

    auto* expect = R"(
@group(1) @binding(1) var tex : texture_2d_array<u32>;

@group(1) @binding(2) var samp : sampler;

fn get_uv() -> vec2<f32> {
  return vec2<f32>(1.0, 2.0);
}

fn f() {
  let tint_symbol = get_uv();
  let r = textureGather(1, tex, samp, tint_symbol, 1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Call_PtrArgAndSE) {
    // Make sure that read-only args don't get hoisted (tex and samp)
    auto* src = R"(

var<private> b : i32 = 0;

fn a(i : i32) -> i32 {
  b = 42;
  return 0;
}

fn g(i : ptr<private, i32>, j : i32) -> i32 {
  return *i;
}

fn f() {
  // a(0) should be hoisted, but not &b
  let r = g(&b, a(0));
  // r should be 42
}
)";

    auto* expect = R"(
var<private> b : i32 = 0;

fn a(i : i32) -> i32 {
  b = 42;
  return 0;
}

fn g(i : ptr<private, i32>, j : i32) -> i32 {
  return *(i);
}

fn f() {
  let tint_symbol = a(0);
  let r = g(&(b), tint_symbol);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, TypeCtor_VarPlusI32CtorPlusVar) {
    auto* src = R"(
fn f() {
  var b = 0;
  var c = 0;
  var d = 0;
  let r = ((b + i32(c)) + d);
}
)";

    auto* expect = src;

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Mixed_ArithPlusLogical) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : bool) -> i32 {
  return 0;
}

fn f() {
  let r = a(0) + g(b(1) && b(2));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : bool) -> i32 {
  return 0;
}

fn f() {
  let tint_symbol = a(0);
  var tint_symbol_1 = b(1);
  if (tint_symbol_1) {
    tint_symbol_1 = b(2);
  }
  let tint_symbol_2 = g(tint_symbol_1);
  let r = (tint_symbol + tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Mixed_LogicalPlusArith) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : bool) -> i32 {
  return 0;
}

fn f() {
  let r = g(b(0) && b(1)) + a(2);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : bool) -> i32 {
  return 0;
}

fn f() {
  var tint_symbol = b(0);
  if (tint_symbol) {
    tint_symbol = b(1);
  }
  let tint_symbol_1 = g(tint_symbol);
  let tint_symbol_2 = a(2);
  let r = (tint_symbol_1 + tint_symbol_2);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Mixed_ArithAndLogicalArgs) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : i32, j : bool) -> i32 {
  return 0;
}

fn f() {
  let r = g(a(0), b(1) && b(2));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : i32, j : bool) -> i32 {
  return 0;
}

fn f() {
  let tint_symbol = a(0);
  var tint_symbol_1 = b(1);
  if (tint_symbol_1) {
    tint_symbol_1 = b(2);
  }
  let r = g(tint_symbol, tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Mixed_LogicalAndArithArgs) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : bool, j : i32) -> i32 {
  return 0;
}

fn f() {
  let r = g(b(0) && b(1), a(2));
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn g(i : bool, j : i32) -> i32 {
  return 0;
}

fn f() {
  var tint_symbol = b(0);
  if (tint_symbol) {
    tint_symbol = b(1);
  }
  let tint_symbol_1 = a(2);
  let r = g(tint_symbol, tint_symbol_1);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteSideEffectsToDeclTest, Binary_Mixed_Complex) {
    auto* src = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn c(i : bool) -> i32 {
  return 0;
}

fn g(i : bool, j : i32) -> i32 {
  return 0;
}

fn f() {
  let r = a(0) + g(b(1) && b(a(2) + a(3)), a(4)) + a(5);
}
)";

    auto* expect = R"(
fn a(i : i32) -> i32 {
  return 0;
}

fn b(i : i32) -> bool {
  return true;
}

fn c(i : bool) -> i32 {
  return 0;
}

fn g(i : bool, j : i32) -> i32 {
  return 0;
}

fn f() {
  let tint_symbol = a(0);
  var tint_symbol_1 = b(1);
  if (tint_symbol_1) {
    let tint_symbol_2 = a(2);
    let tint_symbol_3 = a(3);
    tint_symbol_1 = b((tint_symbol_2 + tint_symbol_3));
  }
  let tint_symbol_4 = a(4);
  let tint_symbol_5 = g(tint_symbol_1, tint_symbol_4);
  let tint_symbol_6 = a(5);
  let r = ((tint_symbol + tint_symbol_5) + tint_symbol_6);
}
)";

    DataMap data;
    auto got = Run<PromoteSideEffectsToDecl>(src, data);

    EXPECT_EQ(expect, str(got));
}
}  // namespace
}  // namespace tint::transform

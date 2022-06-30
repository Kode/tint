// Copyright 2020 The Tint Authors.
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

#include "gmock/gmock.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using ::testing::HasSubstr;

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement) {
    auto* var = Var("a", ty.f32(), ast::StorageClass::kNone);
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Let) {
    auto* var = Let("a", ty.f32(), Construct(ty.f32()));
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), "  float const a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const) {
    auto* var = Const("a", ty.f32(), Construct(ty.f32()));
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), "");  // Not a mistake - 'const' is inlined
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_AInt) {
    auto* C = Const("C", nullptr, Expr(1_a));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int const l = 1;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_AFloat) {
    auto* C = Const("C", nullptr, Expr(1._a));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float const l = 1.0f;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_i32) {
    auto* C = Const("C", nullptr, Expr(1_i));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int const l = 1;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_u32) {
    auto* C = Const("C", nullptr, Expr(1_u));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  uint const l = 1u;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_f32) {
    auto* C = Const("C", nullptr, Expr(1_f));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float const l = 1.0f;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_AInt) {
    auto* C = Const("C", nullptr, Construct(ty.vec3(nullptr), 1_a, 2_a, 3_a));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  int3 const l = int3(1, 2, 3);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_AFloat) {
    auto* C = Const("C", nullptr, Construct(ty.vec3(nullptr), 1._a, 2._a, 3._a));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_vec3_f32) {
    auto* C = Const("C", nullptr, vec3<f32>(1_f, 2_f, 3_f));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_mat2x3_AFloat) {
    auto* C =
        Const("C", nullptr, Construct(ty.mat(nullptr, 2, 3), 1._a, 2._a, 3._a, 4._a, 5._a, 6._a));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_mat2x3_f32) {
    auto* C = Const("C", nullptr, mat2x3<f32>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_arr_f32) {
    auto* C = Const("C", nullptr, Construct(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

void f() {
  tint_array<float, 3> const l = tint_array<float, 3>{1.0f, 2.0f, 3.0f};
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const_arr_vec2_bool) {
    auto* C = Const("C", nullptr,
                    Construct(ty.array(ty.vec2<bool>(), 3_u),  //
                              vec2<bool>(true, false),         //
                              vec2<bool>(false, true),         //
                              vec2<bool>(true, true)));
    Func("f", {}, ty.void_(), {Decl(C), Decl(Let("l", nullptr, Expr(C)))});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

void f() {
  tint_array<bool2, 3> const l = tint_array<bool2, 3>{bool2(true, false), bool2(false, true), bool2(true)};
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Array) {
    auto* var = Var("a", ty.array<f32, 5>(), ast::StorageClass::kNone);
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), "  tint_array<float, 5> a = {};\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Struct) {
    auto* s = Structure("S", {
                                 Member("a", ty.f32()),
                                 Member("b", ty.f32()),
                             });

    auto* var = Var("a", ty.Of(s), ast::StorageClass::kNone);
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), R"(  S a = {};
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Vector) {
    auto* var = Var("a", ty.vec2<f32>());
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), "  float2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Matrix) {
    auto* var = Var("a", ty.mat3x2<f32>());

    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), "  float3x2 a = float3x2(0.0f);\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
    GlobalVar("a", ty.f32(), ast::StorageClass::kPrivate);

    WrapInFunction(Expr("a"));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("thread float tint_symbol_1 = 0.0f;\n"));
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Workgroup) {
    GlobalVar("a", ty.f32(), ast::StorageClass::kWorkgroup);

    WrapInFunction(Expr("a"));

    GeneratorImpl& gen = SanitizeAndBuild();

    gen.increment_indent();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("threadgroup float tint_symbol_2;\n"));
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_ZeroVec) {
    auto* zero_vec = vec3<f32>();

    auto* var = Var("a", ty.vec3<f32>(), ast::StorageClass::kNone, zero_vec);
    auto* stmt = Decl(var);
    WrapInFunction(stmt);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
    EXPECT_EQ(gen.result(), R"(float3 a = float3(0.0f);
)");
}

}  // namespace
}  // namespace tint::writer::msl

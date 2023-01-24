// Copyright 2022 The Tint Authors.
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

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverF16ExtensionTest = ResolverTest;

TEST_F(ResolverF16ExtensionTest, TypeUsedWithExtension) {
    // enable f16;
    // var<private> v : f16;
    Enable(ast::Extension::kF16);

    GlobalVar("v", ty.f16(), type::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, TypeUsedWithoutExtension) {
    // var<private> v : f16;
    GlobalVar("v", ty.f16(Source{{12, 34}}), type::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeUsedWithExtension) {
    // enable f16;
    // var<private> v : vec2<f16>;
    Enable(ast::Extension::kF16);

    GlobalVar("v", ty.vec2<f16>(), type::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeUsedWithoutExtension) {
    // var<private> v : vec2<f16>;
    GlobalVar("v", ty.vec2(ty.f16(Source{{12, 34}})), type::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeInitUsedWithExtension) {
    // enable f16;
    // var<private> v = vec2<f16>();
    Enable(ast::Extension::kF16);

    GlobalVar("v", Construct(ty.vec2<f16>()), type::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeInitUsedWithoutExtension) {
    // var<private> v = vec2<f16>();
    GlobalVar("v", Construct(ty.vec2(ty.f16(Source{{12, 34}}))), type::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeConvUsedWithExtension) {
    // enable f16;
    // var<private> v = vec2<f16>(vec2<f32>());
    Enable(ast::Extension::kF16);

    GlobalVar("v", Construct(ty.vec2<f16>(), Construct(ty.vec2<f32>())),
              type::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeConvUsedWithoutExtension) {
    // var<private> v = vec2<f16>(vec2<f32>());
    GlobalVar("v", Construct(ty.vec2(ty.f16(Source{{12, 34}})), Construct(ty.vec2<f32>())),
              type::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, F16LiteralUsedWithExtension) {
    // enable f16;
    // var<private> v = 16h;
    Enable(ast::Extension::kF16);

    GlobalVar("v", Expr(16_h), type::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, F16LiteralUsedWithoutExtension) {
    // var<private> v = 16h;
    GlobalVar("v", Expr(Source{{12, 34}}, 16_h), type::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

using ResolverF16ExtensionShortNameTest = ResolverTestWithParam<const char*>;

TEST_P(ResolverF16ExtensionShortNameTest, Vec2hTypeUsedWithExtension) {
    // enable f16;
    // var<private> v : vec2h;
    Enable(ast::Extension::kF16);

    GlobalVar("v", ty.type_name(Source{{12, 34}}, GetParam()), type::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(ResolverF16ExtensionShortNameTest, Vec2hTypeUsedWithoutExtension) {
    // var<private> v : vec2h;
    GlobalVar("v", ty.type_name(Source{{12, 34}}, GetParam()), type::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

INSTANTIATE_TEST_SUITE_P(ResolverF16ExtensionShortNameTest,
                         ResolverF16ExtensionShortNameTest,
                         testing::Values("mat2x2h",
                                         "mat2x3h",
                                         "mat2x4h",
                                         "mat3x2h",
                                         "mat3x3h",
                                         "mat3x4h",
                                         "mat4x2h",
                                         "mat4x3h",
                                         "mat4x4h",
                                         "vec2h",
                                         "vec3h",
                                         "vec4h"));

}  // namespace
}  // namespace tint::resolver
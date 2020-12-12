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

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, ArrayAccessor) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // vec3<f32> ary;
  // ary[1]  -> ptr<f32>

  ast::Variable var(Source{}, "ary", ast::StorageClass::kFunction, &vec3, false,
                    nullptr, ast::VariableDecorationList{});

  auto* ary = create<ast::IdentifierExpression>(
      Source{}, mod->RegisterSymbol("ary"), "ary");
  auto* idx_expr = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::SintLiteral>(Source{}, &i32, 1));

  ast::ArrayAccessorExpression expr(Source{}, ary, idx_expr);

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, Accessor_Array_LoadIndex) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // ary : vec3<f32>;
  // idx : i32;
  // ary[idx]  -> ptr<f32>

  ast::Variable var(Source{}, "ary", ast::StorageClass::kFunction, &vec3, false,
                    nullptr, ast::VariableDecorationList{});
  ast::Variable idx(Source{}, "idx", ast::StorageClass::kFunction, &i32, false,
                    nullptr, ast::VariableDecorationList{});

  auto* ary = create<ast::IdentifierExpression>(
      Source{}, mod->RegisterSymbol("ary"), "ary");
  auto* idx_expr = create<ast::IdentifierExpression>(
      Source{}, mod->RegisterSymbol("idx"), "idx");

  ast::ArrayAccessorExpression expr(Source{}, ary, idx_expr);

  td.RegisterVariableForTesting(&var);
  td.RegisterVariableForTesting(&idx);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(&idx)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 12u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%8 = OpTypeInt 32 1
%7 = OpTypePointer Function %8
%9 = OpConstantNull %8
%11 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
%6 = OpVariable %7 Function %9
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %8 %6
%12 = OpAccessChain %11 %1 %10
)");
}

TEST_F(BuilderTest, ArrayAccessor_Dynamic) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // vec3<f32> ary;
  // ary[1 + 2]  -> ptr<f32>

  ast::Variable var(Source{}, "ary", ast::StorageClass::kFunction, &vec3, false,
                    nullptr, ast::VariableDecorationList{});

  auto* ary = create<ast::IdentifierExpression>(
      Source{}, mod->RegisterSymbol("ary"), "ary");

  ast::ArrayAccessorExpression expr(
      Source{}, ary,
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kAdd,
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 1)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%10 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpIAdd %6 %7 %8
%11 = OpAccessChain %10 %1 %9
)");
}

TEST_F(BuilderTest, ArrayAccessor_MultiLevel) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Array ary4(&vec3, 4, ast::ArrayDecorationList{});

  // ary = array<vec3<f32>, 4>
  // ary[3][2];

  ast::Variable var(Source{}, "ary", ast::StorageClass::kFunction, &ary4, false,
                    nullptr, ast::VariableDecorationList{});

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::ArrayAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(Source{},
                                            mod->RegisterSymbol("ary"), "ary"),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 3))),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::SintLiteral>(Source{}, &i32, 2)));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 13u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%8 = OpConstantNull %3
%9 = OpTypeInt 32 1
%10 = OpConstant %9 3
%11 = OpConstant %9 2
%12 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %8
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%13 = OpAccessChain %12 %1 %10 %11
)");
}

TEST_F(BuilderTest, Accessor_ArrayWithSwizzle) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Array ary4(&vec3, 4, ast::ArrayDecorationList{});

  // var a : array<vec3<f32>, 4>;
  // a[2].xy;

  ast::Variable var(Source{}, "ary", ast::StorageClass::kFunction, &ary4, false,
                    nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::ArrayAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(Source{},
                                            mod->RegisterSymbol("ary"), "ary"),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("xy"),
                                        "xy"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 15u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%8 = OpConstantNull %3
%9 = OpTypeInt 32 1
%10 = OpConstant %9 2
%11 = OpTypePointer Function %4
%13 = OpTypeVector %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %8
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpAccessChain %11 %1 %10
%14 = OpLoad %4 %12
%15 = OpVectorShuffle %13 %14 %14 0 1
)");
}

TEST_F(BuilderTest, MemberAccessor) {
  ast::type::F32 f32;

  // my_struct {
  //   a : f32
  //   b : f32
  // }
  // var ident : my_struct
  // ident.b

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>(Source{}, "a", &f32, decos));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, decos));

  auto* s = create<ast::Struct>(Source{}, members, ast::StructDecorationList{});
  ast::type::Struct s_type("my_struct", s);

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &s_type,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("ident"),
                                        "ident"),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("b"),
                                        "b"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested) {
  ast::type::F32 f32;

  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // ident.inner.a
  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "a", &f32, decos));
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "b", &f32, decos));

  ast::type::Struct inner_struct(
      "Inner", create<ast::Struct>(Source{}, inner_members,
                                   ast::StructDecorationList{}));

  ast::StructMemberList outer_members;
  outer_members.push_back(
      create<ast::StructMember>(Source{}, "inner", &inner_struct, decos));

  ast::type::Struct s_type("my_struct",
                           create<ast::Struct>(Source{}, outer_members,
                                               ast::StructDecorationList{}));

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &s_type,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("inner"), "inner")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("a"),
                                        "a"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpAccessChain %9 %1 %8 %8
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_WithAlias) {
  ast::type::F32 f32;

  // type Inner = struct {
  //   a : f32
  //   b : f32
  // }
  // my_struct {
  //   inner : Inner
  // }
  //
  // var ident : my_struct
  // ident.inner.a
  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "a", &f32, decos));
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "b", &f32, decos));

  ast::type::Struct inner_struct(
      "Inner", create<ast::Struct>(Source{}, inner_members,
                                   ast::StructDecorationList{}));

  ast::type::Alias alias(mod->RegisterSymbol("Inner"), "Inner", &inner_struct);

  ast::StructMemberList outer_members;
  outer_members.push_back(
      create<ast::StructMember>(Source{}, "inner", &alias, decos));

  ast::type::Struct s_type("Outer",
                           create<ast::Struct>(Source{}, outer_members,
                                               ast::StructDecorationList{}));

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &s_type,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("inner"), "inner")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("a"),
                                        "a"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpAccessChain %9 %1 %8 %8
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_LHS) {
  ast::type::F32 f32;

  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // ident.inner.a = 2.0f;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "a", &f32, decos));
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "b", &f32, decos));

  ast::type::Struct inner_struct(
      "Inner", create<ast::Struct>(Source{}, inner_members,
                                   ast::StructDecorationList{}));

  ast::StructMemberList outer_members;
  outer_members.push_back(
      create<ast::StructMember>(Source{}, "inner", &inner_struct, decos));

  ast::type::Struct s_type("my_struct",
                           create<ast::Struct>(Source{}, outer_members,
                                               ast::StructDecorationList{}));

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &s_type,
                    false, nullptr, ast::VariableDecorationList{});

  auto* lhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("inner"), "inner")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("a"),
                                        "a"));

  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 2.f));

  ast::AssignmentStatement expr(Source{}, lhs, rhs);

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&expr)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
%11 = OpConstant %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpAccessChain %9 %1 %8 %8
OpStore %10 %11
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_RHS) {
  ast::type::F32 f32;

  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // var store : f32 = ident.inner.a

  ast::StructMemberDecorationList decos;
  ast::StructMemberList inner_members;
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "a", &f32, decos));
  inner_members.push_back(
      create<ast::StructMember>(Source{}, "b", &f32, decos));

  ast::type::Struct inner_struct(
      "Inner", create<ast::Struct>(Source{}, inner_members,
                                   ast::StructDecorationList{}));

  ast::StructMemberList outer_members;
  outer_members.push_back(
      create<ast::StructMember>(Source{}, "inner", &inner_struct, decos));

  ast::type::Struct s_type("my_struct",
                           create<ast::Struct>(Source{}, outer_members,
                                               ast::StructDecorationList{}));

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &s_type,
                    false, nullptr, ast::VariableDecorationList{});
  ast::Variable store(Source{}, "store", ast::StorageClass::kFunction, &f32,
                      false, nullptr, ast::VariableDecorationList{});

  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod->RegisterSymbol("store"), "store");

  auto* rhs = create<ast::MemberAccessorExpression>(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("inner"), "inner")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("a"),
                                        "a"));

  ast::AssignmentStatement expr(Source{}, lhs, rhs);

  td.RegisterVariableForTesting(&var);
  td.RegisterVariableForTesting(&store);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(&store)) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(&expr)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%10 = OpTypeInt 32 0
%11 = OpConstant %10 0
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
%7 = OpVariable %8 Function %9
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpAccessChain %8 %1 %11 %11
%13 = OpLoad %5 %12
OpStore %7 %13
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_Single) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // ident.y

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &vec3,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("ident"),
                                        "ident"),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("y"),
                                        "y"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_MultipleNames) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // ident.yx

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &vec3,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("ident"),
                                        "ident"),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("yx"),
                                        "yx"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeVector %4 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpVectorShuffle %6 %7 %7 1 0
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_of_Swizzle) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // ident.yxz.xz

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &vec3,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(Source{},
                                            mod->RegisterSymbol("yxz"), "yxz")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("xz"),
                                        "xz"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%8 = OpTypeVector %4 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %3 %6 %6 1 0 2
%9 = OpVectorShuffle %8 %7 %7 0 2
)");
}

TEST_F(BuilderTest, MemberAccessor_Member_of_Swizzle) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // ident.yxz.x

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &vec3,
                    false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(Source{},
                                            mod->RegisterSymbol("yxz"), "yxz")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("x"),
                                        "x"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %3 %6 %6 1 0 2
%8 = OpCompositeExtract %4 %7 0
)");
}

TEST_F(BuilderTest, MemberAccessor_Array_of_Swizzle) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // index.yxz[1]

  ast::Variable var(Source{}, "ident", ast::StorageClass::kFunction, &vec3,
                    false, nullptr, ast::VariableDecorationList{});

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod->RegisterSymbol("ident"), "ident"),
          create<ast::IdentifierExpression>(Source{},
                                            mod->RegisterSymbol("yxz"), "yxz")),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::SintLiteral>(Source{}, &i32, 1)));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%8 = OpTypeInt 32 1
%9 = OpConstant %8 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %3 %6 %6 1 0 2
%10 = OpVectorExtractDynamic %4 %7 %9
)");
}

TEST_F(BuilderTest, Accessor_Mixed_ArrayAndMember) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  // type C = struct {
  //   baz : vec3<f32>
  // }
  // type B = struct {
  //  bar : C;
  // }
  // type A = struct {
  //   foo : array<B, 3>
  // }
  // var index : array<A, 2>
  // index[0].foo[2].bar.baz.yx

  ast::StructMemberDecorationList decos;

  auto* s = create<ast::Struct>(Source{},
                                ast::StructMemberList{create<ast::StructMember>(
                                    Source{}, "baz", &vec3, decos)},
                                ast::StructDecorationList{});
  ast::type::Struct c_type("C", s);

  s = create<ast::Struct>(Source{},
                          ast::StructMemberList{create<ast::StructMember>(
                              Source{}, "bar", &c_type, decos)},
                          ast::StructDecorationList{});
  ast::type::Struct b_type("B", s);

  ast::type::Array b_ary_type(&b_type, 3, ast::ArrayDecorationList{});

  s = create<ast::Struct>(Source{},
                          ast::StructMemberList{create<ast::StructMember>(
                              Source{}, "foo", &b_ary_type, decos)},
                          ast::StructDecorationList{});
  ast::type::Struct a_type("A", s);

  ast::type::Array a_ary_type(&a_type, 2, ast::ArrayDecorationList{});

  ast::Variable var(Source{}, "index", ast::StorageClass::kFunction,
                    &a_ary_type, false, nullptr, ast::VariableDecorationList{});

  ast::MemberAccessorExpression expr(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::MemberAccessorExpression>(
              Source{},
              create<ast::ArrayAccessorExpression>(
                  Source{},
                  create<ast::MemberAccessorExpression>(
                      Source{},
                      create<ast::ArrayAccessorExpression>(
                          Source{},
                          create<ast::IdentifierExpression>(
                              Source{}, mod->RegisterSymbol("index"), "index"),
                          create<ast::ScalarConstructorExpression>(
                              Source{},
                              create<ast::SintLiteral>(Source{}, &i32, 0))),
                      create<ast::IdentifierExpression>(
                          Source{}, mod->RegisterSymbol("foo"), "foo")),
                  create<ast::ScalarConstructorExpression>(
                      Source{}, create<ast::SintLiteral>(Source{}, &i32, 2))),
              create<ast::IdentifierExpression>(
                  Source{}, mod->RegisterSymbol("bar"), "bar")),
          create<ast::IdentifierExpression>(Source{},
                                            mod->RegisterSymbol("baz"), "baz")),
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("yx"),
                                        "yx"));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 22u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%7 = OpTypeStruct %8
%6 = OpTypeStruct %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 3
%5 = OpTypeArray %6 %11
%4 = OpTypeStruct %5
%12 = OpConstant %10 2
%3 = OpTypeArray %4 %12
%2 = OpTypePointer Function %3
%13 = OpConstantNull %3
%14 = OpTypeInt 32 1
%15 = OpConstant %14 0
%16 = OpConstant %10 0
%17 = OpConstant %14 2
%18 = OpTypePointer Function %8
%20 = OpTypeVector %9 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %13
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%19 = OpAccessChain %18 %1 %15 %16 %17 %16 %16
%21 = OpLoad %8 %19
%22 = OpVectorShuffle %20 %21 %21 1 0
)");
}

TEST_F(BuilderTest, Accessor_Array_Of_Vec) {
  // const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
  //   vec2<f32>(0.0, 0.5),
  //   vec2<f32>(-0.5, -0.5),
  //   vec2<f32>(0.5, -0.5));
  // pos[1]

  ast::type::F32 f32;
  ast::type::U32 u32;
  ast::type::Vector vec(&f32, 2);
  ast::type::Array arr(&vec, 3, ast::ArrayDecorationList{});

  ast::ExpressionList ary_params;
  ary_params.push_back(create<ast::TypeConstructorExpression>(
      Source{}, &vec,
      ast::ExpressionList{
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, 0.0)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, 0.5)),
      }));

  ary_params.push_back(create<ast::TypeConstructorExpression>(
      Source{}, &vec,
      ast::ExpressionList{
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, -0.5)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, -0.5)),
      }));

  ary_params.push_back(create<ast::TypeConstructorExpression>(
      Source{}, &vec,
      ast::ExpressionList{
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, 0.5)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, -0.5)),
      }));

  ast::Variable var(
      Source{}, "pos", ast::StorageClass::kPrivate, &arr, true,
      create<ast::TypeConstructorExpression>(Source{}, &arr, ary_params),
      ast::VariableDecorationList{});

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("pos"),
                                        "pos"),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::UintLiteral>(Source{}, &u32, 1)));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(var.constructor())) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 18u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%4 = OpTypeInt 32 0
%5 = OpConstant %4 3
%1 = OpTypeArray %2 %5
%6 = OpConstant %3 0
%7 = OpConstant %3 0.5
%8 = OpConstantComposite %2 %6 %7
%9 = OpConstant %3 -0.5
%10 = OpConstantComposite %2 %9 %9
%11 = OpConstantComposite %2 %7 %9
%12 = OpConstantComposite %1 %8 %10 %11
%13 = OpTypePointer Function %1
%15 = OpConstantNull %1
%16 = OpConstant %4 1
%17 = OpTypePointer Function %2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%14 = OpVariable %13 Function %15
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %14 %12
%18 = OpAccessChain %17 %14 %16
)");
}

TEST_F(BuilderTest, Accessor_Const_Vec) {
  // const pos : vec2<f32> = vec2<f32>(0.0, 0.5);
  // pos[1]

  ast::type::F32 f32;
  ast::type::U32 u32;
  ast::type::Vector vec(&f32, 2);

  ast::ExpressionList vec_params;
  vec_params.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 0.0)));
  vec_params.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 0.5)));

  ast::Variable var(Source{}, "pos", ast::StorageClass::kPrivate, &vec, true,
                    create<ast::TypeConstructorExpression>(
                        Source{}, &vec, std::move(vec_params)),
                    ast::VariableDecorationList{});

  ast::ArrayAccessorExpression expr(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod->RegisterSymbol("pos"),
                                        "pos"),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::UintLiteral>(Source{}, &u32, 1)));

  td.RegisterVariableForTesting(&var);
  ASSERT_TRUE(td.DetermineResultType(var.constructor())) << td.error();
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(&var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(&expr), 8u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0
%4 = OpConstant %2 0.5
%5 = OpConstantComposite %1 %3 %4
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), "");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpVectorExtractDynamic %2 %5 %7
)");
}

TEST_F(BuilderTest, DISABLED_Accessor_Array_NonPointer) {
  // const a : array<f32, 3>;
  // a[2]
  //
  // This has to generate an OpConstantExtract and will need to read the 3 value
  // out of the ScalarConstructor as extract requires integer indices.
}

TEST_F(BuilderTest, DISABLED_Accessor_Struct_NonPointer) {
  // type A = struct {
  //   a : f32;
  //   b : f32;
  // };
  // const b : A;
  // b.b
  //
  // This needs to do an OpCompositeExtract on the struct.
}

TEST_F(BuilderTest, DISABLED_Accessor_NonPointer_Multi) {
  // type A = struct {
  //   a : f32;
  //   b : vec3<f32, 3>;
  // };
  // type B = struct {
  //   c : A;
  // }
  // const b : array<B, 3>;
  // b[2].c.b.yx.x
  //
  // This needs to do an OpCompositeExtract similar to the AccessChain case
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint

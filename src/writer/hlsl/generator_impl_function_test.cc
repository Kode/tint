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

#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(Source{}, mod.RegisterSymbol("my_func"),
                                     "my_func", ast::VariableList{}, &void_type,
                                     body, ast::FunctionDecorationList{});

  mod.AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("GeometryShader"), "GeometryShader",
      ast::VariableList{}, &void_type, body, ast::FunctionDecorationList{});

  mod.AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void GeometryShader_tint_0() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "a",                              // name
                            ast::StorageClass::kNone,         // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "b",                              // name
                            ast::StorageClass::kNone,         // storage_class
                            &i32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(Source{}, mod.RegisterSymbol("my_func"),
                                     "my_func", params, &void_type, body,
                                     ast::FunctionDecorationList{});

  mod.AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithInOutVars) {
  ast::type::Void void_type;
  ast::type::F32 f32;

  auto* foo_var =
      create<ast::Variable>(Source{},                   // source
                            "foo",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &f32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                    // source
                            "bar",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("bar"),
                                        "bar"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("foo"),
                                        "foo")));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct frag_main_in {
  float foo : TEXCOORD0;
};

struct frag_main_out {
  float bar : SV_Target1;
};

frag_main_out frag_main(frag_main_in tint_in) {
  frag_main_out tint_out;
  tint_out.bar = tint_in.foo;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithInOut_Builtins) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* coord_var = create<ast::Variable>(
      Source{},                   // source
      "coord",                    // name
      ast::StorageClass::kInput,  // storage_class
      &vec4,                      // type
      false,                      // is_const
      nullptr,                    // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord, Source{}),
      });

  auto* depth_var = create<ast::Variable>(
      Source{},                    // source
      "depth",                     // name
      ast::StorageClass::kOutput,  // storage_class
      &f32,                        // type
      false,                       // is_const
      nullptr,                     // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}),
      });

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod.AddGlobalVariable(coord_var);
  mod.AddGlobalVariable(depth_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("depth"),
                                        "depth"),
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("coord"), "coord"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("x"),
                                            "x"))));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct frag_main_in {
  float4 coord : SV_Position;
};

struct frag_main_out {
  float depth : SV_Depth;
};

frag_main_out frag_main(frag_main_in tint_in) {
  frag_main_out tint_out;
  tint_out.depth = tint_in.coord.x;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_Uniform) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* coord_var =
      create<ast::Variable>(Source{},                     // source
                            "coord",                      // name
                            ast::StorageClass::kUniform,  // storage_class
                            &vec4,                        // type
                            false,                        // is_const
                            nullptr,                      // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(coord_var);
  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  auto* var = create<ast::Variable>(
      Source{},                      // source
      "v",                           // name
      ast::StorageClass::kFunction,  // storage_class
      &f32,                          // type
      false,                         // is_const
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("coord"), "coord"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("x"),
                                            "x")),  // constructor
      ast::VariableDecorationList{});               // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::VariableDeclStatement>(Source{}, var));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_coord : register(b0) {
  float4 coord;
};

void frag_main() {
  float v = coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_UniformStruct) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>(
      Source{}, "coord", &vec4, ast::StructMemberDecorationList{}));

  auto* str =
      create<ast::Struct>(Source{}, members, ast::StructDecorationList{});

  ast::type::Struct s("Uniforms", str);

  auto* coord_var =
      create<ast::Variable>(Source{},                     // source
                            "uniforms",                   // name
                            ast::StorageClass::kUniform,  // storage_class
                            &s,                           // type
                            false,                        // is_const
                            nullptr,                      // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  mod.AddConstructedType(&s);

  td.RegisterVariableForTesting(coord_var);
  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  auto* var = create<ast::Variable>(
      Source{},                      // source
      "v",                           // name
      ast::StorageClass::kFunction,  // storage_class
      &f32,                          // type
      false,                         // is_const
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::MemberAccessorExpression>(
              Source{},
              create<ast::IdentifierExpression>(
                  Source{}, mod.RegisterSymbol("uniforms"), "uniforms"),
              create<ast::IdentifierExpression>(
                  Source{}, mod.RegisterSymbol("coord"), "coord")),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("x"),
                                            "x")),  // constructor
      ast::VariableDecorationList{});               // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::VariableDeclStatement>(Source{}, var));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Uniforms {
  float4 coord;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

void frag_main() {
  float v = uniforms.coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_RW_StorageBuffer_Read) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str =
      create<ast::Struct>(Source{}, members, ast::StructDecorationList{});

  ast::type::Struct s("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, &s);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "coord",                            // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &ac,                                // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(coord_var);
  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  auto* var = create<ast::Variable>(
      Source{},                      // source
      "v",                           // name
      ast::StorageClass::kFunction,  // storage_class
      &f32,                          // type
      false,                         // is_const
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("coord"), "coord"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),  // constructor
      ast::VariableDecorationList{});               // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::VariableDeclStatement>(Source{}, var));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer coord : register(u0);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_RO_StorageBuffer_Read) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str =
      create<ast::Struct>(Source{}, members, ast::StructDecorationList{});

  ast::type::Struct s("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadOnly, &s);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "coord",                            // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &ac,                                // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(coord_var);
  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  auto* var = create<ast::Variable>(
      Source{},                      // source
      "v",                           // name
      ast::StorageClass::kFunction,  // storage_class
      &f32,                          // type
      false,                         // is_const
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("coord"), "coord"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),  // constructor
      ast::VariableDecorationList{});               // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::VariableDeclStatement>(Source{}, var));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(ByteAddressBuffer coord : register(u0);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_StorageBuffer_Store) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::I32 i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "a", &i32, a_deco));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "b", &f32, b_deco));

  auto* str =
      create<ast::Struct>(Source{}, members, ast::StructDecorationList{});

  ast::type::Struct s("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, &s);

  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "coord",                            // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &ac,                                // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(coord_var);

  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  auto* assign = create<ast::AssignmentStatement>(
      Source{},
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("coord"), "coord"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b")),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::FloatLiteral>(Source{}, &f32, 2.0f)));

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(assign);
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer coord : register(u0);

void frag_main() {
  coord.Store(4, asuint(2.0f));
  return;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  ast::type::Void void_type;
  ast::type::F32 f32;

  auto* foo_var =
      create<ast::Variable>(Source{},                   // source
                            "foo",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &f32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                    // source
                            "bar",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  auto* val_var =
      create<ast::Variable>(Source{},                    // source
                            "val",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);
  td.RegisterVariableForTesting(val_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);
  mod.AddGlobalVariable(val_var);

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "param",                          // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("bar"),
                                        "bar"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("foo"),
                                        "foo")));
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("val"),
                                        "val"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("param"),
                                        "param")));
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::IdentifierExpression>(
                    Source{}, mod.RegisterSymbol("foo"), "foo")));
  auto* sub_func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("sub_func"), "sub_func", params, &f32, body,
      ast::FunctionDecorationList{});

  mod.AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f)));

  body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("bar"),
                                        "bar"),
      create<ast::CallExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("sub_func"), "sub_func"),
          expr)));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func_1 = create<ast::Function>(
      Source{}, mod.RegisterSymbol("ep_1"), "ep_1", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_in {
  float foo : TEXCOORD0;
};

struct ep_1_out {
  float bar : SV_Target1;
  float val : SV_Target0;
};

float sub_func_ep_1(in ep_1_in tint_in, out ep_1_out tint_out, float param) {
  tint_out.bar = tint_in.foo;
  tint_out.val = param;
  return tint_in.foo;
}

ep_1_out ep_1(ep_1_in tint_in) {
  ep_1_out tint_out;
  tint_out.bar = sub_func_ep_1(tint_in, tint_out, 1.0f);
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoints_NoUsedGlobals) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* depth_var = create<ast::Variable>(
      Source{},                    // source
      "depth",                     // name
      ast::StorageClass::kOutput,  // storage_class
      &f32,                        // type
      false,                       // is_const
      nullptr,                     // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}),
      });

  td.RegisterVariableForTesting(depth_var);

  mod.AddGlobalVariable(depth_var);

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "param",                          // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::IdentifierExpression>(
                    Source{}, mod.RegisterSymbol("param"), "param")));
  auto* sub_func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("sub_func"), "sub_func", params, &f32, body,
      ast::FunctionDecorationList{});

  mod.AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f)));

  body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("depth"),
                                        "depth"),
      create<ast::CallExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("sub_func"), "sub_func"),
          expr)));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func_1 = create<ast::Function>(
      Source{}, mod.RegisterSymbol("ep_1"), "ep_1", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func(float param) {
  return param;
}

ep_1_out ep_1() {
  ep_1_out tint_out;
  tint_out.depth = sub_func(1.0f);
  return tint_out;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* coord_var = create<ast::Variable>(
      Source{},                   // source
      "coord",                    // name
      ast::StorageClass::kInput,  // storage_class
      &vec4,                      // type
      false,                      // is_const
      nullptr,                    // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord, Source{}),
      });

  auto* depth_var = create<ast::Variable>(
      Source{},                    // source
      "depth",                     // name
      ast::StorageClass::kOutput,  // storage_class
      &f32,                        // type
      false,                       // is_const
      nullptr,                     // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}),
      });

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod.AddGlobalVariable(coord_var);
  mod.AddGlobalVariable(depth_var);

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "param",                          // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("depth"),
                                        "depth"),
      create<ast::MemberAccessorExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("coord"), "coord"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("x"),
                                            "x"))));
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::IdentifierExpression>(
                    Source{}, mod.RegisterSymbol("param"), "param")));
  auto* sub_func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("sub_func"), "sub_func", params, &f32, body,
      ast::FunctionDecorationList{});

  mod.AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f)));

  body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("depth"),
                                        "depth"),
      create<ast::CallExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("sub_func"), "sub_func"),
          expr)));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func_1 = create<ast::Function>(
      Source{}, mod.RegisterSymbol("ep_1"), "ep_1", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_in {
  float4 coord : SV_Position;
};

struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func_ep_1(in ep_1_in tint_in, out ep_1_out tint_out, float param) {
  tint_out.depth = tint_in.coord.x;
  return param;
}

ep_1_out ep_1(ep_1_in tint_in) {
  ep_1_out tint_out;
  tint_out.depth = sub_func_ep_1(tint_in, tint_out, 1.0f);
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_Uniform) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* coord_var =
      create<ast::Variable>(Source{},                     // source
                            "coord",                      // name
                            ast::StorageClass::kUniform,  // storage_class
                            &vec4,                        // type
                            false,                        // is_const
                            nullptr,                      // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(coord_var);

  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "param",                          // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::MemberAccessorExpression>(
                    Source{},
                    create<ast::IdentifierExpression>(
                        Source{}, mod.RegisterSymbol("coord"), "coord"),
                    create<ast::IdentifierExpression>(
                        Source{}, mod.RegisterSymbol("x"), "x"))));
  auto* sub_func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("sub_func"), "sub_func", params, &f32, body,
      ast::FunctionDecorationList{});

  mod.AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f)));

  auto* var = create<ast::Variable>(
      Source{},                      // source
      "v",                           // name
      ast::StorageClass::kFunction,  // storage_class
      &f32,                          // type
      false,                         // is_const
      create<ast::CallExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("sub_func"), "sub_func"),
          expr),                       // constructor
      ast::VariableDecorationList{});  // decorations

  body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::VariableDeclStatement>(Source{}, var));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_coord : register(b0) {
  float4 coord;
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_StorageBuffer) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, &vec4);
  auto* coord_var =
      create<ast::Variable>(Source{},                           // source
                            "coord",                            // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &ac,                                // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(coord_var);

  mod.AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "param",                          // name
                            ast::StorageClass::kFunction,     // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(
      Source{}, create<ast::MemberAccessorExpression>(
                    Source{},
                    create<ast::IdentifierExpression>(
                        Source{}, mod.RegisterSymbol("coord"), "coord"),
                    create<ast::IdentifierExpression>(
                        Source{}, mod.RegisterSymbol("x"), "x"))));
  auto* sub_func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("sub_func"), "sub_func", params, &f32, body,
      ast::FunctionDecorationList{});

  mod.AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f)));

  auto* var = create<ast::Variable>(
      Source{},                      // source
      "v",                           // name
      ast::StorageClass::kFunction,  // storage_class
      &f32,                          // type
      false,                         // is_const
      create<ast::CallExpression>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, mod.RegisterSymbol("sub_func"), "sub_func"),
          expr),                       // constructor
      ast::VariableDecorationList{});  // decorations

  body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::VariableDeclStatement>(Source{}, var));
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("frag_main"), "frag_main", params,
      &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer coord : register(u0);

float sub_func(float param) {
  return asfloat(coord.Load((4 * 0)));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoints_WithGlobal_Nested_Return) {
  ast::type::Void void_type;
  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* bar_var =
      create<ast::Variable>(Source{},                    // source
                            "bar",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(bar_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::AssignmentStatement>(
      Source{},
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("bar"),
                                        "bar"),
      create<ast::ScalarConstructorExpression>(
          Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.0f))));

  auto* list = create<ast::BlockStatement>(Source{});
  list->append(create<ast::ReturnStatement>(Source{}));

  body->append(create<ast::IfStatement>(
      Source{},
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kEqual,
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 1)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::SintLiteral>(Source{}, &i32, 1))),
      list, ast::ElseStatementList{}));

  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func_1 = create<ast::Function>(
      Source{}, mod.RegisterSymbol("ep_1"), "ep_1", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float bar : SV_Target1;
};

ep_1_out ep_1() {
  ep_1_out tint_out;
  tint_out.bar = 1.0f;
  if ((1 == 1)) {
    return tint_out;
  }
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithNameCollision) {
  ast::type::Void void_type;

  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("GeometryShader"), "GeometryShader",
      ast::VariableList{}, &void_type, create<ast::BlockStatement>(Source{}),
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void GeometryShader_tint_0() {
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_Compute) {
  ast::type::Void void_type;

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_Compute_WithWorkgroup) {
  ast::type::Void void_type;

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
          create<ast::WorkgroupDecoration>(2u, 4u, 6u, Source{}),
      });

  mod.AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 5, ast::ArrayDecorationList{});

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{},                         // source
                            "a",                              // name
                            ast::StorageClass::kNone,         // storage_class
                            &ary,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  ast::type::Void void_type;

  auto* body = create<ast::BlockStatement>(Source{});
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func = create<ast::Function>(Source{}, mod.RegisterSymbol("my_func"),
                                     "my_func", params, &void_type, body,
                                     ast::FunctionDecorationList{});

  mod.AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func(float a[5]) {
    return;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(HlslGeneratorImplTest_Function,
       Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   [[offset(0)]] d : f32;
  // };
  // [[binding(0), set(0)]] var<storage_buffer> data : Data;
  //
  // [[stage(compute)]]
  // fn a() -> void {
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() -> void {
  //   return;
  // }

  ast::type::Void void_type;
  ast::type::F32 f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>(Source{}, "d", &f32, a_deco));

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>(Source{}));

  auto* str = create<ast::Struct>(Source{}, members, s_decos);

  ast::type::Struct s("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, &s);

  auto* data_var =
      create<ast::Variable>(Source{},                           // source
                            "data",                             // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &ac,                                // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::BindingDecoration>(0, Source{}),
                                create<ast::SetDecoration>(0, Source{}),
                            });

  mod.AddConstructedType(&s);
  td.RegisterVariableForTesting(data_var);
  mod.AddGlobalVariable(data_var);

  {
    ast::VariableList params;
    auto* var = create<ast::Variable>(
        Source{},                      // source
        "v",                           // name
        ast::StorageClass::kFunction,  // storage_class
        &f32,                          // type
        false,                         // is_const
        create<ast::MemberAccessorExpression>(
            Source{},
            create<ast::IdentifierExpression>(
                Source{}, mod.RegisterSymbol("data"), "data"),
            create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"),
                                              "d")),  // constructor
        ast::VariableDecorationList{});               // decorations

    auto* body = create<ast::BlockStatement>(Source{});
    body->append(create<ast::VariableDeclStatement>(Source{}, var));
    body->append(create<ast::ReturnStatement>(Source{}));
    auto* func = create<ast::Function>(
        Source{}, mod.RegisterSymbol("a"), "a", params, &void_type, body,
        ast::FunctionDecorationList{
            create<ast::StageDecoration>(ast::PipelineStage::kCompute,
                                         Source{}),
        });

    mod.AddFunction(func);
  }

  {
    ast::VariableList params;
    auto* var = create<ast::Variable>(
        Source{},                      // source
        "v",                           // name
        ast::StorageClass::kFunction,  // storage_class
        &f32,                          // type
        false,                         // is_const
        create<ast::MemberAccessorExpression>(
            Source{},
            create<ast::IdentifierExpression>(
                Source{}, mod.RegisterSymbol("data"), "data"),
            create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"),
                                              "d")),  // constructor
        ast::VariableDecorationList{});               // decorations

    auto* body = create<ast::BlockStatement>(Source{});
    body->append(create<ast::VariableDeclStatement>(Source{}, var));
    body->append(create<ast::ReturnStatement>(Source{}));
    auto* func = create<ast::Function>(
        Source{}, mod.RegisterSymbol("b"), "b", params, &void_type, body,
        ast::FunctionDecorationList{
            create<ast::StageDecoration>(ast::PipelineStage::kCompute,
                                         Source{}),
        });

    mod.AddFunction(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Data {
  float d;
};

RWByteAddressBuffer data : register(u0);

[numthreads(1, 1, 1)]
void a() {
  float v = asfloat(data.Load(0));
  return;
}

[numthreads(1, 1, 1)]
void b() {
  float v = asfloat(data.Load(0));
  return;
}

)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint

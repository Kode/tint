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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/semantic/call.h"
#include "src/semantic/function.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"
#include "src/type/sampled_texture_type.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint {
namespace resolver {
namespace {

TEST_F(ResolverTest, Stmt_Assign) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Case) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      assign,
  });
  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(ty.i32(), 3));
  auto* cse = create<ast::CaseStatement>(lit, body);
  WrapInFunction(cse);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Block) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* block = create<ast::BlockStatement>(ast::StatementList{
      assign,
  });
  WrapInFunction(block);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Else) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      assign,
  });
  auto* cond = Expr(3);
  auto* stmt = create<ast::ElseStatement>(cond, body);
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(StmtOf(cond), stmt);
}

TEST_F(ResolverTest, Stmt_If) {
  auto* else_lhs = Expr(2);
  auto* else_rhs = Expr(2.3f);

  auto* else_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(else_lhs, else_rhs),
  });

  auto* else_cond = Expr(3);
  auto* else_stmt = create<ast::ElseStatement>(else_cond, else_body);

  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* body = create<ast::BlockStatement>(ast::StatementList{assign});
  auto* cond = Expr(true);
  auto* stmt =
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{else_stmt});
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(else_lhs), nullptr);
  ASSERT_NE(TypeOf(else_rhs), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::Bool>());
  EXPECT_TRUE(TypeOf(else_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(else_rhs)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(StmtOf(cond), stmt);
  EXPECT_EQ(StmtOf(else_cond), else_stmt);
}

TEST_F(ResolverTest, Stmt_Loop) {
  auto* body_lhs = Expr(2);
  auto* body_rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(body_lhs, body_rhs),
  });
  auto* continuing_lhs = Expr(2);
  auto* continuing_rhs = Expr(2.3f);

  auto* continuing = create<ast::BlockStatement>(

      ast::StatementList{
          create<ast::AssignmentStatement>(continuing_lhs, continuing_rhs),
      });
  auto* stmt = create<ast::LoopStatement>(body, continuing);
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(body_lhs), nullptr);
  ASSERT_NE(TypeOf(body_rhs), nullptr);
  ASSERT_NE(TypeOf(continuing_lhs), nullptr);
  ASSERT_NE(TypeOf(continuing_rhs), nullptr);
  EXPECT_TRUE(TypeOf(body_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(body_rhs)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(continuing_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(continuing_rhs)->Is<type::F32>());
}

TEST_F(ResolverTest, Stmt_Return) {
  auto* cond = Expr(2);

  auto* ret = create<ast::ReturnStatement>(cond);
  WrapInFunction(ret);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(cond), nullptr);
  EXPECT_TRUE(TypeOf(cond)->Is<type::I32>());
}

TEST_F(ResolverTest, Stmt_Return_WithoutValue) {
  auto* ret = create<ast::ReturnStatement>();
  WrapInFunction(ret);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Switch) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(ty.i32(), 3));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(lit, body));

  auto* stmt = create<ast::SwitchStatement>(Expr(2), cases);
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(ResolverTest, Stmt_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* expr = Call("my_func");

  auto* call = create<ast::CallStatement>(expr);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
  EXPECT_EQ(StmtOf(expr), call);
}

TEST_F(ResolverTest, Stmt_VariableDecl) {
  auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor();

  auto* decl = create<ast::VariableDeclStatement>(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_Alias) {
  auto* my_int = ty.alias("MyInt", ty.i32());
  auto* var = Var("my_var", my_int, ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor();

  auto* decl = create<ast::VariableDeclStatement>(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScope) {
  auto* init = Expr(2);
  Global("my_var", ty.i32(), ast::StorageClass::kNone, init);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
  EXPECT_EQ(StmtOf(init), nullptr);
}

TEST_F(ResolverTest, Stmt_VariableDecl_OuterScopeAfterInnerScope) {
  // fn func_i32() -> i32 {
  //   {
  //     var foo : i32 = 2;
  //     var bar : i32 = foo;
  //   }
  //   var foo : f32 = 2.0;
  //   var bar : f32 = foo;
  // }

  ast::VariableList params;

  // Declare i32 "foo" inside a block
  auto* foo_i32 = Var("foo", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* foo_i32_init = foo_i32->constructor();
  auto* foo_i32_decl = create<ast::VariableDeclStatement>(foo_i32);

  // Reference "foo" inside the block
  auto* bar_i32 = Var("bar", ty.i32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_i32_init = bar_i32->constructor();
  auto* bar_i32_decl = create<ast::VariableDeclStatement>(bar_i32);

  auto* inner = create<ast::BlockStatement>(
      ast::StatementList{foo_i32_decl, bar_i32_decl});

  // Declare f32 "foo" at function scope
  auto* foo_f32 = Var("foo", ty.f32(), ast::StorageClass::kNone, Expr(2.f));
  auto* foo_f32_init = foo_f32->constructor();
  auto* foo_f32_decl = create<ast::VariableDeclStatement>(foo_f32);

  // Reference "foo" at function scope
  auto* bar_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_f32_init = bar_f32->constructor();
  auto* bar_f32_decl = create<ast::VariableDeclStatement>(bar_f32);

  Func("func", params, ty.f32(),
       ast::StatementList{inner, foo_f32_decl, bar_f32_decl},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve());
  ASSERT_NE(TypeOf(foo_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(foo_i32_init)->Is<type::I32>());
  ASSERT_NE(TypeOf(foo_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(foo_f32_init)->Is<type::F32>());
  ASSERT_NE(TypeOf(bar_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(bar_i32_init)->UnwrapAll()->Is<type::I32>());
  ASSERT_NE(TypeOf(bar_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(bar_f32_init)->UnwrapAll()->Is<type::F32>());
  EXPECT_EQ(StmtOf(foo_i32_init), foo_i32_decl);
  EXPECT_EQ(StmtOf(bar_i32_init), bar_i32_decl);
  EXPECT_EQ(StmtOf(foo_f32_init), foo_f32_decl);
  EXPECT_EQ(StmtOf(bar_f32_init), bar_f32_decl);
  EXPECT_TRUE(CheckVarUsers(foo_i32, {bar_i32->constructor()}));
  EXPECT_TRUE(CheckVarUsers(foo_f32, {bar_f32->constructor()}));
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScopeAfterFunctionScope) {
  // fn func_i32() -> i32 {
  //   var foo : i32 = 2;
  // }
  // var foo : f32 = 2.0;
  // fn func_f32() -> f32 {
  //   var bar : f32 = foo;
  // }

  ast::VariableList params;

  // Declare i32 "foo" inside a function
  auto* fn_i32 = Var("foo", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* fn_i32_init = fn_i32->constructor();
  auto* fn_i32_decl = create<ast::VariableDeclStatement>(fn_i32);
  Func("func_i32", params, ty.i32(), ast::StatementList{fn_i32_decl},
       ast::FunctionDecorationList{});

  // Declare f32 "foo" at module scope
  auto* mod_f32 = Var("foo", ty.f32(), ast::StorageClass::kNone, Expr(2.f));
  auto* mod_init = mod_f32->constructor();
  AST().AddGlobalVariable(mod_f32);

  // Reference "foo" in another function
  auto* fn_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* fn_f32_init = fn_f32->constructor();
  auto* fn_f32_decl = create<ast::VariableDeclStatement>(fn_f32);
  Func("func_f32", params, ty.f32(), ast::StatementList{fn_f32_decl},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve());
  ASSERT_NE(TypeOf(mod_init), nullptr);
  EXPECT_TRUE(TypeOf(mod_init)->Is<type::F32>());
  ASSERT_NE(TypeOf(fn_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(fn_i32_init)->Is<type::I32>());
  ASSERT_NE(TypeOf(fn_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(fn_f32_init)->UnwrapAll()->Is<type::F32>());
  EXPECT_EQ(StmtOf(fn_i32_init), fn_i32_decl);
  EXPECT_EQ(StmtOf(mod_init), nullptr);
  EXPECT_EQ(StmtOf(fn_f32_init), fn_f32_decl);
  EXPECT_TRUE(CheckVarUsers(fn_i32, {}));
  EXPECT_TRUE(CheckVarUsers(mod_f32, {fn_f32->constructor()}));
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Array) {
  auto* idx = Expr(2);
  Global("my_var", ty.array<f32, 3>(), ast::StorageClass::kFunction);

  auto* acc = IndexAccessor("my_var", idx);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Alias_Array) {
  auto* aary = ty.alias("myarrty", ty.array<f32, 3>());

  Global("my_var", aary, ast::StorageClass::kFunction);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Array_Constant) {
  GlobalConst("my_var", ty.array<f32, 3>());

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  EXPECT_TRUE(TypeOf(acc)->Is<type::F32>()) << TypeOf(acc)->type_name();
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Matrix) {
  Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<type::Vector>());
  EXPECT_EQ(ptr->type()->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Matrix_BothDimensions) {
  Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* acc = IndexAccessor(IndexAccessor("my_var", 2), 1);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Vector) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Bitcast) {
  auto* bitcast = create<ast::BitcastExpression>(ty.f32(), Expr("name"));
  WrapInFunction(bitcast);

  Global("name", ty.f32(), ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(bitcast), nullptr);
  EXPECT_TRUE(TypeOf(bitcast)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call_InBinaryOp) {
  ast::VariableList params;
  Func("func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* expr = Add(Call("func"), Call("func"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call_WithParams) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* param = Expr(2.4f);

  auto* call = Call("my_func", param);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(param), nullptr);
  EXPECT_TRUE(TypeOf(param)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call_Intrinsic) {
  auto* call = Call("round", 2.4f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Cast) {
  Global("name", ty.f32(), ast::StorageClass::kPrivate);

  auto* cast = Construct(ty.f32(), "name");
  WrapInFunction(cast);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(cast), nullptr);
  EXPECT_TRUE(TypeOf(cast)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Constructor_Scalar) {
  auto* s = Expr(1.0f);
  WrapInFunction(s);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(s), nullptr);
  EXPECT_TRUE(TypeOf(s)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Constructor_Type) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 3.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(tc)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalVariable) {
  auto* my_var = Global("my_var", ty.f32(), ast::StorageClass::kNone);

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(ident)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_TRUE(CheckVarUsers(my_var, {ident}));
}

TEST_F(ResolverTest, Expr_Identifier_GlobalConstant) {
  auto* my_var = GlobalConst("my_var", ty.f32());

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<type::F32>());
  EXPECT_TRUE(CheckVarUsers(my_var, {ident}));
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable_Const) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* var = Const("my_var", ty.f32());
  auto* assign = create<ast::AssignmentStatement>(my_var_a, my_var_b);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           assign,
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  EXPECT_TRUE(TypeOf(my_var_b)->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a, my_var_b}));
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* assign = create<ast::AssignmentStatement>(my_var_a, my_var_b);

  auto* var = Var("my_var", ty.f32(), ast::StorageClass::kNone);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           assign,
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_a)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  EXPECT_TRUE(TypeOf(my_var_b)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_b)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a, my_var_b}));
}

TEST_F(ResolverTest, Expr_Identifier_Function_Ptr) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* assign = create<ast::AssignmentStatement>(my_var_a, my_var_b);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(
               Var("my_var", ty.pointer<f32>(ast::StorageClass::kFunction),
                   ast::StorageClass::kNone)),
           assign,
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_a)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  EXPECT_TRUE(TypeOf(my_var_b)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_b)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
}

TEST_F(ResolverTest, Expr_Call_Function) {
  Func("my_func", ast::VariableList{}, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Identifier_Unknown) {
  auto* a = Expr("a");
  WrapInFunction(a);

  EXPECT_FALSE(r()->Resolve());
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables) {
  auto* in_var = Global("in_var", ty.f32(), ast::StorageClass::kInput);
  auto* out_var = Global("out_var", ty.f32(), ast::StorageClass::kOutput);
  auto* sb_var = Global("sb_var", ty.f32(), ast::StorageClass::kStorage);
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  auto* func = Func(
      "my_func", ast::VariableList{}, ty.f32(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("out_var"), Expr("in_var")),
          create<ast::AssignmentStatement>(Expr("wg_var"), Expr("wg_var")),
          create<ast::AssignmentStatement>(Expr("sb_var"), Expr("sb_var")),
          create<ast::AssignmentStatement>(Expr("priv_var"), Expr("priv_var")),
      },
      ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  const auto& vars = func_sem->ReferencedModuleVariables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0]->Declaration(), out_var);
  EXPECT_EQ(vars[1]->Declaration(), in_var);
  EXPECT_EQ(vars[2]->Declaration(), wg_var);
  EXPECT_EQ(vars[3]->Declaration(), sb_var);
  EXPECT_EQ(vars[4]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables_SubFunction) {
  auto* in_var = Global("in_var", ty.f32(), ast::StorageClass::kInput);
  auto* out_var = Global("out_var", ty.f32(), ast::StorageClass::kOutput);
  auto* sb_var = Global("sb_var", ty.f32(), ast::StorageClass::kStorage);
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("out_var"), Expr("in_var")),
           create<ast::AssignmentStatement>(Expr("wg_var"), Expr("wg_var")),
           create<ast::AssignmentStatement>(Expr("sb_var"), Expr("sb_var")),
           create<ast::AssignmentStatement>(Expr("priv_var"), Expr("priv_var")),
       },
       ast::FunctionDecorationList{});

  auto* func2 = Func(
      "func", ast::VariableList{}, ty.f32(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("out_var"), Call("my_func")),
      },
      ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func2_sem = Sem().Get(func2);
  ASSERT_NE(func2_sem, nullptr);

  const auto& vars = func2_sem->ReferencedModuleVariables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0]->Declaration(), out_var);
  EXPECT_EQ(vars[1]->Declaration(), in_var);
  EXPECT_EQ(vars[2]->Declaration(), wg_var);
  EXPECT_EQ(vars[3]->Declaration(), sb_var);
  EXPECT_EQ(vars[4]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_NotRegisterFunctionVariable) {
  auto* var = Var("in_var", ty.f32(), ast::StorageClass::kFunction);

  auto* func =
      Func("my_func", ast::VariableList{}, ty.f32(),
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::AssignmentStatement>(Expr("var"), Expr(1.f)),
           },
           ast::FunctionDecorationList{});

  Global("var", ty.f32(), ast::StorageClass::kFunction);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->ReferencedModuleVariables().size(), 0u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.i32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("S", strct);
  Global("my_struct", st, ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct_Alias) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.i32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("alias", strct);
  auto* alias = ty.alias("alias", st);
  Global("my_struct", alias, ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_vec", "xzyw");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(mem)->As<type::Vector>()->size(), 4u);
  EXPECT_THAT(Sem().Get(mem)->Swizzle(), ElementsAre(0, 2, 1, 3));
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_vec", "b");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<type::F32>());
  EXPECT_THAT(Sem().Get(mem)->Swizzle(), ElementsAre(2));
}

TEST_F(ResolverTest, Expr_Accessor_MultiLevel) {
  // struct b {
  //   vec4<f32> foo
  // }
  // struct A {
  //   vec3<struct b> mem
  // }
  // var c : A
  // c.mem[0].foo.yx
  //   -> vec2<f32>
  //
  // MemberAccessor{
  //   MemberAccessor{
  //     ArrayAccessor{
  //       MemberAccessor{
  //         Identifier{c}
  //         Identifier{mem}
  //       }
  //       ScalarConstructor{0}
  //     }
  //     Identifier{foo}
  //   }
  //   Identifier{yx}
  // }
  //

  auto* strctB =
      create<ast::Struct>(ast::StructMemberList{Member("foo", ty.vec4<f32>())},
                          ast::StructDecorationList{});
  auto* stB = ty.struct_("B", strctB);

  type::Vector vecB(stB, 3);
  auto* strctA = create<ast::Struct>(
      ast::StructMemberList{Member("mem", &vecB)}, ast::StructDecorationList{});

  auto* stA = ty.struct_("A", strctA);
  Global("c", stA, ast::StorageClass::kNone);

  auto* mem = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("c", "mem"), 0), "foo"),
      "yx");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(mem)->As<type::Vector>()->size(), 2u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_InBinaryOp) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.f32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("S", strct);
  Global("my_struct", st, ast::StorageClass::kNone);

  auto* expr = Add(MemberAccessor("my_struct", "first_member"),
                   MemberAccessor("my_struct", "second_member"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

using Expr_Binary_BitwiseTest = ResolverTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_BitwiseTest, Scalar) {
  auto op = GetParam();

  Global("val", ty.i32(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_P(Expr_Binary_BitwiseTest, Vector) {
  auto op = GetParam();

  Global("val", ty.vec3<i32>(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::I32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_BitwiseTest,
                         testing::Values(ast::BinaryOp::kAnd,
                                         ast::BinaryOp::kOr,
                                         ast::BinaryOp::kXor,
                                         ast::BinaryOp::kShiftLeft,
                                         ast::BinaryOp::kShiftRight,
                                         ast::BinaryOp::kAdd,
                                         ast::BinaryOp::kSubtract,
                                         ast::BinaryOp::kDivide,
                                         ast::BinaryOp::kModulo));

using Expr_Binary_LogicalTest = ResolverTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_LogicalTest, Scalar) {
  auto op = GetParam();

  Global("val", ty.bool_(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Expr_Binary_LogicalTest, Vector) {
  auto op = GetParam();

  Global("val", ty.vec3<bool>(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_LogicalTest,
                         testing::Values(ast::BinaryOp::kLogicalAnd,
                                         ast::BinaryOp::kLogicalOr));

using Expr_Binary_CompareTest = ResolverTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_CompareTest, Scalar) {
  auto op = GetParam();

  Global("val", ty.i32(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Expr_Binary_CompareTest, Vector) {
  auto op = GetParam();

  Global("val", ty.vec3<i32>(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_CompareTest,
                         testing::Values(ast::BinaryOp::kEqual,
                                         ast::BinaryOp::kNotEqual,
                                         ast::BinaryOp::kLessThan,
                                         ast::BinaryOp::kGreaterThan,
                                         ast::BinaryOp::kLessThanEqual,
                                         ast::BinaryOp::kGreaterThanEqual));

TEST_F(ResolverTest, Expr_Binary_Multiply_Scalar_Scalar) {
  Global("val", ty.i32(), ast::StorageClass::kNone);

  auto* expr = Mul("val", "val");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Vector_Scalar) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("vector", "scalar");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Scalar_Vector) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("scalar", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Vector_Vector) {
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("vector", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Matrix_Scalar) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("matrix", "scalar");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Scalar_Matrix) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("scalar", "matrix");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Matrix_Vector) {
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("matrix", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Vector_Matrix) {
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("vector", "matrix");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 2u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Matrix_Matrix) {
  Global("mat3x4", ty.mat3x4<f32>(), ast::StorageClass::kNone);
  Global("mat4x3", ty.mat4x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("mat3x4", "mat4x3");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 4u);
  EXPECT_EQ(mat->columns(), 4u);
}

using UnaryOpExpressionTest = ResolverTestWithParam<ast::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
  auto op = GetParam();

  Global("ident", ty.vec4<f32>(), ast::StorageClass::kNone);
  auto* der = create<ast::UnaryOpExpression>(op, Expr("ident"));
  WrapInFunction(der);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(der), nullptr);
  ASSERT_TRUE(TypeOf(der)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(der)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(der)->As<type::Vector>()->size(), 4u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(ResolverTest, StorageClass_SetsIfMissing) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kNone);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kFunction);
}

TEST_F(ResolverTest, StorageClass_DoesNotSetOnConst) {
  auto* var = Const("var", ty.i32());
  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kNone);
}

TEST_F(ResolverTest, Function_EntryPoints_StageDecoration) {
  // fn b() {}
  // fn c() { b(); }
  // fn a() { c(); }
  // fn ep_1() { a(); b(); }
  // fn ep_2() { c();}
  //
  // c -> {ep_1, ep_2}
  // a -> {ep_1}
  // b -> {ep_1, ep_2}
  // ep_1 -> {}
  // ep_2 -> {}

  ast::VariableList params;
  auto* func_b = Func("b", params, ty.f32(), ast::StatementList{},
                      ast::FunctionDecorationList{});
  auto* func_c =
      Func("c", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("second"), Call("b")),
           },
           ast::FunctionDecorationList{});

  auto* func_a =
      Func("a", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("first"), Call("c")),
           },
           ast::FunctionDecorationList{});

  auto* ep_1 =
      Func("ep_1", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("call_a"), Call("a")),
               create<ast::AssignmentStatement>(Expr("call_b"), Call("b")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  auto* ep_2 =
      Func("ep_2", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("call_c"), Call("c")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  Global("first", ty.f32(), ast::StorageClass::kPrivate);
  Global("second", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_a", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_b", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_c", ty.f32(), ast::StorageClass::kPrivate);

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* func_b_sem = Sem().Get(func_b);
  auto* func_a_sem = Sem().Get(func_a);
  auto* func_c_sem = Sem().Get(func_c);
  auto* ep_1_sem = Sem().Get(ep_1);
  auto* ep_2_sem = Sem().Get(ep_2);
  ASSERT_NE(func_b_sem, nullptr);
  ASSERT_NE(func_a_sem, nullptr);
  ASSERT_NE(func_c_sem, nullptr);
  ASSERT_NE(ep_1_sem, nullptr);
  ASSERT_NE(ep_2_sem, nullptr);

  const auto& b_eps = func_b_sem->AncestorEntryPoints();
  ASSERT_EQ(2u, b_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), b_eps[0]);
  EXPECT_EQ(Symbols().Register("ep_2"), b_eps[1]);

  const auto& a_eps = func_a_sem->AncestorEntryPoints();
  ASSERT_EQ(1u, a_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), a_eps[0]);

  const auto& c_eps = func_c_sem->AncestorEntryPoints();
  ASSERT_EQ(2u, c_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), c_eps[0]);
  EXPECT_EQ(Symbols().Register("ep_2"), c_eps[1]);

  EXPECT_TRUE(ep_1_sem->AncestorEntryPoints().empty());
  EXPECT_TRUE(ep_2_sem->AncestorEntryPoints().empty());
}

// Check for linear-time traversal of functions reachable from entry points.
// See: crbug.com/tint/245
TEST_F(ResolverTest, Function_EntryPoints_LinearTime) {
  // fn lNa() { }
  // fn lNb() { }
  // ...
  // fn l2a() { l3a(); l3b(); }
  // fn l2b() { l3a(); l3b(); }
  // fn l1a() { l2a(); l2b(); }
  // fn l1b() { l2a(); l2b(); }
  // fn main() { l1a(); l1b(); }

  static constexpr int levels = 64;

  auto fn_a = [](int level) { return "l" + std::to_string(level + 1) + "a"; };
  auto fn_b = [](int level) { return "l" + std::to_string(level + 1) + "b"; };

  Func(fn_a(levels), {}, ty.void_(), {}, {});
  Func(fn_b(levels), {}, ty.void_(), {}, {});

  for (int i = levels - 1; i >= 0; i--) {
    Func(fn_a(i), {}, ty.void_(),
         {
             create<ast::CallStatement>(Call(fn_a(i + 1))),
             create<ast::CallStatement>(Call(fn_b(i + 1))),
         },
         {});
    Func(fn_b(i), {}, ty.void_(),
         {
             create<ast::CallStatement>(Call(fn_a(i + 1))),
             create<ast::CallStatement>(Call(fn_b(i + 1))),
         },
         {});
  }

  Func("main", {}, ty.void_(),
       {
           create<ast::CallStatement>(Call(fn_a(0))),
           create<ast::CallStatement>(Call(fn_b(0))),
       },
       {
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace resolver
}  // namespace tint
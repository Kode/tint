// Copyright 2023 The Tint Authors.
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

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

enum class Def {
    kAccess,
    kAddressSpace,
    kBuiltinFunction,
    kBuiltinType,
    kFunction,
    kStruct,
    kTexelFormat,
    kTypeAlias,
    kVariable,
};

std::ostream& operator<<(std::ostream& out, Def def) {
    switch (def) {
        case Def::kAccess:
            return out << "Def::kAccess";
        case Def::kAddressSpace:
            return out << "Def::kAddressSpace";
        case Def::kBuiltinFunction:
            return out << "Def::kBuiltinFunction";
        case Def::kBuiltinType:
            return out << "Def::kBuiltinType";
        case Def::kFunction:
            return out << "Def::kFunction";
        case Def::kStruct:
            return out << "Def::kStruct";
        case Def::kTexelFormat:
            return out << "Def::kTexelFormat";
        case Def::kTypeAlias:
            return out << "Def::kTypeAlias";
        case Def::kVariable:
            return out << "Def::kVariable";
    }
    return out << "<unknown>";
}

enum class Use {
    kAccess,
    kAddressSpace,
    kBinaryOp,
    kCallExpr,
    kCallStmt,
    kFunctionReturnType,
    kMemberType,
    kTexelFormat,
    kValueExpression,
    kVariableType,
    kUnaryOp
};

std::ostream& operator<<(std::ostream& out, Use use) {
    switch (use) {
        case Use::kAccess:
            return out << "Use::kAccess";
        case Use::kAddressSpace:
            return out << "Use::kAddressSpace";
        case Use::kBinaryOp:
            return out << "Use::kBinaryOp";
        case Use::kCallExpr:
            return out << "Use::kCallExpr";
        case Use::kCallStmt:
            return out << "Use::kCallStmt";
        case Use::kFunctionReturnType:
            return out << "Use::kFunctionReturnType";
        case Use::kMemberType:
            return out << "Use::kMemberType";
        case Use::kTexelFormat:
            return out << "Use::kTexelFormat";
        case Use::kValueExpression:
            return out << "Use::kValueExpression";
        case Use::kVariableType:
            return out << "Use::kVariableType";
        case Use::kUnaryOp:
            return out << "Use::kUnaryOp";
    }
    return out << "<unknown>";
}

struct Case {
    Def def;
    Use use;
    const char* error;
};

std::ostream& operator<<(std::ostream& out, Case c) {
    return out << "{" << c.def << ", " << c.use << "}";
}

static const char* kPass = "<pass>";

static const Source kDefSource{Source::Range{{1, 2}, {3, 4}}};
static const Source kUseSource{Source::Range{{5, 6}, {7, 8}}};

using ResolverExpressionKindTest = ResolverTestWithParam<Case>;

TEST_P(ResolverExpressionKindTest, Test) {
    Symbol sym;
    std::function<void(const sem::Expression*)> check_expr;
    switch (GetParam().def) {
        case Def::kAccess: {
            sym = Sym("write");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<type::Access>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), type::Access::kWrite);
            };
            break;
        }
        case Def::kAddressSpace: {
            sym = Sym("workgroup");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<type::AddressSpace>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), type::AddressSpace::kWorkgroup);
            };
            break;
        }
        case Def::kBuiltinFunction: {
            sym = Sym("workgroupBarrier");
            check_expr = [](const sem::Expression* expr) { EXPECT_EQ(expr, nullptr); };
            break;
        }
        case Def::kBuiltinType: {
            sym = Sym("vec4f");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                EXPECT_TRUE(ty_expr->Type()->Is<type::Vector>());
            };
            break;
        }
        case Def::kFunction: {
            auto* fn = Func(kDefSource, "FUNCTION", utils::Empty, ty.i32(), Return(1_i));
            sym = Sym("FUNCTION");
            check_expr = [fn](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* fn_expr = expr->As<sem::FunctionExpression>();
                ASSERT_NE(fn_expr, nullptr);
                EXPECT_EQ(fn_expr->Function()->Declaration(), fn);
            };
            break;
        }
        case Def::kStruct: {
            auto* s = Structure(kDefSource, "STRUCT", utils::Vector{Member("m", ty.i32())});
            sym = Sym("STRUCT");
            check_expr = [s](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                auto* got = ty_expr->Type()->As<sem::Struct>();
                ASSERT_NE(got, nullptr);
                EXPECT_EQ(got->Declaration(), s);
            };
            break;
        }
        case Def::kTexelFormat: {
            sym = Sym("rgba8unorm");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* enum_expr = expr->As<sem::BuiltinEnumExpression<type::TexelFormat>>();
                ASSERT_NE(enum_expr, nullptr);
                EXPECT_EQ(enum_expr->Value(), type::TexelFormat::kRgba8Unorm);
            };
            break;
        }
        case Def::kTypeAlias: {
            Alias(kDefSource, "ALIAS", ty.i32());
            sym = Sym("ALIAS");
            check_expr = [](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* ty_expr = expr->As<sem::TypeExpression>();
                ASSERT_NE(ty_expr, nullptr);
                EXPECT_TRUE(ty_expr->Type()->Is<type::I32>());
            };
            break;
        }
        case Def::kVariable: {
            auto* c = GlobalConst(kDefSource, "VARIABLE", Expr(1_i));
            sym = Sym("VARIABLE");
            check_expr = [c](const sem::Expression* expr) {
                ASSERT_NE(expr, nullptr);
                auto* var_expr = expr->As<sem::VariableUser>();
                ASSERT_NE(var_expr, nullptr);
                EXPECT_EQ(var_expr->Variable()->Declaration(), c);
            };
            break;
        }
    }

    auto* expr = Expr(Ident(kUseSource, sym));
    switch (GetParam().use) {
        case Use::kAccess:
            GlobalVar("v", ty("texture_storage_2d", "rgba8unorm", expr), Group(0_u), Binding(0_u));
            break;
        case Use::kAddressSpace:
            Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
            Func("f", utils::Vector{Param("p", ty("ptr", expr, ty.f32()))}, ty.void_(),
                 utils::Empty);
            break;
        case Use::kCallExpr:
            Func("f", utils::Empty, ty.void_(), Decl(Var("v", Call(expr))));
            break;
        case Use::kCallStmt:
            Func("f", utils::Empty, ty.void_(), CallStmt(Call(expr)));
            break;
        case Use::kBinaryOp:
            GlobalVar("v", type::AddressSpace::kPrivate, Mul(1_a, expr));
            break;
        case Use::kFunctionReturnType:
            Func("f", utils::Empty, ty(expr), Return(Call(sym)));
            break;
        case Use::kMemberType:
            Structure("s", utils::Vector{Member("m", ty(expr))});
            break;
        case Use::kTexelFormat:
            GlobalVar("v", ty("texture_storage_2d", ty(expr), "write"), Group(0_u), Binding(0_u));
            break;
        case Use::kValueExpression:
            GlobalVar("v", type::AddressSpace::kPrivate, expr);
            break;
        case Use::kVariableType:
            GlobalVar("v", type::AddressSpace::kPrivate, ty(expr));
            break;
        case Use::kUnaryOp:
            GlobalVar("v", type::AddressSpace::kPrivate, Negation(expr));
            break;
    }

    if (GetParam().error == kPass) {
        EXPECT_TRUE(r()->Resolve());
        EXPECT_EQ(r()->error(), "");
        check_expr(Sem().Get(expr));
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), GetParam().error);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    ResolverExpressionKindTest,
    testing::ValuesIn(std::vector<Case>{
        {Def::kAccess, Use::kAccess, kPass},
        {Def::kAccess, Use::kAddressSpace,
         R"(5:6 error: cannot use access 'write' as address space)"},
        {Def::kAccess, Use::kBinaryOp, R"(5:6 error: cannot use access 'write' as value)"},
        {Def::kAccess, Use::kCallExpr, R"(5:6 error: cannot use access 'write' as call target)"},
        {Def::kAccess, Use::kCallStmt, R"(5:6 error: cannot use access 'write' as call target)"},
        {Def::kAccess, Use::kFunctionReturnType, R"(5:6 error: cannot use access 'write' as type)"},
        {Def::kAccess, Use::kMemberType, R"(5:6 error: cannot use access 'write' as type)"},
        {Def::kAccess, Use::kTexelFormat,
         R"(5:6 error: cannot use access 'write' as texel format)"},
        {Def::kAccess, Use::kValueExpression, R"(5:6 error: cannot use access 'write' as value)"},
        {Def::kAccess, Use::kVariableType, R"(5:6 error: cannot use access 'write' as type)"},
        {Def::kAccess, Use::kUnaryOp, R"(5:6 error: cannot use access 'write' as value)"},

        {Def::kAddressSpace, Use::kAccess,
         R"(5:6 error: cannot use address space 'workgroup' as access)"},
        {Def::kAddressSpace, Use::kAddressSpace, kPass},
        {Def::kAddressSpace, Use::kBinaryOp,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kCallExpr,
         R"(5:6 error: cannot use address space 'workgroup' as call target)"},
        {Def::kAddressSpace, Use::kCallStmt,
         R"(5:6 error: cannot use address space 'workgroup' as call target)"},
        {Def::kAddressSpace, Use::kFunctionReturnType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kMemberType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kTexelFormat,
         R"(5:6 error: cannot use address space 'workgroup' as texel format)"},
        {Def::kAddressSpace, Use::kValueExpression,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},
        {Def::kAddressSpace, Use::kVariableType,
         R"(5:6 error: cannot use address space 'workgroup' as type)"},
        {Def::kAddressSpace, Use::kUnaryOp,
         R"(5:6 error: cannot use address space 'workgroup' as value)"},

        {Def::kBuiltinFunction, Use::kAccess,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kAddressSpace,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kBinaryOp,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kCallStmt, kPass},
        {Def::kBuiltinFunction, Use::kFunctionReturnType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kMemberType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kTexelFormat,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kValueExpression,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kVariableType,
         R"(7:8 error: missing '(' for builtin function call)"},
        {Def::kBuiltinFunction, Use::kUnaryOp,
         R"(7:8 error: missing '(' for builtin function call)"},

        {Def::kBuiltinType, Use::kAccess, R"(5:6 error: cannot use type 'vec4<f32>' as access)"},
        {Def::kBuiltinType, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'vec4<f32>' as address space)"},
        {Def::kBuiltinType, Use::kBinaryOp,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for type initializer?)"},
        {Def::kBuiltinType, Use::kCallExpr, kPass},
        {Def::kBuiltinType, Use::kFunctionReturnType, kPass},
        {Def::kBuiltinType, Use::kMemberType, kPass},
        {Def::kBuiltinType, Use::kTexelFormat,
         R"(5:6 error: cannot use type 'vec4<f32>' as texel format)"},
        {Def::kBuiltinType, Use::kValueExpression,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for type initializer?)"},
        {Def::kBuiltinType, Use::kVariableType, kPass},
        {Def::kBuiltinType, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'vec4<f32>' as value
7:8 note: are you missing '()' for type initializer?)"},

        {Def::kFunction, Use::kAccess, R"(5:6 error: cannot use function 'FUNCTION' as access
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kAddressSpace,
         R"(5:6 error: cannot use function 'FUNCTION' as address space
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kBinaryOp, R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kCallExpr, kPass},
        {Def::kFunction, Use::kCallStmt, kPass},
        {Def::kFunction, Use::kFunctionReturnType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kMemberType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kTexelFormat,
         R"(5:6 error: cannot use function 'FUNCTION' as texel format
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kValueExpression,
         R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kVariableType,
         R"(5:6 error: cannot use function 'FUNCTION' as type
1:2 note: function 'FUNCTION' declared here)"},
        {Def::kFunction, Use::kUnaryOp, R"(5:6 error: cannot use function 'FUNCTION' as value
1:2 note: function 'FUNCTION' declared here)"},

        {Def::kStruct, Use::kAccess, R"(5:6 error: cannot use type 'STRUCT' as access)"},
        {Def::kStruct, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'STRUCT' as address space)"},
        {Def::kStruct, Use::kBinaryOp, R"(5:6 error: cannot use type 'STRUCT' as value
7:8 note: are you missing '()' for type initializer?
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kFunctionReturnType, kPass},
        {Def::kStruct, Use::kMemberType, kPass},
        {Def::kStruct, Use::kTexelFormat, R"(5:6 error: cannot use type 'STRUCT' as texel format)"},
        {Def::kStruct, Use::kValueExpression,
         R"(5:6 error: cannot use type 'STRUCT' as value
7:8 note: are you missing '()' for type initializer?
1:2 note: struct 'STRUCT' declared here)"},
        {Def::kStruct, Use::kVariableType, kPass},
        {Def::kStruct, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'STRUCT' as value
7:8 note: are you missing '()' for type initializer?
1:2 note: struct 'STRUCT' declared here)"},

        {Def::kTexelFormat, Use::kAccess,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as access)"},
        {Def::kTexelFormat, Use::kAddressSpace,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as address space)"},
        {Def::kTexelFormat, Use::kBinaryOp,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kCallExpr,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as call target)"},
        {Def::kTexelFormat, Use::kCallStmt,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as call target)"},
        {Def::kTexelFormat, Use::kFunctionReturnType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kMemberType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kTexelFormat, kPass},
        {Def::kTexelFormat, Use::kValueExpression,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},
        {Def::kTexelFormat, Use::kVariableType,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as type)"},
        {Def::kTexelFormat, Use::kUnaryOp,
         R"(5:6 error: cannot use texel format 'rgba8unorm' as value)"},

        {Def::kTypeAlias, Use::kAccess, R"(5:6 error: cannot use type 'i32' as access)"},
        {Def::kTypeAlias, Use::kAddressSpace,
         R"(5:6 error: cannot use type 'i32' as address space)"},
        {Def::kTypeAlias, Use::kBinaryOp,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for type initializer?)"},
        {Def::kTypeAlias, Use::kCallExpr, kPass},
        {Def::kTypeAlias, Use::kFunctionReturnType, kPass},
        {Def::kTypeAlias, Use::kMemberType, kPass},
        {Def::kTypeAlias, Use::kTexelFormat, R"(5:6 error: cannot use type 'i32' as texel format)"},
        {Def::kTypeAlias, Use::kValueExpression,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for type initializer?)"},
        {Def::kTypeAlias, Use::kVariableType, kPass},
        {Def::kTypeAlias, Use::kUnaryOp,
         R"(5:6 error: cannot use type 'i32' as value
7:8 note: are you missing '()' for type initializer?)"},

        {Def::kVariable, Use::kAccess, R"(5:6 error: cannot use const 'VARIABLE' as access
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kAddressSpace,
         R"(5:6 error: cannot use const 'VARIABLE' as address space
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kBinaryOp, kPass},
        {Def::kVariable, Use::kCallStmt,
         R"(5:6 error: cannot use const 'VARIABLE' as call target
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kCallExpr,
         R"(5:6 error: cannot use const 'VARIABLE' as call target
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kFunctionReturnType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kMemberType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kTexelFormat,
         R"(5:6 error: cannot use const 'VARIABLE' as texel format
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kValueExpression, kPass},
        {Def::kVariable, Use::kVariableType,
         R"(5:6 error: cannot use const 'VARIABLE' as type
1:2 note: const 'VARIABLE' declared here)"},
        {Def::kVariable, Use::kUnaryOp, kPass},
    }));

}  // namespace
}  // namespace tint::resolver
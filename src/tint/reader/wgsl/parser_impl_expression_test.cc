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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, Expression_InvalidLHS) {
    auto p = parser("if (a) {} || true");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(e.value, nullptr);
}

TEST_F(ParserImplTest, Expression_Or_Parses) {
    auto p = parser("a || true");
    auto e = p->maybe_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 3u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 5u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kLogicalOr, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, Expression_Or_Parses_Multiple) {
    auto p = parser("a || true || b");
    auto e = p->maybe_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    // lhs: (a || true)
    // rhs: b
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kLogicalOr, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("b"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());
    // lhs: a
    // rhs: true
    rel = rel->lhs->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kLogicalOr, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, Expression_Or_InvalidRHS) {
    auto p = parser("true || if (a) {}");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: unable to parse right side of || expression");
}

TEST_F(ParserImplTest, Expression_And_Parses) {
    auto p = parser("a && true");
    auto e = p->maybe_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 3u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 5u);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kLogicalAnd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, Expression_And_Parses_Multple) {
    auto p = parser("a && true && b");
    auto e = p->maybe_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    // lhs: (a && true)
    // rhs: b
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kLogicalAnd, rel->op);

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    auto* ident = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("b"));

    ASSERT_TRUE(rel->lhs->Is<ast::BinaryExpression>());
    // lhs: a
    // rhs: true
    rel = rel->lhs->As<ast::BinaryExpression>();
    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    ident = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::BoolLiteralExpression>());
    ASSERT_TRUE(rel->rhs->As<ast::BoolLiteralExpression>()->value);
}

TEST_F(ParserImplTest, Expression_And_InvalidRHS) {
    auto p = parser("true && if (a) {}");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: unable to parse right side of && expression");
}

TEST_F(ParserImplTest, Expression_Mixing_OrWithAnd) {
    auto p = parser("a && true || b");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: mixing '&&' and '||' requires parenthesis");
}

TEST_F(ParserImplTest, Expression_Mixing_AndWithOr) {
    auto p = parser("a || true && b");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: mixing '||' and '&&' requires parenthesis");
}

TEST_F(ParserImplTest, Expression_Bitwise) {
    auto p = parser("a & b");
    auto e = p->maybe_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kAnd, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, Expression_Relational) {
    auto p = parser("a <= b");
    auto e = p->maybe_expression();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(e.value, nullptr);

    ASSERT_TRUE(e->Is<ast::BinaryExpression>());
    auto* rel = e->As<ast::BinaryExpression>();
    EXPECT_EQ(ast::BinaryOp::kLessThanEqual, rel->op);

    ASSERT_TRUE(rel->lhs->Is<ast::IdentifierExpression>());
    auto* ident = rel->lhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("a"));

    ASSERT_TRUE(rel->rhs->Is<ast::IdentifierExpression>());
    ident = rel->rhs->As<ast::IdentifierExpression>();
    EXPECT_EQ(ident->symbol, p->builder().Symbols().Get("b"));
}

TEST_F(ParserImplTest, Expression_InvalidUnary) {
    auto p = parser("!if || true");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:2: unable to parse right side of ! expression");
}

TEST_F(ParserImplTest, Expression_InvalidBitwise) {
    auto p = parser("a & if");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: unable to parse right side of & expression");
}

TEST_F(ParserImplTest, Expression_InvalidRelational) {
    auto p = parser("a <= if");
    auto e = p->maybe_expression();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: unable to parse right side of <= expression");
}

}  // namespace
}  // namespace tint::reader::wgsl

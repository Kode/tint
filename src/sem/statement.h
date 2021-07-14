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

#ifndef SRC_SEM_STATEMENT_H_
#define SRC_SEM_STATEMENT_H_

#include "src/sem/node.h"

// Forward declarations
namespace tint {
namespace ast {
class Function;
class Statement;
}  // namespace ast
namespace sem {
class BlockStatement;
}  // namespace sem
}  // namespace tint

namespace tint {
namespace sem {

/// Forward declaration
class CompoundStatement;

/// Statement holds the semantic information for a statement.
class Statement : public Castable<Statement, Node> {
 public:
  /// Constructor
  /// @param declaration the AST node for this statement
  /// @param parent the owning statement
  Statement(const ast::Statement* declaration, const CompoundStatement* parent);

  /// @return the AST node for this statement
  const ast::Statement* Declaration() const { return declaration_; }

  /// @return the statement that encloses this statement
  const CompoundStatement* Parent() const { return parent_; }

  /// @returns the closest enclosing parent that satisfies the given predicate,
  /// which may be the statement itself, or nullptr if no match is found
  /// @param pred a predicate that the resulting block must satisfy
  template <typename Pred>
  const CompoundStatement* FindFirstParent(Pred&& pred) const;

  /// @returns the statement itself if it matches the template type `T`,
  /// otherwise the nearest enclosing statement that matches `T`, or nullptr if
  /// there is none.
  template <typename T>
  const T* FindFirstParent() const;

  /// @return the closest enclosing block for this statement
  const BlockStatement* Block() const;

  /// @returns the function that owns this statement
  const ast::Function* Function() const;

 private:
  ast::Statement const* const declaration_;
  CompoundStatement const* const parent_;
};

/// CompoundStatement is the base class of statements that can hold other
/// statements.
class CompoundStatement : public Castable<Statement, Statement> {
 public:
  /// Constructor
  /// @param declaration the AST node for this statement
  /// @param parent the owning statement
  CompoundStatement(const ast::Statement* declaration,
                    const CompoundStatement* parent);

  /// Destructor
  ~CompoundStatement() override;
};

template <typename Pred>
const CompoundStatement* Statement::FindFirstParent(Pred&& pred) const {
  if (auto* self = As<CompoundStatement>()) {
    if (pred(self)) {
      return self;
    }
  }
  const auto* curr = parent_;
  while (curr && !pred(curr)) {
    curr = curr->Parent();
  }
  return curr;
}

template <typename T>
const T* Statement::FindFirstParent() const {
  if (auto* p = As<T>()) {
    return p;
  }
  const auto* curr = parent_;
  while (curr) {
    if (auto* p = curr->As<T>()) {
      return p;
    }
    curr = curr->Parent();
  }
  return nullptr;
}

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_STATEMENT_H_

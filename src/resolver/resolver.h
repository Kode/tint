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

#ifndef SRC_RESOLVER_RESOLVER_H_
#define SRC_RESOLVER_RESOLVER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/intrinsic_table.h"
#include "src/program_builder.h"
#include "src/scope_stack.h"
#include "src/utils/unique_vector.h"

namespace tint {

// Forward declarations
namespace ast {
class ArrayAccessorExpression;
class BinaryExpression;
class BitcastExpression;
class CallExpression;
class CaseStatement;
class ConstructorExpression;
class Function;
class IdentifierExpression;
class MemberAccessorExpression;
class UnaryOpExpression;
class Variable;
}  // namespace ast
namespace semantic {
class Statement;
}  // namespace semantic

namespace resolver {

/// Resolves types for all items in the given tint program
class Resolver {
 public:
  /// Constructor
  /// @param builder the program builder
  explicit Resolver(ProgramBuilder* builder);

  /// Destructor
  ~Resolver();

  /// @returns error messages from the resolver
  std::string error() const { return diagnostics_.str(); }

  /// @returns true if the resolver was successful
  bool Resolve();

 private:
  /// Structure holding semantic information about a variable.
  /// Used to build the semantic::Variable nodes at the end of resolving.
  struct VariableInfo {
    explicit VariableInfo(ast::Variable* decl);
    ~VariableInfo();

    ast::Variable* const declaration;
    ast::StorageClass storage_class;
    std::vector<ast::IdentifierExpression*> users;
  };

  /// Structure holding semantic information about a function.
  /// Used to build the semantic::Function nodes at the end of resolving.
  struct FunctionInfo {
    explicit FunctionInfo(ast::Function* decl);
    ~FunctionInfo();

    ast::Function* const declaration;
    UniqueVector<VariableInfo*> referenced_module_vars;
    UniqueVector<VariableInfo*> local_referenced_module_vars;

    // List of transitive calls this function makes
    UniqueVector<FunctionInfo*> transitive_calls;
  };

  /// Structure holding semantic information about an expression.
  /// Used to build the semantic::Expression nodes at the end of resolving.
  struct ExpressionInfo {
    type::Type* type;
    semantic::Statement* statement;
  };

  /// Structure holding semantic information about a call expression to an
  /// ast::Function.
  /// Used to build the semantic::Call nodes at the end of resolving.
  struct FunctionCallInfo {
    FunctionInfo* function;
    semantic::Statement* statement;
  };

  /// Structure holding semantic information about a block (i.e. scope), such as
  /// parent block and variables declared in the block.
  /// Used to validate variable scoping rules.
  struct BlockInfo {
    enum class Type { kGeneric, kLoop, kLoopContinuing, kSwitchCase };

    BlockInfo(Type type, BlockInfo* parent);
    ~BlockInfo();

    template <typename Pred>
    BlockInfo* FindFirstParent(Pred&& pred) {
      BlockInfo* curr = this;
      while (curr && !pred(curr)) {
        curr = curr->parent;
      }
      return curr;
    }

    BlockInfo* FindFirstParent(BlockInfo::Type ty) {
      return FindFirstParent(
          [ty](auto* block_info) { return block_info->type == ty; });
    }

    Type const type;
    BlockInfo* const parent;
    std::vector<const ast::Variable*> decls;

    // first_continue is set to the index of the first variable in decls
    // declared after the first continue statement in a loop block, if any.
    constexpr static size_t kNoContinue = size_t(~0);
    size_t first_continue = kNoContinue;
  };

  /// Resolves the program, without creating final the semantic nodes.
  /// @returns true on success, false on error
  bool ResolveInternal();

  /// Resolves functions
  /// @param funcs the functions to check
  /// @returns true on success, false on error
  bool Functions(const ast::FunctionList& funcs);
  /// Resolves a function. Requires all dependency
  /// (callee) functions to have DetermineFunction() called on them first.
  /// @param func the function to check
  /// @returns true on success, false on error
  bool Function(ast::Function* func);
  /// Resolves a block statement
  /// @param stmt the block statement
  /// @returns true if determination was successful
  bool BlockStatement(const ast::BlockStatement* stmt);
  /// Resolves the list of statements
  /// @param stmts the statements to resolve
  /// @returns true on success, false on error
  bool Statements(const ast::StatementList& stmts);
  /// Resolves a statement
  /// @param stmt the statement to check
  /// @returns true on success, false on error
  bool Statement(ast::Statement* stmt);
  /// Resolves an expression list
  /// @param list the expression list to check
  /// @returns true on success, false on error
  bool Expressions(const ast::ExpressionList& list);
  /// Resolves an expression
  /// @param expr the expression to check
  /// @returns true on success, false on error
  bool Expression(ast::Expression* expr);
  /// Resolves the storage class for variables. This assumes that it is only
  /// called for things in function scope, not module scope.
  /// @param stmt the statement to check
  /// @returns false on error
  bool VariableStorageClass(ast::Statement* stmt);

  /// Creates the nodes and adds them to the semantic::Info mappings of the
  /// ProgramBuilder.
  void CreateSemanticNodes() const;

  /// Retrieves information for the requested import.
  /// @param src the source of the import
  /// @param path the import path
  /// @param name the method name to get information on
  /// @param params the parameters to the method call
  /// @param id out parameter for the external call ID. Must not be a nullptr.
  /// @returns the return type of `name` in `path` or nullptr on error.
  type::Type* GetImportData(const Source& src,
                            const std::string& path,
                            const std::string& name,
                            const ast::ExpressionList& params,
                            uint32_t* id);

  void set_referenced_from_function_if_needed(VariableInfo* var, bool local);

  bool ArrayAccessor(ast::ArrayAccessorExpression* expr);
  bool Binary(ast::BinaryExpression* expr);
  bool Bitcast(ast::BitcastExpression* expr);
  bool Call(ast::CallExpression* expr);
  bool CaseStatement(ast::CaseStatement* stmt);
  bool Constructor(ast::ConstructorExpression* expr);
  bool Identifier(ast::IdentifierExpression* expr);
  bool IfStatement(ast::IfStatement* stmt);
  bool IntrinsicCall(ast::CallExpression* call,
                     semantic::IntrinsicType intrinsic_type);
  bool MemberAccessor(ast::MemberAccessorExpression* expr);
  bool UnaryOp(ast::UnaryOpExpression* expr);

  bool VariableDeclStatement(const ast::VariableDeclStatement* stmt);

  VariableInfo* CreateVariableInfo(ast::Variable*);

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  type::Type* TypeOf(ast::Expression* expr);

  /// Creates a semantic::Expression node with the resolved type `type`, and
  /// assigns this semantic node to the expression `expr`.
  /// @param expr the expression
  /// @param type the resolved type
  void SetType(ast::Expression* expr, type::Type* type);

  /// Constructs a new BlockInfo with the given type and with #current_block_ as
  /// its parent, assigns this to #current_block_, and then calls `callback`.
  /// The original #current_block_ is restored on exit.
  template <typename F>
  bool BlockScope(BlockInfo::Type type, F&& callback);

  ProgramBuilder* const builder_;
  std::unique_ptr<IntrinsicTable> const intrinsic_table_;
  diag::List diagnostics_;
  BlockInfo* current_block_ = nullptr;
  ScopeStack<VariableInfo*> variable_stack_;
  std::unordered_map<Symbol, FunctionInfo*> symbol_to_function_;
  std::unordered_map<ast::Function*, FunctionInfo*> function_to_info_;
  std::unordered_map<ast::Variable*, VariableInfo*> variable_to_info_;
  std::unordered_map<ast::CallExpression*, FunctionCallInfo> function_calls_;
  std::unordered_map<ast::Expression*, ExpressionInfo> expr_info_;
  FunctionInfo* current_function_ = nullptr;
  semantic::Statement* current_statement_ = nullptr;
  BlockAllocator<VariableInfo> variable_infos_;
  BlockAllocator<FunctionInfo> function_infos_;
};

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_H_
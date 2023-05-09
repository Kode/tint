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

#ifndef SRC_TINT_IR_INSTRUCTION_H_
#define SRC_TINT_IR_INSTRUCTION_H_

#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An instruction in the IR.
class Instruction : public utils::Castable<Instruction, Value> {
  public:
    /// The identifier used by instructions that have no value.
    static constexpr uint32_t kNoID = 0;

    Instruction(const Instruction& inst) = delete;
    Instruction(Instruction&& inst) = delete;
    /// Destructor
    ~Instruction() override;

    Instruction& operator=(const Instruction& inst) = delete;
    Instruction& operator=(Instruction&& inst) = delete;

    /// @returns the type of the value
    const type::Type* Type() const override { return type; }

    /// The instruction identifier
    const uint32_t id = kNoID;

    /// The instruction type
    const type::Type* type = nullptr;

  protected:
    /// Constructor
    Instruction();
    /// Constructor
    /// @param id the instruction id
    /// @param type the result type
    Instruction(uint32_t id, const type::Type* type);
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_INSTRUCTION_H_

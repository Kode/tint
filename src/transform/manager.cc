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

#include "src/transform/manager.h"

#include "src/type_determiner.h"

namespace tint {
namespace transform {

Manager::Manager() : context_(nullptr), module_(nullptr) {}

Manager::Manager(Context* context, ast::Module* module)
    : context_(context), module_(module) {}

Manager::~Manager() = default;

bool Manager::Run() {
  for (auto& transform : transforms_) {
    if (!transform->Run()) {
      error_ = transform->error();
      return false;
    }
  }

  if (context_ != nullptr && module_ != nullptr) {
    // The transformed have potentially inserted nodes into the AST, so the type
    // determinater needs to be run.
    TypeDeterminer td(context_, module_);
    if (!td.Determine()) {
      error_ = td.error();
      return false;
    }
  }

  return true;
}

}  // namespace transform
}  // namespace tint

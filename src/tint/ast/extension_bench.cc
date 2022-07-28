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

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   src/tint/ast/extension_bench.cc.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/ast/extension.h"

#include <array>

#include "benchmark/benchmark.h"

namespace tint::ast {
namespace {

void ExtensionParser(::benchmark::State& state) {
    std::array kStrings{
        "cc6",
        "s",
        "HH6",
        "f16",
        "116",
        "qJ6",
        "f17ll",
        "chromippHm_experqqmetal_dp4a",
        "chrmium_expecimntal_dp4",
        "chrmiumGexpebimental_dp4a",
        "chromium_experimental_dp4a",
        "chromium_exverimentiil_dp4a",
        "chro8ium_experimenWWal_dp4a",
        "chromiMm_eperimxxntal_dp4a",
        "chXggmium_disable_uniformity_aalysis",
        "Xhomiuu_disale_uniformity_analysis",
        "chromium_3isable_uniformity_analysis",
        "chromium_disable_uniformity_analysis",
        "chromiuE_disable_uniformity_analysis",
        "chromium_disable_uniTTormity_aPPalsis",
        "ddhromium_disabexxuniformity_analysis",
    };
    for (auto _ : state) {
        for (auto& str : kStrings) {
            auto result = ParseExtension(str);
            benchmark::DoNotOptimize(result);
        }
    }
}

BENCHMARK(ExtensionParser);

}  // namespace
}  // namespace tint::ast

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
//   src/tint/ast/interpolate_attribute.h.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_TINT_AST_INTERPOLATE_ATTRIBUTE_H_
#define SRC_TINT_AST_INTERPOLATE_ATTRIBUTE_H_

#include <ostream>
#include <string>

#include "src/tint/ast/attribute.h"

namespace tint::ast {

/// The interpolation type.
enum class InterpolationType {
    kUndefined,
    kFlat,
    kLinear,
    kPerspective,
};

/// @param out the std::ostream to write to
/// @param value the InterpolationType
/// @returns `out` so calls can be chained
std::ostream& operator<<(std::ostream& out, InterpolationType value);

/// ParseInterpolationType parses a InterpolationType from a string.
/// @param str the string to parse
/// @returns the parsed enum, or InterpolationType::kUndefined if the string could not be parsed.
InterpolationType ParseInterpolationType(std::string_view str);

constexpr const char* kInterpolationTypeStrings[] = {
    "flat",
    "linear",
    "perspective",
};

/// The interpolation sampling.
enum class InterpolationSampling {
    kUndefined,
    kCenter,
    kCentroid,
    kSample,
};

/// @param out the std::ostream to write to
/// @param value the InterpolationSampling
/// @returns `out` so calls can be chained
std::ostream& operator<<(std::ostream& out, InterpolationSampling value);

/// ParseInterpolationSampling parses a InterpolationSampling from a string.
/// @param str the string to parse
/// @returns the parsed enum, or InterpolationSampling::kUndefined if the string could not be
/// parsed.
InterpolationSampling ParseInterpolationSampling(std::string_view str);

constexpr const char* kInterpolationSamplingStrings[] = {
    "center",
    "centroid",
    "sample",
};

/// An interpolate attribute
class InterpolateAttribute final : public Castable<InterpolateAttribute, Attribute> {
  public:
    /// Create an interpolate attribute.
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param type the interpolation type
    /// @param sampling the interpolation sampling
    InterpolateAttribute(ProgramID pid,
                         NodeID nid,
                         const Source& src,
                         InterpolationType type,
                         InterpolationSampling sampling);
    ~InterpolateAttribute() override;

    /// @returns the WGSL name for the attribute
    std::string Name() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const InterpolateAttribute* Clone(CloneContext* ctx) const override;

    /// The interpolation type
    const InterpolationType type;

    /// The interpolation sampling
    const InterpolationSampling sampling;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_INTERPOLATE_ATTRIBUTE_H_

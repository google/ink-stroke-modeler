/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INK_STROKE_MODELER_NUMBERS_H_
#define INK_STROKE_MODELER_NUMBERS_H_

#if __cplusplus >= 202002L
#include <numbers>
#else
#include <cmath>
#endif

namespace ink {
namespace stroke_model {

#if __cplusplus >= 202002L
constexpr double kPi = std::numbers::pi;
constexpr double kSqrt = std::numbers::sqrt2;
#else
constexpr double kPi = M_PI;
constexpr double kSqrt = M_SQRT2;
#endif

}  // namespace stroke_model
}  // namespace ink

#endif  // INK_STROKE_MODELER_NUMBERS_H_

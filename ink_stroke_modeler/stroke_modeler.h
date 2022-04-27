/*
 * Copyright 2022 Google LLC
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

#ifndef INK_STROKE_MODELER_STROKE_MODELER_H_
#define INK_STROKE_MODELER_STROKE_MODELER_H_

#include <memory>
#include <optional>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ink_stroke_modeler/internal/position_modeler.h"
#include "ink_stroke_modeler/internal/prediction/input_predictor.h"
#include "ink_stroke_modeler/internal/stylus_state_modeler.h"
#include "ink_stroke_modeler/internal/wobble_smoother.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

// This class models a stroke from a raw input stream. The modeling is performed
// in several stages, which are delegated to component classes:
// - Wobble Smoothing: Dampens high-frequency noise from quantization error.
// - Position Modeling: Models the pen tip as a mass, connected by a spring, to
//   a moving anchor.
// - Stylus State Modeling: Constructs stylus states for modeled positions by
//   interpolating over the raw input.
//
// Additionally, this class provides prediction of the modeled stroke.
//
// StrokeModeler is completely unit-agnostic. That is, it doesn't matter what
// units or coordinate-system the input is given in; the output will be given in
// the same coordinate-system and units.
class StrokeModeler {
 public:
  // Clears any in-progress stroke, and initializes (or re-initializes) the
  // model with the given parameters. Returns an error if the parameters are
  // invalid.
  absl::Status Reset(const StrokeModelParams &stroke_model_params);

  // Clears any in-progress stroke, keeping the same model parameters.
  // Returns an error if the model has not yet been initialized via
  // Reset(StrokeModelParams).
  absl::Status Reset();

  // Updates the model with a raw input, returning the generated results. Any
  // previously generated results are stable, i.e. any previously returned
  // Results are still valid.
  //
  // Returns an error if the the model has not yet been initialized (via Reset)
  // or if the input stream is malformed (e.g decreasing time, Up event before
  // Down event).
  //
  // If this does not return an error, the result will contain at least one
  // Result, and potentially more than one if the inputs are slower than
  // the minimum output rate.
  absl::StatusOr<std::vector<Result>> Update(const Input &input);

  // Model the given input prediction without changing the internal model state.
  //
  // Returns an error if the the model has not yet been initialized (via Reset),
  // or if there is no stroke in progress. The output is limited to results
  // where the predictor has sufficient confidence,
  absl::StatusOr<std::vector<Result>> Predict() const;

 private:
  absl::StatusOr<std::vector<Result>> ProcessDownEvent(const Input &input);
  absl::StatusOr<std::vector<Result>> ProcessMoveEvent(const Input &input);
  absl::StatusOr<std::vector<Result>> ProcessUpEvent(const Input &input);

  std::unique_ptr<InputPredictor> predictor_;

  std::optional<StrokeModelParams> stroke_model_params_;

  WobbleSmoother wobble_smoother_;
  PositionModeler position_modeler_;
  StylusStateModeler stylus_state_modeler_;

  struct InputAndCorrectedPosition {
    Input input;
    Vec2 corrected_position{0};
  };
  std::optional<InputAndCorrectedPosition> last_input_;
};

}  // namespace stroke_model
}  // namespace ink

#endif  // INK_STROKE_MODELER_STROKE_MODELER_H_

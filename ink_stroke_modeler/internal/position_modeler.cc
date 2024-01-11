#include "ink_stroke_modeler/internal/position_modeler.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

namespace {

// Discrete estimate of the change in velocity of TipState over one interval.
// Estimates the change in velocity by computing the acceleration of the
// TipState if it were attached by a spring to a fixed anchor at
// anchor_position, than estimating the change in that velocity as that uniform
// acceleration multiplied by the input duration.
Vec2 DeltaV(const TipState& tip_state, const Vec2& anchor_position,
            const Duration& duration, const PositionModelerParams& params) {
  return ((anchor_position - tip_state.position) / params.spring_mass_constant -
          params.drag_constant * tip_state.velocity) *
         duration.Value();
}

}  // namespace

absl::StatusOr<int> NumberOfStepsBetweenInputs(
    const TipState& tip_state, const Input& start, const Input& end,
    const SamplingParams& sampling_params,
    const PositionModelerParams& position_modeler_params) {
  float float_delta = (end.time - start.time).Value();
  int n_steps =
      std::min(std::ceil(float_delta * sampling_params.min_output_rate),
               static_cast<double>(std::numeric_limits<int>::max()));
  Vec2 estimated_end_v = DeltaV(tip_state, end.position, end.time - start.time,
                                position_modeler_params);
  float estimated_angle = tip_state.velocity.AbsoluteAngleTo(estimated_end_v);
  if (sampling_params.max_estimated_angle_to_traverse_per_input > 0) {
    int steps_for_angle = std::min(
        std::ceil(estimated_angle /
                  sampling_params.max_estimated_angle_to_traverse_per_input),
        static_cast<double>(std::numeric_limits<int>::max()));
    if (steps_for_angle > n_steps) {
      n_steps = steps_for_angle;
    }
  }
  if (n_steps > sampling_params.max_outputs_per_call) {
    return absl::InvalidArgumentError(absl::Substitute(
        "Input events are too far apart, requested $0 > $1 samples.", n_steps,
        sampling_params.max_outputs_per_call));
  }
  return n_steps;
}

TipState PositionModeler::Update(Vec2 anchor_position, Time time) {
  Duration delta_time = time - state_.time;
  state_.velocity += DeltaV(state_, anchor_position, delta_time, params_);
  state_.position += delta_time.Value() * state_.velocity;
  state_.time = time;

  return state_;
}

}  // namespace stroke_model
}  // namespace ink

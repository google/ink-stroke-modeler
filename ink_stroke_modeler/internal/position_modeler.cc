#include "ink_stroke_modeler/internal/position_modeler.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/substitute.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

absl::StatusOr<int> NumberOfStepsBetweenInputs(
    const Input& start, const Input& end,
    const ink::stroke_model::SamplingParams& sampling_params) {
  float float_delta = (end.time - start.time).Value();
  int n_steps =
      std::min(std::ceil(float_delta * sampling_params.min_output_rate),
               static_cast<double>(std::numeric_limits<int>::max()));
  if (n_steps > sampling_params.max_outputs_per_call) {
    return absl::InvalidArgumentError(absl::Substitute(
        "Input events are too far apart, requested $0 > $1 samples.", n_steps,
        sampling_params.max_outputs_per_call));
  }
  return n_steps;
}

}  // namespace stroke_model
}  // namespace ink

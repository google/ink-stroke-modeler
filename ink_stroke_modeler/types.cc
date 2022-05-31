#include "ink_stroke_modeler/types.h"

#include "absl/status/status.h"
#include "ink_stroke_modeler/internal/validation.h"

// This convenience macro evaluates the given expression, and if it does not
// return an OK status, returns and propagates the status.
#define RETURN_IF_ERROR(expr)                              \
  do {                                                     \
    if (auto status = (expr); !status.ok()) return status; \
  } while (false)

namespace ink {
namespace stroke_model {

absl::Status ValidateInput(const Input& input) {
  switch (input.event_type) {
    case Input::EventType::kUp:
    case Input::EventType::kMove:
    case Input::EventType::kDown:
      break;
    default:
      return absl::InvalidArgumentError("Unknown Input.event_type.");
  }
  RETURN_IF_ERROR(ValidateIsFiniteNumber(input.position.x, "Input.position.x"));
  RETURN_IF_ERROR(ValidateIsFiniteNumber(input.position.y, "Input.position.y"));
  RETURN_IF_ERROR(ValidateIsFiniteNumber(input.time.Value(), "Input.time"));
  // This probably should also ValidateIsFiniteNumber for pressure, tilt, and
  // orientation, since unknown values for those should be represented as -1.
  // However, some consumers are forwarding NaN values for those fields.
  return absl::OkStatus();
}

}  // namespace stroke_model
}  // namespace ink

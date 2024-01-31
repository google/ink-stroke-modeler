#include "ink_stroke_modeler/types.h"

#include <cmath>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "ink_stroke_modeler/internal/validation.h"

// This convenience macro evaluates the given expression, and if it does not
// return an OK status, returns and propagates the status.
#define RETURN_IF_ERROR(expr)                              \
  do {                                                     \
    if (auto status = (expr); !status.ok()) return status; \
  } while (false)

namespace ink {
namespace stroke_model {

absl::StatusOr<float> Vec2::AbsoluteAngleTo(Vec2 other) const {
  if (!IsFinite() || !other.IsFinite()) {
    return absl::InvalidArgumentError(
        absl::StrFormat("Non-finite inputs: this=%v; other=%v.", *this, other));
  }
  float dot = x * other.x + y * other.y;
  float det = x * other.y - y * other.x;
  return std::abs(std::atan2(det, dot));
}

std::string ToFormattedString(Vec2 vec) {
  // Use StrCat instead of StrFormat to avoid trailing zeros in short decimals.
  return absl::StrCat("(", vec.x, ", ", vec.y, ")");
}

absl::Status ValidateInput(const Input &input) {
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

std::string ToFormattedString(Input::EventType event_type) {
  switch (event_type) {
    case Input::EventType::kDown:
      return "Down";
    case Input::EventType::kMove:
      return "Move";
    case Input::EventType::kUp:
      return "Up";
  }
  return absl::StrFormat("UnknownEventType<%d>", static_cast<int>(event_type));
}

std::string ToFormattedString(const Input &input) {
  return absl::StrFormat(
      "<Input: %v, pos: %v, time: %v, pressure: %v, tilt: %v, orientation:%v>",
      input.event_type, input.position, input.time, input.pressure, input.tilt,
      input.orientation);
}

std::string ToFormattedString(const Result &result) {
  return absl::StrFormat(
      "<Result: pos: %v, velocity: %v, time: %v, pressure: %v, tilt: %v, "
      "orientation: %v>",
      result.position, result.velocity, result.time, result.pressure,
      result.tilt, result.orientation);
}

}  // namespace stroke_model
}  // namespace ink

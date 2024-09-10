#include "ink_stroke_modeler/internal/utils.h"

#include <optional>

#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

constexpr float kStrokeNormalMagnitudeThreshold = 0.999998477;

std::optional<Vec2> GetStrokeNormal(const TipState &tip_state, Time prev_time) {
  if (tip_state.velocity.Magnitude() == 0) {
    if (tip_state.acceleration.Magnitude() == 0) {
      return std::nullopt;
    } else {
      return Vec2{-tip_state.acceleration.y, tip_state.acceleration.x};
    }
  }
  if (tip_state.acceleration.Magnitude() == 0) {
    return Vec2{-tip_state.velocity.y, tip_state.velocity.x};
  }
  if (Vec2::DotProduct(tip_state.velocity, tip_state.acceleration) <
      kStrokeNormalMagnitudeThreshold) {
    return std::nullopt;
  }

  auto unit = [](Vec2 x) { return x / x.Magnitude(); };

  Vec2 stroke_dir =
      unit(tip_state.velocity) +
      unit(tip_state.velocity +
           tip_state.acceleration * (tip_state.time - prev_time).Value());

  return Vec2{-stroke_dir.y, stroke_dir.x};
}

std::optional<float> ProjectToSegmentAlongNormal(Vec2 segment_start,
                                                 Vec2 segment_end,
                                                 Vec2 position,
                                                 Vec2 stroke_normal) {
  auto cross = [](Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; };

  Vec2 v = segment_end - segment_start;
  float det = cross(stroke_normal, v);
  if (det == 0) return std::nullopt;

  Vec2 w = segment_start - position;
  float param = cross(w, stroke_normal) / det;
  if (param < 0 || param > 1) return std::nullopt;
  return param;
}

}  // namespace stroke_model
}  // namespace ink

// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ink_stroke_modeler/internal/stylus_state_modeler.h"

#include <cmath>
#include <deque>
#include <limits>
#include <optional>

#include "ink_stroke_modeler/internal/internal_types.h"
#include "ink_stroke_modeler/internal/utils.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

bool ShouldDropOldestInput(
    const StylusStateModelerParams &params,
    const std::deque<Result> &raw_input_and_stylus_states) {
  if (params.use_stroke_normal_projection) {
    return raw_input_and_stylus_states.size() > params.min_input_samples &&
           // Check the difference between the newest and second-oldest inputs
           // -- if that's greater than `min_sample_duration` then we can drop
           // the oldest without going below `min_sample_duration`.
           // Since `min_input_samples` > 0, the clause above guarantees that we
           // have at least two inputs.
           (raw_input_and_stylus_states.back().time -
            raw_input_and_stylus_states[1].time) > params.min_sample_duration;

  } else {
    return raw_input_and_stylus_states.size() > params.max_input_samples;
  }
}

}  // namespace

void StylusStateModeler::Update(Vec2 position, Time time,
                                const StylusState &state) {
  // Possibly NaN should be prohibited in ValidateInput, but due to current
  // consumers, that can't be tightened for these values currently.
  if (state.pressure < 0 || std::isnan(state.pressure)) {
    state_.received_unknown_pressure = true;
  }
  if (state.tilt < 0 || std::isnan(state.tilt)) {
    state_.received_unknown_tilt = true;
  }
  if (state.orientation < 0 || std::isnan(state.orientation)) {
    state_.received_unknown_orientation = true;
  }

  if (!params_.use_stroke_normal_projection &&
      state_.received_unknown_pressure && state_.received_unknown_tilt &&
      state_.received_unknown_orientation) {
    // We've stopped tracking all fields, so there's no need to keep updating.
    state_.raw_input_and_stylus_states.clear();
    return;
  }

  Vec2 velocity = {0, 0};
  Vec2 acceleration = {0, 0};
  if (!state_.raw_input_and_stylus_states.empty() &&
      time != state_.raw_input_and_stylus_states.back().time) {
    velocity = (position - state_.raw_input_and_stylus_states.back().position) /
               (time - state_.raw_input_and_stylus_states.back().time).Value();
    acceleration =
        (velocity - state_.raw_input_and_stylus_states.back().velocity) /
        (time - state_.raw_input_and_stylus_states.back().time).Value();
  }

  state_.raw_input_and_stylus_states.push_back({
      .position = position,
      .velocity = velocity,
      .acceleration = acceleration,
      .time = time,
      .pressure = state.pressure,
      .tilt = state.tilt,
      .orientation = state.orientation,
  });

  while (ShouldDropOldestInput(params_, state_.raw_input_and_stylus_states)) {
    state_.raw_input_and_stylus_states.pop_front();
  }
}

void StylusStateModeler::Reset(const StylusStateModelerParams &params) {
  state_.raw_input_and_stylus_states.clear();
  state_.received_unknown_pressure = false;
  state_.received_unknown_tilt = false;
  state_.received_unknown_orientation = false;
  save_active_ = false;
  params_ = params;
}

namespace {

// The location of the projection point along the raw input polyline.
struct RawInputProjection {
  int segment_index;
  float ratio_along_segment;
};

std::optional<RawInputProjection> ProjectAlongStrokeNormal(
    Vec2 position, Vec2 acceleration, Time time, Vec2 stroke_normal,
    const std::deque<Result> &raw_input_polyline) {
  // We track the best candidate separately for the left and right sides of the
  // stroke, in case the closest projection is not in the right direction.
  std::optional<RawInputProjection> best_left_projection;
  std::optional<RawInputProjection> best_right_projection;
  float best_distance_left = std::numeric_limits<float>::infinity();
  float best_distance_right = std::numeric_limits<float>::infinity();

  // Update `best_projection` and `best_distance` if needed.
  auto maybe_update_projection =
      [](RawInputProjection candidate, float distance,
         std::optional<RawInputProjection> &best_projection,
         float &best_distance) {
        if (distance < best_distance) {
          best_projection = candidate;
          best_distance = distance;
        }
      };

  for (decltype(raw_input_polyline.size()) i = 0;
       i < raw_input_polyline.size() - 1; ++i) {
    const Vec2 segment_start = raw_input_polyline[i].position;
    const Vec2 segment_end = raw_input_polyline[i + 1].position;

    // Find the intersection of the stroke normal with the polyline segment.
    std::optional<float> segment_ratio = ProjectToSegmentAlongNormal(
        segment_start, segment_end, position, stroke_normal);
    if (!segment_ratio.has_value()) continue;

    Vec2 projection = Interp(segment_start, segment_end, *segment_ratio);
    float distance = Distance(position, projection);

    // We update either the best left or the right projection, depending which
    // side of the stroke it lies on -- recall that the stroke normal always
    // points to the left.
    RawInputProjection candidate{.segment_index = static_cast<int>(i),
                                 .ratio_along_segment = *segment_ratio};
    if (Vec2::DotProduct(projection - position, stroke_normal) < 0) {
      maybe_update_projection(candidate, distance, best_right_projection,
                              best_distance_right);
    } else {
      maybe_update_projection(candidate, distance, best_left_projection,
                              best_distance_left);
    }
  }

  if (best_left_projection.has_value() && best_right_projection.has_value()) {
    // We have candidate projections on both sides of the stroke, so we want to
    // choose the one on the "outside" of the turn. The acceleration will always
    // point to the "inside" of the curve, so we can compare it to the stroke
    // normal (which always points left) to determine whether to use the left or
    // right candidate.
    return Vec2::DotProduct(stroke_normal, acceleration) > 0
               ? best_right_projection
               : best_left_projection;
  }

  // We have at most one projection -- return it if we have it. If we have
  // neither, this returns std::nullopt, which is exactly what we want.
  return best_right_projection.has_value() ? best_right_projection
                                           : best_left_projection;
}

std::optional<RawInputProjection> ProjectToClosestPoint(
    Vec2 position, const std::deque<Result> &raw_input_polyline) {
  std::optional<RawInputProjection> best_projection;
  float min_distance = std::numeric_limits<float>::infinity();
  for (decltype(raw_input_polyline.size()) i = 0;
       i < raw_input_polyline.size() - 1; ++i) {
    const Vec2 segment_start = raw_input_polyline[i].position;
    const Vec2 segment_end = raw_input_polyline[i + 1].position;
    float segment_ratio =
        NearestPointOnSegment(segment_start, segment_end, position);
    float distance =
        Distance(position, Interp(segment_start, segment_end, segment_ratio));
    if (distance <= min_distance) {
      best_projection =
          RawInputProjection{.segment_index = static_cast<int>(i),
                             .ratio_along_segment = segment_ratio};
      min_distance = distance;
    }
  }
  return best_projection;
}

}  // namespace

Result StylusStateModeler::Query(const TipState &tip,
                                 std::optional<Vec2> stroke_normal) const {
  if (state_.raw_input_and_stylus_states.empty())
    return {
        .position = {0, 0},
        .velocity = {0, 0},
        .acceleration = {0, 0},
        .time = Time(0),
        .pressure = -1,
        .tilt = -1,
        .orientation = -1,
    };

  std::optional<RawInputProjection> projection =
      params_.use_stroke_normal_projection && stroke_normal.has_value()
          ? ProjectAlongStrokeNormal(tip.position, tip.acceleration, tip.time,
                                     *stroke_normal,
                                     state_.raw_input_and_stylus_states)
          : ProjectToClosestPoint(tip.position,
                                  state_.raw_input_and_stylus_states);

  Result projected_result;
  if (projection.has_value()) {
    projected_result = InterpResult(
        state_.raw_input_and_stylus_states[projection->segment_index],
        state_.raw_input_and_stylus_states[projection->segment_index + 1],
        projection->ratio_along_segment);
  } else {
    // We didn't find an appropriate projection; fall back to projecting to the
    // closest endpoint of the raw input polyline.
    projected_result =
        Distance(state_.raw_input_and_stylus_states.front().position,
                 tip.position) <
                Distance(state_.raw_input_and_stylus_states.back().position,
                         tip.position)
            ? state_.raw_input_and_stylus_states.front()
            : state_.raw_input_and_stylus_states.back();
  }

  // Correct the time and strip missing fields before returning.
  projected_result.time = tip.time;
  if (state_.received_unknown_pressure) {
    projected_result.pressure = -1;
  }
  if (state_.received_unknown_tilt) {
    projected_result.tilt = -1;
  }
  if (state_.received_unknown_orientation) {
    projected_result.orientation = -1;
  }

  return projected_result;
}

void StylusStateModeler::Save() {
  saved_state_ = state_;
  save_active_ = true;
}

void StylusStateModeler::Restore() {
  if (save_active_) {
    state_ = saved_state_;
  }
}

}  // namespace stroke_model
}  // namespace ink

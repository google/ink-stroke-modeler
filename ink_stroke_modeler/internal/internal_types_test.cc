#include "ink_stroke_modeler/internal/internal_types.h"

#include "gtest/gtest.h"
#include "absl/strings/str_format.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

TEST(InternalTypesTest, TipStateString) {
  EXPECT_EQ(absl::StrFormat("%v", TipState{.position = {1, 2},
                                           .velocity = {3, 4},
                                           .acceleration = {5, 6},
                                           .time = Time(7)}),
            "<TipState: pos: (1, 2), vel: (3, 4), acc: (5, 6), time: 7>");
}

TEST(InternalTypesTest, StylusStateEquals) {
  EXPECT_EQ((StylusState{0.1, 0.2, 0.3}), (StylusState{0.1, 0.2, 0.3}));
  EXPECT_FALSE((StylusState{0.1, 0.2, 0.3}) == (StylusState{0.1, 0.02, 0.3}));
}

TEST(InternalTypesTest, StylusStateString) {
  EXPECT_EQ(absl::StrFormat("%v", StylusState{0.1, 0.2, 0.3}),
            "<StylusState: pressure: 0.1, tilt: 0.2, orientation: 0.3>");
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink

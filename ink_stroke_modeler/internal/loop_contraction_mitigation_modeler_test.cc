#include "ink_stroke_modeler/internal/loop_contraction_mitigation_modeler.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {
namespace {

using ::testing::FloatNear;
using LoopContractionMitigationParameters =
    PositionModelerParams::LoopContractionMitigationParameters;

const LoopContractionMitigationParameters kDefaultParams{
    .is_enabled = true,
    .speed_lower_bound = 0,
    .speed_upper_bound = 100,
    .interpolation_strength_at_speed_lower_bound = 1,
    .interpolation_strength_at_speed_upper_bound = 0,
    .min_speed_sampling_window = Duration(0.6),
    .min_discrete_speed_samples = 5};

TEST(LoopContractionMitigationModelerTest,
     GetInterpolationValueOnEmptyModeler) {
  LoopContractionMitigationModeler modeler;
  modeler.Reset(kDefaultParams);

  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(1, 0.01));
}

TEST(LoopContractionMitigationModelerTest, UpdateWithOneSample) {
  LoopContractionMitigationModeler modeler;
  modeler.Reset(kDefaultParams);

  EXPECT_THAT(modeler.Update({3, 4}, Time(0)), FloatNear(0.95, 0.01));
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(0.95, 0.01));
}

TEST(LoopContractionMitigationModelerTest, IsEnabledFalseResultsInOne) {
  LoopContractionMitigationParameters params = {
      .is_enabled = false,
      .speed_lower_bound = 0,
      .speed_upper_bound = 10,
      .interpolation_strength_at_speed_lower_bound = 1,
      .interpolation_strength_at_speed_upper_bound = 0,
      .min_speed_sampling_window = Duration(0.6),
      .min_discrete_speed_samples = 5};

  LoopContractionMitigationModeler modeler;
  modeler.Reset(params);

  EXPECT_THAT(modeler.Update({3, 4}, Time(0)), FloatNear(1, 0.01));
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(1, 0.01));
}

TEST(LoopContractionMitigationModelerTest, ResetClearsModeler) {
  LoopContractionMitigationModeler modeler;
  modeler.Reset(kDefaultParams);

  EXPECT_THAT(modeler.Update({3, 4}, Time(0)), FloatNear(0.95, 0.01));
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(0.95, 0.01));

  modeler.Reset(kDefaultParams);
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(1, 0.01));
}

TEST(LoopContractionMitigationModelerTest,
     MultipleUpdatesButLessThanDurationOrNumberOfSamples) {
  LoopContractionMitigationModeler modeler;
  modeler.Reset(kDefaultParams);

  // Average is 5.
  EXPECT_THAT(modeler.Update({3, 4}, Time(0)), FloatNear(0.95, 0.01));
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(0.95, 0.01));

  // Average is (5 + 3) / 2=4.
  EXPECT_THAT(modeler.Update({0, 3}, Time(0.1)), FloatNear(0.96, 0.01));
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(0.96, 0.01));

  // Average is (5 + 3 + 10) / 3 = 6.
  EXPECT_THAT(modeler.Update({-10, 0}, Time(0.2)), FloatNear(0.94, 0.01));
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(0.94, 0.01));
}

TEST(LoopContractionMitigationModelerTest,
     MultipleUpdatesMoreSamplesAndOverDurationTheshold) {
  LoopContractionMitigationModeler modeler;
  modeler.Reset(kDefaultParams);

  // Average is 5.
  ASSERT_THAT(modeler.Update({0, 5}, Time(0)), FloatNear(0.95, 0.01));
  // Average is (5 + 3) / 2=4.
  ASSERT_THAT(modeler.Update({0, 3}, Time(0.15)), FloatNear(0.96, 0.01));
  // Average is (5 + 3 + 10) / 3 = 6.
  ASSERT_THAT(modeler.Update({-10, 0}, Time(0.3)), FloatNear(0.94, 0.01));
  // Average is (5 + 3 + 10 + 2) / 4 = 5.
  ASSERT_THAT(modeler.Update({0, 2}, Time(0.5)), FloatNear(0.95, 0.01));
  // Average is (5 + 3 + 10 + 2 + 2) / 5 = 4.4. This update crosses the duration
  // boundary but not the min sample count.
  ASSERT_THAT(modeler.Update({0, 2}, Time(0.65)), FloatNear(0.956, 0.01));

  // The next one should clear the first value.
  // Average is (3 + 10 + 2 + 2 + 1) / 5 = 3.6)
  EXPECT_THAT(modeler.Update({0, 2}, Time(0.75)), FloatNear(0.964, 0.01));
}

TEST(LoopContractionMitigationModelerTest, SaveAndRestore) {
  LoopContractionMitigationModeler modeler;
  modeler.Reset(kDefaultParams);

  // Average is 5.
  ASSERT_THAT(modeler.Update({0, 5}, Time(0)), FloatNear(0.95, 0.01));
  // Average is (5 + 3) / 2=4.
  ASSERT_THAT(modeler.Update({0, 3}, Time(0.15)), FloatNear(0.96, 0.01));
  // Average is (5 + 3 + 10) / 3 = 6.
  ASSERT_THAT(modeler.Update({-10, 0}, Time(0.3)), FloatNear(0.94, 0.01));
  // Average is (5 + 3 + 10 + 2) / 4 = 5.
  ASSERT_THAT(modeler.Update({0, 2}, Time(0.5)), FloatNear(0.95, 0.01));
  // Average is (5 + 3 + 10 + 2 + 2) / 5 = 4.4.
  ASSERT_THAT(modeler.Update({0, 2}, Time(0.65)), FloatNear(0.956, 0.01));

  modeler.Save();

  // This clears the first 2 values
  // Average is (3 + 10 + 2 + 2 + 1) / 5 = 3.6)
  ASSERT_THAT(modeler.Update({0, 2}, Time(0.75)), FloatNear(0.964, 0.01));
  // Average is (10 + 2 + 2 + 1 + 5) / 5  = 4.
  ASSERT_THAT(modeler.Update({0, 5}, Time(0.9)), FloatNear(0.96, 0.01));

  modeler.Restore();
  // This should return the last value from before the save.
  EXPECT_THAT(modeler.GetInterpolationValue(), FloatNear(0.956, 0.01));
}

}  // namespace
}  // namespace stroke_model
}  // namespace ink

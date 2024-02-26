#include <variant>

#include "fuzztest/fuzztest.h"
#include "ink_stroke_modeler/params.h"
#include "ink_stroke_modeler/stroke_modeler.h"
#include "ink_stroke_modeler/types.h"

namespace ink {
namespace stroke_model {

// Tag type for when the input calls StrokeModeler.Predict, which takes
// no arguments.
struct PredictionCommand {};

fuzztest::Domain<Duration> ArbitraryDuration() {
  return fuzztest::ConstructorOf<Duration>(fuzztest::Arbitrary<double>());
}

fuzztest::Domain<Time> ArbitraryTime() {
  return fuzztest::ConstructorOf<Time>(fuzztest::Arbitrary<double>());
}

fuzztest::Domain<StrokeModelParams> ArbitraryStrokeModelParams() {
  return fuzztest::StructOf<StrokeModelParams>(
      fuzztest::StructOf<WobbleSmootherParams>(ArbitraryDuration(),
                                               fuzztest::Arbitrary<float>(),
                                               fuzztest::Arbitrary<float>()),
      fuzztest::Arbitrary<PositionModelerParams>(),
      fuzztest::StructOf<SamplingParams>(
          fuzztest::Arbitrary<double>(), fuzztest::Arbitrary<float>(),
          fuzztest::Arbitrary<int>(),
          /*max_outputs_per_call*/ fuzztest::InRange(1000, 100000),
          fuzztest::Arbitrary<double>()),
      fuzztest::Arbitrary<StylusStateModelerParams>(),
      fuzztest::VariantOf(
          fuzztest::Arbitrary<StrokeEndPredictorParams>(),
          fuzztest::StructOf<KalmanPredictorParams>(
              fuzztest::Arbitrary<double>(), fuzztest::Arbitrary<double>(),
              fuzztest::Arbitrary<int>(), fuzztest::Arbitrary<int>(),
              fuzztest::Arbitrary<float>(), fuzztest::Arbitrary<float>(),
              fuzztest::Arbitrary<float>(), ArbitraryDuration(),
              fuzztest::Arbitrary<KalmanPredictorParams::ConfidenceParams>()),
          fuzztest::Arbitrary<DisabledPredictorParams>()),
      fuzztest::Arbitrary<ExperimentalParams>());
}

fuzztest::Domain<Input> ArbitraryInput() {
  return fuzztest::StructOf<Input>(
      fuzztest::ElementOf<Input::EventType>({Input::EventType::kDown,
                                             Input::EventType::kMove,
                                             Input::EventType::kUp}),
      fuzztest::Arbitrary<Vec2>(), ArbitraryTime(),
      fuzztest::Arbitrary<float>(), fuzztest::Arbitrary<float>(),
      fuzztest::Arbitrary<float>());
}

// Runs a series of calls to Rest, Update, and Predict.
void StrokeModelerDoesNotCrash(
    std::vector<std::variant<StrokeModelParams, Input, PredictionCommand>>
        commands) {
  StrokeModeler stroke_modeler;
  for (auto& command : commands) {
    if (std::holds_alternative<StrokeModelParams>(command)) {
      stroke_modeler.Reset(std::get<StrokeModelParams>(command)).IgnoreError();
    } else if (std::holds_alternative<Input>(command)) {
      std::vector<Result> results;
      stroke_modeler.Update(std::get<Input>(command), results).IgnoreError();
    } else if (std::holds_alternative<PredictionCommand>(command)) {
      std::vector<Result> results;
      stroke_modeler.Predict(results).IgnoreError();
    }
  }
}
FUZZ_TEST(StrokeModelerFuzzTest, StrokeModelerDoesNotCrash)
    .WithDomains(fuzztest::VectorOf(
        fuzztest::VariantOf(ArbitraryStrokeModelParams(), ArbitraryInput(),
                            fuzztest::Arbitrary<PredictionCommand>())));

}  // namespace stroke_model
}  // namespace ink

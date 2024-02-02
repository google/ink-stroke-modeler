#include "ink_stroke_modeler/internal/validation.h"

#include <cmath>

#include "gtest/gtest.h"
#include "absl/status/status.h"

namespace {

TEST(ValidateIsFiniteNumberTest, AcceptFinite) {
  ASSERT_TRUE(ValidateIsFiniteNumber(1, "foo").ok());
}

TEST(ValidateIsFiniteNumberTest, RejectNan) {
  absl::Status status = ValidateIsFiniteNumber(NAN, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo is NaN");
}

TEST(ValidateIsFiniteNumberTest, RejectInf) {
  absl::Status status = ValidateIsFiniteNumber(INFINITY, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo is infinite");
}

TEST(ValidateGreaterThanZeroTest, AcceptPositive) {
  ASSERT_TRUE(ValidateGreaterThanZero(1, "foo").ok());
}

TEST(ValidateGreaterThanZeroTest, RejectZero) {
  absl::Status status = ValidateGreaterThanZero(0, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo must be greater than zero. Actual value: 0");
}

TEST(ValidateGreaterThanZeroTest, RejectNegative) {
  absl::Status status = ValidateGreaterThanZero(-1, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(),
            "foo must be greater than zero. Actual value: -1");
}

TEST(ValidateGreaterThanZeroTest, RejectNan) {
  absl::Status status = ValidateGreaterThanZero(NAN, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo is NaN");
}

TEST(ValidateGreaterThanZeroTest, RejectInf) {
  absl::Status status = ValidateGreaterThanZero(INFINITY, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo is infinite");
}

TEST(ValidateGreaterThanOrEqualToZeroTest, AcceptPositive) {
  ASSERT_TRUE(ValidateGreaterThanOrEqualToZero(1, "foo").ok());
}

TEST(ValidateGreaterThanOrEqualToZeroTest, AcceptZero) {
  absl::Status status = ValidateGreaterThanOrEqualToZero(0, "foo");
  ASSERT_TRUE(ValidateGreaterThanOrEqualToZero(0, "foo").ok());
}

TEST(ValidateGreaterThanOrEqualToZeroTest, RejectNegative) {
  absl::Status status = ValidateGreaterThanOrEqualToZero(-1, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(),
            "foo must be greater than or equal to zero. Actual value: -1");
}

TEST(ValidateGreaterThanOrEqualToZeroTest, RejectNan) {
  absl::Status status = ValidateGreaterThanOrEqualToZero(NAN, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo is NaN");
}

TEST(ValidateGreaterThanOrEqualToZeroTest, RejectInf) {
  absl::Status status = ValidateGreaterThanOrEqualToZero(INFINITY, "foo");
  ASSERT_FALSE(status.ok());
  ASSERT_EQ(status.message(), "foo is infinite");
}

}  // namespace

// FIXME: I am creating these functions here. I will come back
// later to determine if the equivalent is available in softfloat.
// I did not immediately see what I was looking for.

#include "dromajo_protos.h"
#include <cstdint>
#include <cmath>
// -------------------------------------------------------------------------
// TODO: future feature
// If we move away from softfloat for performance, these functions
// will manage host FP state
// -------------------------------------------------------------------------
void setHostRoundingMode(RoundingModeEnum) { }
void clearHostFpFlags() { }
void raiseHostFpFlags(uint32_t) { }
// -----------------------------------------------------------------------------
// FIXME: Before implementing this need a good test
// -----------------------------------------------------------------------------
uint16_t froundHalfPrecision(uint16_t value) { return 0; }
// -----------------------------------------------------------------------------
// Checks for nan, inf, zero have already been performed
// FIXME: this needs to handle the rounding mode
// -----------------------------------------------------------------------------
uint32_t froundSinglePrecision(uint32_t val,RoundingModeEnum erm, bool exact)
{
  float f1  = std::bit_cast<float>(val);
  float res = f1;

  int32_t exp=0;

  std::frexp(f1,&exp);

  if(exp < std::numeric_limits<float>::digits - 1) {

    int32_t intVal = static_cast<int32_t>(f1);
    res            = static_cast<float>(intVal);

    clearHostFpFlags();

    if (intVal == 0 && std::signbit(f1)) res = std::copysign(res, f1);

    if (exact && res != f1) {
      raiseHostFpFlags(FFLAG_INEXACT);
    }
  }

  return std::bit_cast<uint32_t>(res);
}
// -----------------------------------------------------------------------------
// Checks for nan, inf, zero have already been performed
// FIXME: this needs to handle the rounding mode
// -----------------------------------------------------------------------------
uint64_t froundDoublePrecision(uint64_t val,RoundingModeEnum erm, bool exact)
{
  double f1  = std::bit_cast<double>(val);
  double res = f1;

  int32_t exp=0;

  std::frexp(f1,&exp);

  if(exp < std::numeric_limits<double>::digits - 1) {

    int64_t intVal = static_cast<int64_t>(f1);
    res            = static_cast<double>(intVal);

    clearHostFpFlags();

    if (intVal == 0 && std::signbit(f1)) res = std::copysign(res, f1);

    if (exact && res != f1) {
      raiseHostFpFlags(FFLAG_INEXACT);
    }
  }

  return std::bit_cast<uint64_t>(res);
}
// -----------------------------------------------------------------------------
// Nan = exp all 1's, fraction != 0
// HP quiet bit is [9]
// SP quiet bit is [22]
// DP quiet bit is [51]
// -----------------------------------------------------------------------------
bool isHalfPrecisionNaN(uint16_t value)
{
    return (value & 0x7C00) == 0x7C00 && (value & 0x03FF) != 0;
}
// -----------------------------------------------------------------------------
bool isHalfPrecisionSignalingNaN(uint16_t value)
{
    return isHalfPrecisionNaN(value) && (value & 0x0200) == 0;
}
// -----------------------------------------------------------------------------
bool isHalfPrecisionQuietNaN(uint16_t value)
{
    return isHalfPrecisionNaN(value) && (value & 0x0200) != 0;
}
// -----------------------------------------------------------------------------
bool isSinglePrecisionNaN(uint32_t value)
{
    return (value & 0x7F800000) == 0x7F800000 && (value & 0x007FFFFF) != 0;
}
// -----------------------------------------------------------------------------
bool isSinglePrecisionSignalingNaN(uint32_t value)
{
    return isSinglePrecisionNaN(value) && (value & 0x00400000) == 0;
}
// -----------------------------------------------------------------------------
bool isSinglePrecisionQuietNaN(uint32_t value)
{
    return isSinglePrecisionNaN(value) && (value & 0x00400000) != 0;
}
// -----------------------------------------------------------------------------
bool isDoublePrecisionNaN(uint64_t value)
{
    return (value & 0x7FF0000000000000) == 0x7FF0000000000000
        && (value & 0x000FFFFFFFFFFFFF) != 0;
}
// -----------------------------------------------------------------------------
bool isDoublePrecisionSignalingNaN(uint64_t value)
{
    return isDoublePrecisionNaN(value) && (value & 0x0008000000000000) == 0;
}
// -----------------------------------------------------------------------------
bool isDoublePrecisionQuietNaN(uint64_t value)
{
    return isDoublePrecisionNaN(value) && (value & 0x0008000000000000) != 0;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
bool isHalfPrecisionZero(uint16_t value)
{
    return (value & 0x7FFF) == 0;
}
bool isSinglePrecisionZero(uint32_t value)
{
    return (value & 0x7FFFFFFF) == 0;
}
bool isDoublePrecisionZero(uint64_t value)
{
    return (value & 0x7FFFFFFFFFFFFFFF) == 0;
}
// -----------------------------------------------------------------------------
bool isHalfPrecisionInfinity(uint16_t value)
{
    // Check if exponent is all ones and fraction is zero
    return (value & 0x7C00) == 0x7C00 && (value & 0x03FF) == 0;
}
// -----------------------------------------------------------------------------
bool isPositiveInfinity(uint16_t value)
{
    // Positive infinity: sign bit is 0
    return isHalfPrecisionInfinity(value) && (value & 0x8000) == 0;
}
// -----------------------------------------------------------------------------
bool isNegativeInfinity(uint16_t value)
{
    // Negative infinity: sign bit is 1
    return isHalfPrecisionInfinity(value) && (value & 0x8000) != 0;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
bool isSinglePrecisionInfinity(uint32_t value)
{
    // Check if exponent is all ones and fraction is zero
    return (value & 0x7F800000) == 0x7F800000 && (value & 0x007FFFFF) == 0;
}

// -----------------------------------------------------------------------------
bool isPositiveInfinity(uint32_t value)
{
    // Positive infinity: sign bit is 0
    return isSinglePrecisionInfinity(value) && (value & 0x80000000) == 0;
}

// -----------------------------------------------------------------------------
bool isNegativeInfinity(uint32_t value)
{
    // Negative infinity: sign bit is 1
    return isSinglePrecisionInfinity(value) && (value & 0x80000000) != 0;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
bool isDoublePrecisionInfinity(uint64_t value)
{
    // Check if exponent is all ones and fraction is zero
    return (value & 0x7FF0000000000000) == 0x7FF0000000000000
        && (value & 0x000FFFFFFFFFFFFF) == 0;
}
// -----------------------------------------------------------------------------
bool isPositiveInfinity(uint64_t value)
{
    // Positive infinity: sign bit is 0
    return isDoublePrecisionInfinity(value) && (value & 0x8000000000000000) == 0;
}
// -----------------------------------------------------------------------------
bool isNegativeInfinity(uint64_t value)
{
    // Negative infinity: sign bit is 1
    return isDoublePrecisionInfinity(value) && (value & 0x8000000000000000) != 0;
}

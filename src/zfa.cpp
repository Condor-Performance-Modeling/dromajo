
#include "dromajo_protos.h"
#include <cstdint>
#include <unordered_map>

// =========================================================================
// ZFA
//
// fli/fround
// =========================================================================
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
uint64_t fround_h64(uint32_t rs1, uint32_t rm, uint32_t *pfflags) { return 0; }
uint64_t fround_s64(uint32_t rs1, uint32_t rm, uint32_t *pfflags) { return 0; }
uint64_t fround_d64(uint32_t rs1, uint32_t rm, uint32_t *pfflags) { return 0; }
uint64_t froundnx_h64(uint32_t rs1, uint32_t rm, uint32_t *pfflags) { return 0; }
uint64_t froundnx_s64(uint32_t rs1, uint32_t rm, uint32_t *pfflags) { return 0; }
uint64_t froundnx_d64(uint32_t rs1, uint32_t rm, uint32_t *pfflags) { return 0; }
// -------------------------------------------------------------------------
enum FormatField {
 FMT_S = 0, //00 S 32-bit single-precision
 FMT_D = 1, //01 D 64-bit double-precision
 FMT_H = 2, //10 H 16-bit half-precision
 FMT_Q = 3  //11 Q 128-bit quad-precision
};
// -------------------------------------------------------------------------
enum RoundingModes {
  RNE = 0,    //000 RNE Round to Nearest, ties to Even
  RTZ = 1,    //001 RTZ Round towards Zero
  RDN = 2,    //010 RDN Round Down (towards )
  RUP = 3,    //011 RUP Round Up (towards )
  RMM = 4,    //100 RMM Round to Nearest, ties to Max Magnitude
  RSRV0 = 5,  //101 Reserved for future use.
  RSRV1 = 6,  //1110 Reserved for future use.
  DYN   = 7   //111 DYN In instruction’s rm field, selects dynamic rounding mode; 
};
// -------------------------------------------------------------------------
// The magic numbers for fli_X are explained in the zfa adoc, see also test files
// -------------------------------------------------------------------------
struct FPSizes {
    uint16_t half;   // Half-precision (16-bit)
    uint32_t single; // Single-precision (32-bit)
    uint64_t dbl;    // Double-precision (64-bit)
};

static const std::unordered_map<uint32_t, FPSizes> FLI_MAGIC = {
    {0,  {0xBC00, 0xBF800000, 0xBFF0000000000000}},
    {1,  {0x0400, 0x00800000, 0x0010000000000000}},
    {2,  {0x0100, 0x37800000, 0x3EF0000000000000}},
    {3,  {0x0200, 0x38000000, 0x3F00000000000000}},
    {4,  {0x1C00, 0x3B800000, 0x3F70000000000000}},
    {5,  {0x2000, 0x3C000000, 0x3F80000000000000}},
    {6,  {0x3000, 0x3D800000, 0x3FB0000000000000}},
    {7,  {0x3400, 0x3E000000, 0x3FC0000000000000}},
    {8,  {0x3800, 0x3E800000, 0x3FD0000000000000}},
    {9,  {0x3840, 0x3E800010, 0x3FD2000000000000}},
    {10, {0x3880, 0x3E800020, 0x3FD4000000000000}},
    {11, {0x38C0, 0x3E800030, 0x3FD6000000000000}},
    {12, {0x3C00, 0x3F000000, 0x3FE0000000000000}},
    {13, {0x3C40, 0x3F000010, 0x3FE2000000000000}},
    {14, {0x3C80, 0x3F000020, 0x3FE4000000000000}},
    {15, {0x3CC0, 0x3F000030, 0x3FE6000000000000}},
    {16, {0x3E00, 0x3F800000, 0x3FF0000000000000}},
    {17, {0x4000, 0x40000000, 0x4000000000000000}},
    {18, {0x4200, 0x40400000, 0x4010000000000000}},
    {19, {0x4300, 0x40800000, 0x4020000000000000}},
    {20, {0x4400, 0x40A00000, 0x4024000000000000}},
    {21, {0x4500, 0x40C00000, 0x4028000000000000}},
    {22, {0x4600, 0x40E00000, 0x402C000000000000}},
    {23, {0x4800, 0x41000000, 0x4030000000000000}},
    {24, {0x5000, 0x42000000, 0x4040000000000000}},
    {25, {0x5800, 0x43000000, 0x4050000000000000}},
    {26, {0x7000, 0x47000000, 0x4070000000000000}},
    {27, {0x7800, 0x47800000, 0x4080000000000000}},
    {28, {0x7C00, 0x4F000000, 0x40E0000000000000}},
    {29, {0x7E00, 0x4F800000, 0x40F0000000000000}},
    {30, {0x7C00, 0x7F800000, 0x7FF0000000000000}},
    {31, {0x7C01, 0x7FC00000, 0x7FF8000000000000}},
};
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
template <typename T>
T get_fli(uint32_t idx) {
  auto it = FLI_MAGIC.find(idx);
  if (it == FLI_MAGIC.end()) {
    if constexpr (std::is_same<T, uint16_t>::value) return 0x7C01; // H NaN
    if constexpr (std::is_same<T, uint32_t>::value) return 0x7FC00000; // S NaN
    if constexpr (std::is_same<T, uint64_t>::value) return 0x7FF8000000000000; // D NaN
  }
  if constexpr (std::is_same<T, uint16_t>::value) return it->second.half;
  if constexpr (std::is_same<T, uint32_t>::value) return it->second.single;
  if constexpr (std::is_same<T, uint64_t>::value) return it->second.dbl;
  return 0; // Should never reach here
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
uint64_t fli_h64(uint32_t rs) {
    uint32_t idx = rs & 0x1F;
    return get_fli<uint16_t>(idx);
}

uint64_t fli_s64(uint32_t rs) {
    uint32_t idx = rs & 0x1F;
    return get_fli<uint32_t>(idx);
}

uint64_t fli_d64(uint32_t rs) {
    uint32_t idx = rs & 0x1F;
    return get_fli<uint64_t>(idx);
}

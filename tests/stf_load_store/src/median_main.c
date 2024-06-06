// See LICENSE for license details.

//**************************************************************************
// Median filter benchmark
//--------------------------------------------------------------------------
//
// This benchmark performs a 1D three element median filter. The
// input data (and reference data) should be generated using the
// median_gendata.pl perl script and dumped to a file named
// dataset1.h.

#include "util.h"
#include "median.h"
#include "trace_macros.h"
#include <stdio.h>
#include <stdint.h>
extern volatile uint64_t tohost;
//--------------------------------------------------------------------------
// Input/Reference Data
#include "dataset1.h"
//--------------------------------------------------------------------------
// Main

//volatile uint64_t tohost=0;

int main( int argc, char* argv[] )
{
  START_TRACE;
  int results_data[DATA_SIZE];

#if PREALLOCATE
  // If needed we preallocate everything in the caches
  median( DATA_SIZE, input_data, results_data );
#endif

  // Do the filter
  setStats(1);
  median( DATA_SIZE, input_data, results_data );
  setStats(0);

  // Check the results
  //return verify( DATA_SIZE, results_data, verify_data );
  uint32_t ret = verify( DATA_SIZE, results_data, verify_data );

  STOP_TRACE;
  tohost = 1;
  return 0;
}

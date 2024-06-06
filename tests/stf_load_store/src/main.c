#include "util.h"
#include <stdio.h>
#include <stdint.h>

int main( int argc, char* argv[] )
{
  START_TRACE;

  asm volatile("li t0, 42");

  STOP_TRACE;
  return 0;
}

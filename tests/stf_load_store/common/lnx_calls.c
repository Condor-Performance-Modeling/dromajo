// This file is linux specific and holds a few common prototypes
// and the volatile globals

#include <stdint.h>
#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

volatile uint64_t tohost=0;
volatile uint64_t fromhost=0;
volatile char     conio=0;

//This is for the VCAD ISS simulator
void _pass() __attribute__((used));
void _pass() {}

//Access to CSRs is limited in linux
void setStats(int arg) { UNUSED(arg); }

//One micro-bench (mm) needs this function
int __attribute__((weak)) main(int argc, char** argv)
{
  UNUSED(argc);
  UNUSED(argv);
  return 0;
}

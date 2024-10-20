#pragma once
#include <cstdio>

extern FILE* dromajo_stderr;
extern void usage_isa();
extern void usage_interactive();
extern void usage(const char *prog, const char *msg);

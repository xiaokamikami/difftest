#ifndef __LOADTRACE_H__
#define __LOADTRACE_H__
#include <iostream>
#include <fstream>
#include <vector>
#include <zstd.h>

#include "difftest.h"
#include "difftrace.h"

void dcompressIOTraceInit();
void dcompressIOTraceFinsh();
int decompressIOTraceDCTX(std::queue<char> &traceQueue);
int decompress(std::queue<char> &traceQueue);
#endif // __LOADTRACE_H__

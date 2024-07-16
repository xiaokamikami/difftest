#ifndef __DIFFTRACE_H__
#define __DIFFTRACE_H__

#include "common.h"
#ifdef CONFIG_DIFFTEST_IOTRACE
#include <zstd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <sys/stat.h>
#include <cstdlib>
#endif // CONFIG_DIFFTEST_IOTRACE

class DiffTrace {
public:
  char trace_name[32];
  bool is_read;

  DiffTrace(const char *trace_name, bool is_read, uint64_t buffer_size = 1024 * 1024);
  ~DiffTrace() {
    if (!is_read) {
      trace_file_next();
    }
    if (buffer) {
      free(buffer);
    }
  }
  bool append(const DiffTestState *trace);
  bool read_next(DiffTestState *trace);

private:
  uint64_t buffer_size;
  uint64_t buffer_count = 0;
  DiffTestState *buffer = nullptr;

  bool trace_file_next();
};

#ifdef CONFIG_DIFFTEST_IOTRACE
typedef struct {
 char *traceInfo;
 uint64_t ptr;
} DifftestIOTrace;

class DiffIOTrace {
public:
  const char *difftest_IOtrace_file = "./difftest/iotrace.zstd";
  int clk_count;

  DiffIOTrace();
  ~DiffIOTrace() {
    difftest_IOtrace_finish();
  }

  void difftest_IOtrace_dump();
  void difftest_IOtrace_finish();

private:
  ZSTD_CCtx *trace_cctx;
  std::ofstream outputFile;
};
#endif // CONFIG_DIFFTEST_IOTRACE

#endif

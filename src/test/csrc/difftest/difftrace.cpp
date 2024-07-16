#include "difftrace.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef CONFIG_DIFFTEST_IOTRACE
#include "difftest-dpic.h"
#endif // CONFIG_DIFFTEST_IOTRACE

DiffTrace::DiffTrace(const char *_trace_name, bool is_read, uint64_t _buffer_size) : is_read(is_read) {
  if (!is_read) {
    buffer_size = _buffer_size;
    buffer = (DiffTestState *)calloc(buffer_size, sizeof(DiffTestState));
  }
  if (strlen(trace_name) > 31) {
    printf("Length of trace_name %s is more than 31 characters.\n", trace_name);
    printf("Please use a shorter name.\n");
    exit(0);
  }
  strcpy(trace_name, _trace_name);
}

bool DiffTrace::append(const DiffTestState *trace) {
  memcpy(buffer + buffer_count, trace, sizeof(DiffTestState));
  buffer_count++;
  if (buffer_count == buffer_size) {
    return trace_file_next();
  }
  return 0;
}

bool DiffTrace::read_next(DiffTestState *trace) {
  if (!buffer || buffer_count == buffer_size) {
    trace_file_next();
  }
  memcpy(trace, buffer + buffer_count, sizeof(DiffTestState));
  buffer_count++;
  // printf("%lu...\n", buffer_count);
  return 0;
}

bool DiffTrace::trace_file_next() {
  static uint64_t trace_index = 0;
  static FILE *file = nullptr;
  if (file) {
    fclose(file);
  }
  char filename[128];
  char *noop_home = getenv("NOOP_HOME");
  snprintf(filename, 128, "%s/%s", noop_home, trace_name);
  mkdir(filename, 0755);
  const char *prefix = "bin";
  snprintf(filename, 128, "%s/%s/%lu.%s", noop_home, trace_name, trace_index, prefix);
  if (is_read) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
      printf("File %s not found.\n", filename);
      exit(0);
    }
    // check the number of traces
    fseek(file, 0, SEEK_END);
    buffer_size = ftell(file) / sizeof(DiffTestState);
    if (buffer) {
      free(buffer);
    }
    buffer = (DiffTestState *)calloc(buffer_size, sizeof(DiffTestState));
    // read the binary file
    Info("Loading %lu traces from %s ...\n", buffer_size, filename);
    fseek(file, 0, SEEK_SET);
    uint64_t read_bytes = fread(buffer, sizeof(DiffTestState), buffer_size, file);
    assert(read_bytes == buffer_size);
    fclose(file);
    buffer_count = 0;
  } else if (buffer_count > 0) {
    Info("Writing %lu traces to %s ...\n", buffer_count, filename);
    FILE *file = fopen(filename, "wb");
    fwrite(buffer, sizeof(DiffTestState), buffer_count, file);
    fclose(file);
    buffer_count = 0;
  }
  trace_index++;
  return 0;
}

#ifdef CONFIG_DIFFTEST_IOTRACE
DifftestIOTrace diffIOTraceBuff;
DiffIOTrace::DiffIOTrace() {
  diffIOTraceBuff.traceInfo = (char *)malloc(MAX_IOTRACE_BUFF_SIZE);
  outputFile.open(difftest_IOtrace_file, std::ios::binary);
  clk_count = 0;
  std::cout << "difftest IO trace init" << std::endl;
  if (!outputFile.is_open()) {
    std::cerr << "Failed to open file: " << difftest_IOtrace_file << std::endl;
  }
  trace_cctx = ZSTD_createCCtx();
  if (!trace_cctx) {
    std::cerr << "Failed to create ZSTD_CCtx" << std::endl;
    return;
  }
}

void DiffIOTrace::difftest_IOtrace_dump() {
  static size_t cLevel = 1; // compression level
  std::vector<char> outputBuffer(MAX_IOTRACE_BUFF_SIZE);
  size_t compressedSize = ZSTD_compressCCtx(trace_cctx, outputBuffer.data(), outputBuffer.size(),
                                            diffIOTraceBuff.traceInfo, diffIOTraceBuff.ptr, cLevel);
  if (ZSTD_isError(compressedSize)) {
    std::cerr << "Compression error: " << ZSTD_getErrorName(compressedSize) << std::endl;
    ZSTD_freeCCtx(trace_cctx);
    return;
  }

  diffIOTraceBuff.ptr = 0;
  outputFile.write(outputBuffer.data(), compressedSize);
}

void DiffIOTrace::difftest_IOtrace_finish() {
  ZSTD_freeCCtx(trace_cctx);
  free(diffIOTraceBuff.traceInfo);
  outputFile.close();
}
#endif // CONFIG_DIFFTEST_IOTRACE

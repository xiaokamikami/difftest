/***************************************************************************************
* Copyright (c) 2020-2023 Institute of Computing Technology, Chinese Academy of Sciences
* Copyright (c) 2020-2021 Peng Cheng Laboratory
*
* DiffTest is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

//#ifdef CONFIG_SIMDIFFTEST
#include "loadtrace.h"

static std::string tracePath = "./difftest/iotrace.zstd";
static std::ifstream traceFile = NULL;
static ZSTD_DCtx* dctx = NULL;

void dcompressIOTraceInit() {
  std::ifstream traceFile(tracePath, std::ios::binary);
  // Open the input file
  if (!traceFile.is_open()) {
    std::cerr << "Error opening input file: " << tracePath << std::endl;
    return;
  }
  dctx = ZSTD_createDCtx();
  if (!dctx) {
    std::cerr << "Error creating ZSTD decompression context" << std::endl;
    return;
  }
}

void dcompressIOTraceFinsh() {
  ZSTD_freeDCtx(dctx);
  traceFile.close();
}

int decompressIOTraceDCTX(std::queue<char> &traceQueue) {
  // Set up buffers
  const size_t bufferSize = ZSTD_DStreamOutSize();// Use ZSTD's recommended output buffer size
  std::vector<char> inputBuffer(bufferSize);
  std::vector<char> outputBuffer(bufferSize);

  // Read and decompress data in a loop
  size_t readBytes;
  if ((readBytes = inputFile.read(inputBuffer.data(), bufferSize).gcount()) > 0) {
    const char* inputPtr = inputBuffer.data();
    size_t remainingInput = readBytes;

    ZSTD_outBuffer output = {outputBuffer.data(), outputBuffer.size(), 0};
    ZSTD_inBuffer input = {inputPtr, remainingInput, 0};

    // Decompress the data
    size_t ret = ZSTD_decompressStream(dctx, &output, &input);
    if (ZSTD_isError(ret)) {
      std::cerr << "Decompression error: " << ZSTD_getErrorName(ret) << std::endl;
      ZSTD_freeDCtx(dctx);
      exit(0);
    }

    if (ret == 0) {
      std::cout << "Decompress parse to the end of the file" << std::endl;
      return 1;
    }

    for (size_t i = 0; i < output.pos; ++i) {
      traceQueue.push(outputBuffer[i]);
    }
  }
  return 0;
}



//#endif // CONFIG_SIMDIFFTEST

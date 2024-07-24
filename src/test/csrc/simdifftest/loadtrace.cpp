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

static std::string tracePath = "./iotrace.zstd";
static std::ifstream traceFile;
static ZSTD_DCtx* dctx = NULL;

void dcompressIOTraceInit() {
  traceFile.open(tracePath, std::ios::binary);
  // Open the input file
  if (!traceFile.is_open()) {
    std::cerr << "Error opening input file: " << tracePath << std::endl;
    exit(0);
  }
  dctx = ZSTD_createDCtx();
  if (!dctx) {
    std::cerr << "Error creating ZSTD decompression context" << std::endl;
    exit(0);
  }
  std::cout << "dcompress init" << std::endl;
}

void dcompressIOTraceFinsh() {
  ZSTD_freeDCtx(dctx);
  traceFile.close();
}

int decompress(std::queue<char> &traceQueue) {
  const size_t inBufferSize = ZSTD_DStreamInSize();
  const size_t outBufferSize = ZSTD_DStreamOutSize();
  std::vector<char> inputBuffer(inBufferSize);
  std::vector<char> outputBuffer(outBufferSize);

  ZSTD_inBuffer input = { inputBuffer.data(), 0, 0 };
  ZSTD_outBuffer output = { outputBuffer.data(), outBufferSize, 0 };
  size_t ret;

  while (true) {
    if (input.pos == input.size && input.size == 0) {
      traceFile.read(inputBuffer.data(), inBufferSize);
      input.size = traceFile.gcount();
      input.pos = 0;

      if (input.size == 0) {
        break; // No more input to read, exit loop
        }
      }

      do {
        output.pos = 0; // Reset output position before decompressing

        ret = ZSTD_decompressStream(dctx, &output, &input);
        if (ZSTD_isError(ret)) {
          std::cerr << "解压错误: " << ZSTD_getErrorName(ret) << std::endl;
          exit(1);
        }
        if(ret == 0)
          return 0;
        std::cout << "解压后 output.pos: " << output.pos << " input.pos: " << input.pos << " ret: " << ret << std::endl;
        for (size_t i = 0; i < output.pos; ++i) {
          traceQueue.push(outputBuffer[i]);
        }
        // If ret == 0, the frame is completely decoded and the loop will exit
      } while (ret != 0 && input.pos < input.size);
  }
  return 1;
}

int decompressIOTraceDCTX(std::queue<char> &traceQueue) {
  // Set up buffers
  static const size_t inbufferSize = ZSTD_DStreamInSize();// Use ZSTD's recommended output buffer size
  static const size_t outbufferSize = ZSTD_DStreamOutSize() * 1024;
  static std::vector<char> inputBuffer(inbufferSize);
  std::vector<char> outputBuffer(outbufferSize);
  //inputBuffer.resize(inbufferSize);
  outputBuffer.resize(outbufferSize);
  // Read and decompress data in a loop
  ZSTD_outBuffer output = {outputBuffer.data(), outbufferSize, 0};
  static ZSTD_inBuffer input = {inputBuffer.data(), 0, 0};
  
  if (input.pos == input.size) {
    inputBuffer.resize(inbufferSize);
    traceFile.read(inputBuffer.data(), inbufferSize);
    input.size = traceFile.gcount();
    input.pos = 0;
    printf("reset input pos \n");
    // 输出当前文件指针位置
    std::streampos currentPos = traceFile.tellg();
    if(currentPos == -1) {
      std::cout << "Decompress parse to the end of the file" << std::endl;
      return 2;
    }
    std::cout << "Current input file position: " << currentPos << std::endl;
  } else if (input.size == 0){
    std::cout << "Decompress parse to the end of the file" << std::endl;
    return 2;
  } else {
    input.size = input.size;
    input.pos = input.pos;
    printf("input %ld %ld\n",input.size , input.pos);
  }

  std::cout << "readBytes " << input.size << " inbuffer size " << inputBuffer.size() << " outbuffer size " << outputBuffer.size() << std::endl;

  // Decompress the data
  size_t ret = ZSTD_decompressStream(dctx, &output, &input);
  if (output.pos == output.size) {
    std::cout << "there might be some data left within internal buffers" << std::endl;
  }
  if (input.pos == input.size) {
    std::cout << "once a block has been processed, more data is needed" << std::endl;
  }

  std::cout << "Dcompress output.pos: " << output.pos << " input.pos: " << input.pos << " ret: " << ret << std::endl;
  if (ZSTD_isError(ret)) {
    std::cerr << "Decompression error: " << ZSTD_getErrorName(ret) << std::endl;
    dcompressIOTraceFinsh();
    exit(0);
  }

  for (size_t i = 0; i < output.pos; ++i) {
    traceQueue.push(outputBuffer[i]);
  }

  if (ret == 0) {
    std::cout << "Decompress parse to the end of the block" << std::endl;
  }
  return 1;
}



//#endif // CONFIG_SIMDIFFTEST

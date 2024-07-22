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
#include <iostream>
#include <fstream>
#include <vector>
#include <zstd.h>

#include "difftest-dpic.h"
#include "difftest.h"
#include "difftrace.h"
#include "loadtrace.h"

static std::queue<char> traceQueue;
static DiffTestState cores[NUM_CORES];

int main() {
  simdifftest_main();
  return 0;
}

void simdifftest_main() {
  dcompressIOTraceInit();

  while (decompressIOTraceDCTX(std::queue<char> &traceQueue) == 0) {
    pars_iotrace(traceQueue);
  }

  dcompressIOTraceFinsh();
}

#define MAX_CLASS_INFO_SIZE 64 * 130

int pars_iotrace(std::queue<char> &traceQueue) {
  char *trace = new char[MAX_IOTRACE_BUFF_SIZE / MAX_IOTRACE_COUNT];
  int end_step = 0;

  while (end == 0) { // difftest state
    int strat_io = 0;
    char str[MAX_CLASS_INFO_SIZE] = {};
    char class_name[32];
    for (size_t i = 0; i < MAX_CLASS_INFO_SIZE; i++) {
      char buff = traceQueue.front();
      if (buff == ':') {
        strat_io = 1;
        strncpy(class_name, str, i);
        printf("get class name %s\n", class_name);
      } else {
        
      }
      str[i] = buff;
    }
  }

  delete[] trace;
  return 0;
}

int dpic_funs_case(char *funs_name) {

}

//#endif // CONFIG_SIMDIFFTEST

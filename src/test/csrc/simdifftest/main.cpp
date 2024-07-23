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

void simdifftest_main();
int pars_iotrace(std::queue<char> &traceQueue);
int dpic_funs_case(char *funs_name);

int main() {
  simdifftest_main();
  return 0;
}

void simdifftest_main() {
  dcompressIOTraceInit();
  
  while (1) {
    int result = decompressIOTraceDCTX(traceQueue);
    if (result == 1) {
      pars_iotrace(traceQueue);
    } else if (result == 2) {
      break;
    } else if (result == 3){
      pars_iotrace(traceQueue);
      break;
    }
  }

  dcompressIOTraceFinsh();
}

#define MAX_CLASS_INFO_SIZE 64 * 130

int pars_iotrace(std::queue<char> &traceQueue) {
  char *trace = new char[MAX_IOTRACE_BUFF_SIZE / MAX_IOTRACE_COUNT];
  int end_step = 0;

  while (end_step == 0) { // difftest state
    char class_info[MAX_CLASS_INFO_SIZE] = {};
    char class_name[32] = {};
    int name_size = 0;
    for (size_t i = 0; i < MAX_CLASS_INFO_SIZE; i++) {
      if (end_step == 1)
        break;
      char buff = traceQueue.front();
      traceQueue.pop();
      //std::cout << "get buff" << buff << std::endl;
      if (buff == ':') {
      //printf("get class name %s\n", class_name);
      memset(class_name, 0, sizeof(class_name));
      name_size = 0;
        for (size_t i = 0; i < MAX_CLASS_INFO_SIZE; i++) {
          char buff2 = traceQueue.front();
          traceQueue.pop();
          if (buff2 == ';') {
            //printf("Read through the data of class\n");
            break;
          } else if (buff2 == '.') {
            //printf("Read through a class of data\n");
            end_step = 1;
            break;
          }
          class_info[i] = buff2;
        }
      } else {
        class_name[name_size] = buff;
        name_size ++;
      }

    }
  }

  delete[] trace;
  return 0;
}

int dpic_funs_case(char *funs_name) {

}

//#endif // CONFIG_SIMDIFFTEST

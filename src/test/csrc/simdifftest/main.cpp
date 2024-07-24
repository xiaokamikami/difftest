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
#include "sqltrace.h"

#define MAX_CLASS_INFO_SIZE 64 * 130 * NUM_CORES
#define MAX_CLASS_SIZE 22
#define MAX_CLASS_NAME_SIZE 32
#define USE_IOTRACE_SQLLITE

const char *sql_file_path = "./iotrace.db";
IoTraceDb *sqlbase = NULL;

static std::queue<char> traceQueue;
static DiffTestState cores[NUM_CORES];

void simdifftest_main();
int pars_iotrace(std::queue<char> &traceQueue);
int dpic_funs_case(char *funs_name);
int sql_init(const char *file_path);

int main() {
#ifdef USE_IOTRACE_SQLLITE
  sql_init(sql_file_path);
#endif // USE_TRACE_SQLLITE
  simdifftest_main();
  return 0;
}

void simdifftest_main() {
  dcompressIOTraceInit();

  while (1) {
    int result = decompressIOTraceDCTX(traceQueue);
    if (result == 1) {
      while (traceQueue.size() > MAX_CLASS_INFO_SIZE * MAX_CLASS_SIZE) {
        pars_iotrace(traceQueue);
      }
      //std::cout << "queue size " << traceQueue.size() << std::endl;
      //exit(0);
    } else if (result == 2) {
      //while(traceQueue.size()) {
      //  pars_iotrace(traceQueue);
      //}
      break;
    } else if (result == 3){
      pars_iotrace(traceQueue);
      break;
    }
  }

  dcompressIOTraceFinsh();
}

int pars_iotrace(std::queue<char> &traceQueue) {
  static uint64_t step_count = 0;

  char class_name[MAX_CLASS_NAME_SIZE] = {};
  char class_info[MAX_CLASS_INFO_SIZE] = {};
  int end_step = 0;
  int name_str_size = 0;
  while (end_step == 0) {
    int class_count = 0;
    for (size_t i = 0; i < MAX_CLASS_SIZE; i++) {
      if (end_step == 1) {
        exit(0);
        break;
      }
      char buff = traceQueue.front();
      traceQueue.pop();
      //std::cout << "get buff" << buff << std::endl;
      if (buff == ':') {
        printf("get class name %s\n", class_name);
        for (size_t i = 0; i < MAX_CLASS_INFO_SIZE; i++) {
          char buff2 = traceQueue.front();
          traceQueue.pop();
          if (buff2 == ';' || buff2 == '.') {
            // push info to sql.db
            if (class_count == 0) {
              //sqlbase->insertBatch(class_name, sizeof(class_name), class_info, sizeof(class_info));  
            } else {
              //sqlbase->alterTable(class_count, class_name, sizeof(class_name), class_info, sizeof(class_info));
            }
            sqlbase->insertBatch(class_name, sizeof(class_name), class_info, sizeof(class_info));  
            if (buff2 == '.') {
              end_step = 1;
              step_count ++;
              class_count = 0;
            } else {
              class_count ++;
            }
            //printf("Read through the data of class %s\n", class_info);
            memset(class_name, 0, MAX_CLASS_NAME_SIZE);
            memset(class_info, 0, MAX_CLASS_INFO_SIZE);
            name_str_size = 0;
            break;
          } else {
            class_info[i] = buff2;  
          }
        }
      } else {
        class_name[name_str_size ++] = buff;
      }
    }
  }
  return 0;
}

int dpic_funs_case(char *funs_name) {

}

int sql_init(const char *file_path) {
  sqlbase = new IoTraceDb;
  sqlbase->createData(file_path);

  // delete sqlbase;
  // sqlbase = nullptr;
  return 0;
}

//#endif // CONFIG_SIMDIFFTEST

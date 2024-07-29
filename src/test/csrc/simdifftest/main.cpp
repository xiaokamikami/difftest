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
//#define PROF_SDL
#define MAX_CLASS_INFO_SIZE 64 * 130 * NUM_CORES
#define MAX_CLASS_SIZE 22
#define MAX_CLASS_NAME_SIZE 20
#define USE_IOTRACE_SQLLITE
#define USE_THREAD

//#ifdef CONFIG_SIMDIFFTEST
#include <iostream>
#include <fstream>
#include <vector>
#include <zstd.h>
#include <thread>

#ifdef PROF_SDL
#include <chrono>
#endif // PROF_SDL

#include "difftest-dpic.h"
#include "difftest.h"
#include "difftrace.h"
#include "loadtrace.h"
#include "sqltrace.h"

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
  std::thread dbThread;
  while (1) {
#ifdef PROF_SDL
    static int sdl_prof_count = 0;
    static double prof_tims = 0;
    auto start = std::chrono::high_resolution_clock::now();
#endif // PROF_SDL
    int result = decompressIOTraceDCTX(traceQueue);
    if (result == 1) {
      sqlbase->sqlite_stmt_begin();
      while (traceQueue.size() > MAX_CLASS_INFO_SIZE * MAX_CLASS_SIZE) {
        pars_iotrace(traceQueue);
      }
#ifdef USE_THREAD
      if (dbThread.joinable()) {
        dbThread.join();
      }
      dbThread = std::thread(&IoTraceDb::sqlite_stmt_exec, sqlbase);
#else
      sqlbase->sqlite_stmt_exec();
#endif // USE_THREAD
    } else if (result == 2) {
      break;
    }
#ifdef PROF_SDL
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    prof_tims += elapsed.count();
    sdl_prof_count ++;
    if (sdl_prof_count == 10) {
      std::cout << "Time taken by a trace parse to dump the database: " << prof_tims / 10.0 << "S" << std::endl;
      sdl_prof_count = 0;
      prof_tims = 0;
    }
#endif // PROF_SDL
  }
#ifdef USE_THREAD
  if (dbThread.joinable())
    dbThread.join();
#endif // USE_THREAD
  dcompressIOTraceFinsh();
  delete sqlbase;
}

void sql_exec() {
  sqlbase->sqlite_stmt_exec();
}

int pars_iotrace(std::queue<char> &traceQueue) {
#ifdef PROF_SDL
  static int sdl_prof_count = 0;
  static double prof_tims = 0;
  auto start = std::chrono::high_resolution_clock::now();
#endif // PROF_SDL
  static uint64_t step_count = 0;
  static char class_name[MAX_CLASS_NAME_SIZE] = {};
  static char class_info[MAX_CLASS_INFO_SIZE] = {};

  int end_step = 0;
  int name_str_size = 0;
  int class_count = 0;
  while (end_step == 0) {
    for (size_t i = 0; i < MAX_CLASS_SIZE; i++) {
      if (end_step == 1) {
#ifdef PROF_SDL
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        prof_tims += elapsed.count();
        sdl_prof_count ++;
        if (sdl_prof_count == 500) {
          std::cout << "trace String parsing Elapsed time: " << prof_tims / 500.0 << "S" << std::endl;
          sdl_prof_count = 0;
          prof_tims = 0;
        }
#endif // PROF_SDL
        break;
      }                   
      char buff = traceQueue.front();
      traceQueue.pop();
      if (buff == ':') {
        //printf("get class name %s\n", class_name);
        for (size_t i = 0; i < MAX_CLASS_INFO_SIZE; i++) {
          char buff2 = traceQueue.front();
          traceQueue.pop();
          if (buff2 == ';' || buff2 == '.') {
            // push info to sql.db
            if (buff2 == '.') {
              end_step = 1;
              class_count = 0;
              sqlbase->update_head(++step_count);
            } else if (class_count == 0) {
              sqlbase->insert_batch(class_name, sizeof(class_name), class_info, sizeof(class_info));
            } else {
              sqlbase->alter_table(class_count, class_name, sizeof(class_name), class_info, sizeof(class_info));
            }
            class_count ++;
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
  sqlbase->create_data(file_path);

  return 0;
}

//#endif // CONFIG_SIMDIFFTEST

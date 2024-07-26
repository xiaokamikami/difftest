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

//#ifdef USE_IOTRACE_SQLLITE
#include <sqlite3.h>
#include <string.h>
#include <iostream>
#include <string>

#include "sqltrace.h"

using namespace std;

void IoTraceDb::drop() {
  const char *drop = "drop table iotrace";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(conn,drop,strlen(drop),&stmt,nullptr ) != SQLITE_OK) {
    close();
    sqlite3_finalize(stmt);
    return;
  }
  if (sqlite3_step(stmt) == SQLITE_DONE) {
    cout << "The data table was destroyed successfully" << endl;
  }
  sqlite3_finalize(stmt);
  close();
}

void IoTraceDb::alter_table(int size, const char *name, const int name_len, const char *value_str, const int value_len) {
  sqlite3_stmt *stmt;

  char update_sql[80];
  sprintf(update_sql, "UPDATE iotrace SET %s = ? WHERE ID = %d;", name, head);
  if (sqlite3_prepare_v2(conn, update_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(conn));
    close();
    exit(0);
  }

  // writing parameters
  sqlite3_bind_text(stmt, 1, value_str, value_len, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    close();
    sqlite3_finalize(stmt);
    fprintf(stderr, "Failed to bind text: %s\n", sqlite3_errmsg(conn));
    exit(0);
    return;
  }
  //printf("alter insert info succeed\n");
  sqlite3_finalize(stmt);
}

// Checks if the ID already exists
int IoTraceDb::id_exists(int id) {
  sqlite3_stmt *stmt;
  static const char *sql = "SELECT COUNT(*) FROM iotrace WHERE ID = ?;";
  if (sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(conn));
    return 0;
  }
  sqlite3_bind_int(stmt, 1, id);

  int count = 0;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    count = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return count > 0;
}

void IoTraceDb::insert_batch(const char *name, const int name_len, const char *value_str, const int value_len) {
  char insert[80];
  sprintf(insert, "insert into iotrace (ID, %s) values(?,?)", name);
  sqlite3_stmt *stmt2 = nullptr;
  if (sqlite3_prepare_v2(conn, insert, strlen(insert), &stmt2, nullptr) != SQLITE_OK) {
    printf("%s\n",insert);
    close();
    exit(0);
    return;
  }

  // writing parameters
  sqlite3_bind_int(stmt2, 1, head);
  sqlite3_bind_text(stmt2, 2, value_str, value_len, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt2) != SQLITE_DONE) {
    close();
    sqlite3_finalize(stmt2);
    fprintf(stderr, "Failed to bind text: %s\n", sqlite3_errmsg(conn));
    exit(0);
    return;
  }
  sqlite3_reset(stmt2);
  sqlite3_finalize(stmt2);
}

void IoTraceDb::create_data(const char *file_path) {
  rc = sqlite3_open(file_path, &conn);    
  if (rc != SQLITE_OK) {
    close();
    cout << "Database creation failed !!" <<endl;
    return ;
  }
  // SQL CREAT
  const char * createTable = "create table iotrace(ID INTEGER PRIMARY KEY NOT NULL,RefillEvent TEXT,L1TLBEvent TEXT,\
  InstrCommit TEXT,LoadEvent TEXT,TrapEvent TEXT,ArchIntRegState TEXT,ArchFpRegState TEXT,ArchVecRegState TEXT,\
  ArchEvent TEXT,CSRState TEXT,HCSRState TEXT,DebugMode TEXT,VecCSRState TEXT,IntWriteback TEXT,FpWriteback TEXT,\
  VecWriteback TEXT,L2TLBEvent TEXT,AtomicEvent TEXT,LrScEvent TEXT,SbufferEvent TEXT,StoreEvent TEXT)";

  sqlite3_stmt *stmt = nullptr;

  // Precompiling SQL statements
  if (sqlite3_prepare_v2(conn,createTable,strlen(createTable),&stmt,nullptr) != SQLITE_OK) {
    cout << "Precompilation failure" << endl;
    sqlite3_finalize(stmt);
    close();
    return;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    cout << "Execution failure" << endl;
    close();
    return;
  }
  sqlite3_finalize(stmt);
  cout << "Create DATABASE and data table successfully!!" << endl;
         
}

//#endif // USE_IOTRACE_SQLLITE

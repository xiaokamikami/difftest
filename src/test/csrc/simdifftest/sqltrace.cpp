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

void IoTraceDb::updateData(const int id,const int n) {
  const char *field = nullptr;
  const char *update ; 
  char str[64] = {0};
  if (1 == n) {
    cout<<"约束主键不能修改"<<endl;
    // field = "ID";   
    //  update = "update test set ID = %d where ID = %d";
    //  sprintf(str,update,n,id);
  } else if (2 == n) {
    field = "name";    
    update = "update test set name = '%s' where ID = %d";
    sprintf(str,update,"yxg",id);
  } else {
    field = "value";    
    update = "update test set width = %d where ID = %d";
    sprintf(str,update,1,id);
  };

  sqlite3_stmt *stmt = nullptr;
  if(sqlite3_prepare_v2(conn,str,strlen(str),&stmt,nullptr) != SQLITE_OK){
    sqlite3_finalize(stmt);
    close();
    return ;
  }
  if(sqlite3_step(stmt)!=SQLITE_DONE){
    sqlite3_finalize(stmt);
    close();
    return;
  }
  sqlite3_finalize(stmt); 
}

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

void IoTraceDb::alterTable(int size, const char *name, const int name_len, const char *value_str, const int value_len) {
  sqlite3_stmt *stmt;
  // Adding new columns
  char add_table_sql[512];
  sprintf(add_table_sql, "alter table iotrace add (data%d varchar2(name_len) default not null,\
          value%d varchar2(value_len) default not null);", size, size);
  if (sqlite3_exec(conn, add_table_sql, NULL, NULL, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to add column: %s\n", sqlite3_errmsg(conn));
    close();
  }

  // Update the new column
  const char *update_sql = "UPDATE iotrace SET new_column = ? WHERE id = ?;";
  if (sqlite3_prepare_v2(conn, update_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(conn));
    close();
  }

  sqlite3_finalize(stmt);
}

void IoTraceDb::insertBatch(const char *name, const int name_len, const char *value_str, const int value_len) {
  static uint64_t head = 0;
  // Start a transaction
  const char *begin = "begin transaction";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(conn, begin, strlen(begin), &stmt, nullptr) != SQLITE_OK) {
    close();
    cout << "Precompile thing failed" << endl;
    return;    
  }
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    close();
    cout << "Failure to execute things" << endl;
    return;
  }
  sqlite3_finalize(stmt);

  // Insert data based on bound variables
  const char *insert = "insert into iotrace values(?,?,?)";
  sqlite3_stmt *stmt2 = nullptr;
  if (sqlite3_prepare_v2(conn, insert, strlen(insert), &stmt2, nullptr) != SQLITE_OK) {
    close();
    return;
  }

  // writing parameters
  sqlite3_bind_int(stmt2, 1, head);
  sqlite3_bind_text(stmt2, 2, name, name_len, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt2, 3, value_str, value_len, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt2)!= SQLITE_DONE) {
    close();
    sqlite3_finalize(stmt2);
    return;
  }
  sqlite3_reset(stmt2);
  cout << "Insert succeed!" <<endl;

  sqlite3_finalize(stmt2);

  //提交事务
  const char * commit= "commit";
  sqlite3_stmt *stmt3 = nullptr;
  if(sqlite3_prepare_v2(conn,commit,strlen(commit),&stmt3,nullptr)!= SQLITE_OK){
    close();
    sqlite3_finalize(stmt3);
    return;    
  }
  if(sqlite3_step(stmt3)!=SQLITE_DONE){
    close();
    sqlite3_finalize(stmt3);
    return;    
  }
  sqlite3_finalize(stmt3);

  head++;
}

void IoTraceDb::createData(const char *file_path) {
  rc = sqlite3_open(file_path, &conn);    
  if (rc != SQLITE_OK) {
    close();
    cout<<"创建数据库失败！！"<<endl;
    return ;
  }
  // SQL语句
  /*
  const char *createTable = "create table iotrace(" \
      "ID INT PRIMARY KEY NOT NULL,"\
      "name TEXT NOT NULL," \
      "age INT NOT NULL," \
      " );";*/
  const char * createTable = "create table iotrace(ID INT PRIMARY KEY NOT NULL,name TEXT,value REAL)";
  sqlite3_stmt *stmt = nullptr;

  // Precompiling SQL statements
  if (sqlite3_prepare_v2(conn,createTable,strlen(createTable),&stmt,nullptr) != SQLITE_OK) {
    cout<<"预编译失败"<<endl;
    sqlite3_finalize(stmt);
    close();
    return;
  }
  //执行SQL语句
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    cout<<"执行失败"<<endl;
    close();
    return;
  }
  sqlite3_finalize(stmt);
  cout<<"创建数据库和数据表成功!!"<<endl;
         
}

//#endif // USE_IOTRACE_SQLLITE

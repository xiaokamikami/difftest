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
#include <sqlite3.h>

class IoTraceDb{
public:
    IoTraceDb() {

    };

    ~IoTraceDb() {

    }
    // Create the database and tables
    void createData(const char *file_path);
    // Insert data
    void alterTable(int size, const char *name, const int name_len, const char *value_str, const int value_len);
    // Update data (update the value of the NTH column of the id record)
    void updateData(const int id,const int n);
    // Display open things, batch insert data
    void insertBatch(const char *name, const int name_len, const char *value_str, const int value_len);
    // Closing the database
    void close() {
        sqlite3_close(conn);    
        return;
    }
    // Clear the database tables
    void drop();

private:
    sqlite3 *conn = nullptr;
    int rc;
    int count = 10;
};


int sql_init(char *file_path);

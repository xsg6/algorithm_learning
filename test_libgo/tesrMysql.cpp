#include <mysql/mysql.h>
#include <iostream>
using namespace std;

int main() {
    // 1. 初始化 MySQL 连接句柄
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        cerr << "初始化连接失败：" << mysql_error(conn) << endl;
        return 1;
    }

    // 2. 连接 MySQL 服务器
    // 参数：连接句柄、主机地址、用户名、密码、数据库名、端口、unix_socket、客户端标志
    if (!mysql_real_connect(
        conn, 
        "localhost",  // 主机地址（本地可填 "localhost" 或 "127.0.0.1"）
        "root",       // 用户名
        "123456",  // 密码
        "testLibgo",    // 要连接的数据库名
        3306,         // 端口（默认 3306）
        NULL,         // unix_socket（本地连接可填 NULL）
        0             // 客户端标志（0 为默认）
    )) {
        cerr << "连接失败：" << mysql_error(conn) << endl;
        mysql_close(conn);  // 关闭连接句柄
        return 1;
    }

    cout << "连接 MySQL 成功！" << endl;

    // 3. 执行 SQL 语句（示例：查询）
    const char* sql = "SELECT id, name FROM users LIMIT 5";
    if (mysql_query(conn, sql) != 0) {
        cerr << "查询失败：" << mysql_error(conn) << endl;
        mysql_close(conn);
        return 1;
    }

    // 4. 获取查询结果
    MYSQL_RES* result = mysql_store_result(conn);  // 存储完整结果集
    if (result == NULL) {
        cerr << "获取结果失败：" << mysql_error(conn) << endl;
        mysql_close(conn);
        return 1;
    }

    // 5. 遍历结果集
    MYSQL_ROW row;  // 一行数据（数组形式）
    unsigned int num_fields = mysql_num_fields(result);  // 列数
    while ((row = mysql_fetch_row(result))) {  // 逐行获取
        for (unsigned int i = 0; i < num_fields; i++) {
            cout << (row[i] ? row[i] : "NULL") << "\t";  // 输出字段值（注意 NULL 处理）
        }
        cout << endl;
    }

    // 6. 释放资源
    mysql_free_result(result);  // 释放结果集
    mysql_close(conn);          // 关闭连接

    return 0;
}


#include <iostream>
#include <stdlib.h>
#include "Utils/DB/DBOpr.h"
#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
using namespace std;

DBOpr::DBOpr(const string& server, const string& user, const string& password, const string& dataBase, const string& sock)
{
        m_Server   = server;
        m_User     = user;
        m_Password = password;
        m_DataBase = dataBase;
        m_Sock     = sock;
        m_Conn = mysql_init(NULL);
}

DBOpr::DBOpr()
{
        m_Conn = mysql_init(NULL);
}

DBOpr::~DBOpr()
{
}

int DBOpr::Init()
{
        int ret = 0;

        /* 读取配置信息，确认归档的参数 */
        IniParser iniOpr("/etc/scigw/default/Mysql.cnf");
        ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        /* mysql配置信息 */
        ret = iniOpr.GetVal("mysql", "server", m_Server);
        if (ret < 0) {
                LOG_ERROR("server GetVal Error! ret=" << ret);
                return ret;
        }

        ret = iniOpr.GetVal("mysql", "user", m_User);
        if (ret < 0) {
                LOG_ERROR("server GetVal Error! ret=" << ret);
                return ret;
        }

        ret = iniOpr.GetVal("mysql", "password", m_Password);
        if (ret < 0) {
                LOG_ERROR("password GetVal Error! ret=" << ret);
                return ret;
        }

        ret = iniOpr.GetVal("mysql", "database", m_DataBase);
        if (ret < 0) {
                LOG_ERROR("dataBase GetVal Error! ret=" << ret);
                return ret;
        }

        ret = iniOpr.GetVal("mysql", "socket", m_Sock);
        if (ret < 0) {
                LOG_ERROR("socket GetVal Error! ret=" << ret);
        }
        return 0;
}

int  DBOpr::Connect()
{
        int ret = 0;
        ret = Connect(m_Conn, m_Server, m_User, m_Password, m_DataBase, m_Sock);
        if (ret < 0) {
                LOG_ERROR("Connect Error!");
        }
        return ret;
}

int DBOpr::Connect(MYSQL* conn, const string& server, const string& user, 
                                const string& password, const string& dataBase, 
                                const string& sock)
{
        MYSQL* ret = mysql_real_connect(conn, server.c_str(), user.c_str(), 
                                password.c_str(), dataBase.c_str(), 
                                0, sock.c_str(), 0);
        if (ret == NULL) {
                LOG_ERROR("mysql_real_connect error!" << mysql_error(m_Conn));
                return -1;
        }
        return 0;
}

int DBOpr::Query(const string& sql)
{
        int ret = mysql_query(m_Conn, sql.c_str());
        if (ret != 0) {
                //Close();
                LOG_ERROR("mysql_query error! errInfo: " << mysql_error(m_Conn)
                                        << "; sqlCmd=" << sql);
                return -1;
        }
        return 0;
}

int DBOpr::Query(const string& sql, Record& record)
{
        int ret = mysql_query(m_Conn, sql.c_str());
        if (ret != 0) {
                //Close();
                LOG_ERROR("mysql_query error!" << mysql_error(m_Conn));
                return -1;
        }

        MYSQL_RES* resultSet = mysql_store_result(m_Conn);
        MYSQL_ROW  row       = mysql_fetch_row(resultSet);

        record.Path       = row[0];
        record.LineNum    = atoi(row[1]);
        if (atoi(row[2]) == 0) {
                record.IsFinished = UNFINISH;
        } else {
                record.IsFinished = FINISHED;
        }
        return 0;
}

int DBOpr::Query(const string& sql, vector<vector<string> >& record)
{
        int ret = mysql_query(m_Conn, sql.c_str());
        if (ret != 0) {
                //Close();
                LOG_ERROR("mysql_query error! errInfo: " << mysql_error(m_Conn));
                return -1;
        }

        MYSQL_ROW  row;
        MYSQL_RES* resultSet;
        vector<string> fieldSet;
        resultSet = mysql_store_result(m_Conn);

        while ((row = mysql_fetch_row(resultSet)) != NULL) {
                fieldSet.clear();
                for (size_t i = 0; i < mysql_num_fields(resultSet); ++i) {
                        if (row[i]) {
                                fieldSet.push_back(row[i]);
                        } else {                                //字段值为NULL
                                fieldSet.push_back("NULL");
                        }
                }
                record.push_back(fieldSet);
        }

        return 0;
}

void DBOpr::Close()
{
        mysql_close(m_Conn);
}

#ifndef _DB_OPR_H_
#define _DB_OPR_H_

#include <string>
#include <vector>
#include <mysql/mysql.h>
#include "Archive/DataArchive/DBInfo.h"

using std::string;
using std::vector;

class DBOpr
{
public:
        DBOpr(const string& server, const string& user, const string& password, const string& dataBase, const string& sock);
        DBOpr();
        ~DBOpr();

        int  Init();
        void Close();
        int  Query(const string& sql);
        int  Query(const string& sql, Record& record);
        int  Query(const string& sql, vector<vector<string> >& record);
        int  Connect();

private:        
        int  Connect(MYSQL* conn, const string& server, const string& user, 
        const string& password, const string& dataBase, const string& sock);
private:
        MYSQL*  m_Conn;
        string  m_Server;
        string  m_User;
        string  m_Password;
        string  m_DataBase;
        string  m_Sock;
};

#endif

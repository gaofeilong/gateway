#ifndef _DB_MGR_H_
#define _DB_MGR_H_

#include <vector>
#include <string>

using std::vector;
using std::string;

class DBOpr;

class DBMgr {
public:
        DBMgr();
        ~DBMgr();

public:
        /*
         * @note 获取集群中ip列表
         */        
        int GetIpListByLocal(vector<string>& ipList); 

        /*
         * @note 通过ipList中的Ip获取其中所有指向该Ip的mysql链接
         */
        int CreateAllDbConn(const vector<string>& ipList, vector<DBOpr*>& dbConnList);
        
        /*
         * @note 在执行ipList所有mysql中执行sql
         */
        int ExecuteSqlToAllDb(const vector<string>& ipList, const string& sql);
};

#endif //_DB_MGR_H_

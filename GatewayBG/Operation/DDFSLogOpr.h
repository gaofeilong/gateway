#ifndef _DDFS_LOG_OPR_H_
#define _DDFS_LOG_OPR_H_

#include <map>
#include <vector>
#include <string>
#include "Utils/DB/DBOpr.h"

using std::map;
using std::string;
using std::vector;

class DDFSLogOpr {
public:
        DDFSLogOpr();
        ~DDFSLogOpr();

public:
        /*
         * @note 该接口会把本地的ddfs日志输入数据中
         */
        int InsertLogToDb();

private:
        /*
         * @note 获取数据库操作指针
         */
        int GetDBOpr(DBOpr** dbOpr);

        /*
         * @note 通过Ip获取指定Ip上的挂载点和配置文件路径
         */
        int GetMpInfoByIp(const string& ip, map<string, string>& mpInfo);

        /*
         * @note 通过配置文件获取该挂载点对应的日志
         */
        int GetLogInfo(const string& config, const string& type, vector<string>& logList);

        /*
         * @note 把指定类型的日志输入数据库中
         */
        int InsertFixedType(const string& configPath, const string& mp, 
                                const string& type, const string& ip);

        /*
         * @note 对数据库的实际操作，只更新数据库中没有的日志。已经存在的不会再重新插入
         */
        int DoInsert(size_t lineNum, const string& ip, const string& type, 
                        const string& mp, const vector<string>& logList);

};

#endif

#include "DDFSLogOpr.h"
#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

#define ERROR "ERROR"
#define WARN  "WARN"

DDFSLogOpr::DDFSLogOpr()
{

}

DDFSLogOpr::~DDFSLogOpr()
{
}

int DDFSLogOpr::GetLogInfo(const string& config, const string& type, vector<string>& logList)
{
        int ret = 0;
        ChildProcessOpr cpo;
        DirFileOpr dfo;

        string logPath;
        dfo.GetParPath(config, logPath);
        logPath += "/ddfs.log";

        string cmd,cmd1;
        cmd1 = "| awk -F [@] '{print $1}' | awk -F ] '{print $2}'";
        cmd  = "grep -r \"\\<" + type + "\\>\" " + logPath + cmd1;

        ret = cpo.Popen(cmd, logList);
        if (ret < 0) {
                LOG_ERROR("Popen Error! cmd=" << cmd);
        }
        return ret;
}

int DDFSLogOpr::GetDBOpr(DBOpr** dbOpr)
{
        *dbOpr = new DBOpr;
        int ret = (*dbOpr)->Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        ret = (*dbOpr)->Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error!");
        }
        return ret;
}

int DDFSLogOpr::GetMpInfoByIp(const string& ip, map<string, string>& mpInfo)
{
        int ret = 0;
        DBOpr* dbOpr = NULL;
        ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }
        
        vector<vector<string> > mpList;
        string sql = "select mp_path,config_path from sci_mp_status where ip_addr='" + ip + "'";
        ret = dbOpr->Query(sql, mpList);
        if (ret < 0) {
                LOG_ERROR("Query Error!");
                dbOpr->Close();
                return ret;
        }
        dbOpr->Close();

        for (size_t i=0; i<mpList.size(); ++i) {
                mpInfo.insert(make_pair(mpList[i][0], mpList[i][1]));
        }
        return ret;
}

int DDFSLogOpr::DoInsert(size_t lineNum, const string& ip, 
                        const string& type, const string& mp, 
                        const vector<string>& logList)
{
        int ret = 0;
        DBOpr* dbOpr = NULL;
        ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        string sql;
        for (size_t i=0; i<logList.size(); ++i) {
                /* 跳过已经记录的日志 */
                if (i<lineNum && lineNum!=0) {
                        continue;
                }
                sql = "insert into sci_ddfs_log values('" + ip + "','" + mp + "','"
                        + type + "','"+logList[i] + "')";
                ret = dbOpr->Query(sql);
                if (ret < 0) {
                        LOG_ERROR("Query Error! sql=" << sql);
                        dbOpr->Close();
                        return ret;
                }
        }
        dbOpr->Close();
        return ret;
}

int DDFSLogOpr::InsertFixedType(const string& configPath, const string& mp,
                        const string& type, const string& ip)
{
        int ret = 0;
        vector<string> curLogList;
        ret = GetLogInfo(configPath, type, curLogList);
        if (ret < 0) {
                LOG_ERROR("GetDDFSLogInfo Error!");
                return ret;
        }
        /* 查看数据库信息是否同现在日志个数相同 */
        DBOpr* dbOpr = NULL;
        ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        vector<vector<string> > recordLogList;
        string sql = "select count(*) from sci_ddfs_log where type='" + 
                        type + "'and ip_addr='" + ip + "' and mp='" + mp + "'";
        ret = dbOpr->Query(sql, recordLogList);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sql);
                dbOpr->Close();
                return ret;
        }
        dbOpr->Close();

        int recordSize = atoi(recordLogList[0][0].c_str());
        int curSize    = curLogList.size();

        if (recordSize == curSize) {
                return ret;
        }
        
        ret = DoInsert(recordSize, ip, type, mp, curLogList);
        if (ret < 0) {
                LOG_ERROR("DoInsert Error!");
        }
        return ret;
}

int DDFSLogOpr::InsertLogToDb()
{
        int ret = 0;
        BaseOpr baseOpr;
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error!");
                return ret;
        }

        string ip;
        ret = baseOpr.GetIp(networkId, ip);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                return ret;
        }

        /* 获取本机挂载点和配置文件路径 */
        map<string, string> mpInfo;
        ret = GetMpInfoByIp(ip, mpInfo);
        if (ret < 0) {
                LOG_ERROR("GetMpInfoByIp Error!");
                return ret;
        }

        map<string,string>::iterator mssIter = mpInfo.begin();
        for (; mssIter != mpInfo.end(); ++mssIter) {
                ret = InsertFixedType(mssIter->second, mssIter->first, ERROR, ip);
                if (ret < 0) {
                        LOG_ERROR("InsertFixedType Error!" << mssIter->second);
                        return ret;
                }
                ret = InsertFixedType(mssIter->second, mssIter->first, WARN, ip);
                if (ret < 0) {
                        LOG_ERROR("InsertFixedType Error!" << mssIter->second);
                        return ret;
                }
        }
        return ret;
}

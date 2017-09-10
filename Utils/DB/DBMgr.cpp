#include "DBMgr.h"
#include "Utils/DB/DBOpr.h"
#include "Utils/Log/Log.h"

DBMgr::DBMgr()
{

}

DBMgr::~DBMgr()
{

}

int DBMgr::GetIpListByLocal(vector<string>& ipList)
{
        int ret = 0;
        /* 获取ip集群中ip列表 */
        DBOpr dbOpr("127.0.0.1", "root", "", "scidata", "/var/lib/mysql/mysql.sock");
        dbOpr.Init();

        ret = dbOpr.Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error!" );
                return -1;
        }

        vector<vector<string> > gwInfo;

        string sql = "select ip_addr from sci_gw_node_info";
        ret = dbOpr.Query(sql, gwInfo);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sql);
                return -2;
        }

        for (size_t i=0; i < gwInfo.size(); ++i) {
                ipList.push_back(gwInfo[i][0]);
        }
        dbOpr.Close();

        return 0;
}

int DBMgr::CreateAllDbConn(const vector<string>& ipList, vector<DBOpr*>& dbConnList)
{
        int ret = 0;
        /* 获取ipList中mysql的链接，放入dbConnList */

        vector<string>::const_iterator vscIter = ipList.begin();
        for ( ; vscIter != ipList.end(); ++vscIter ) {
                /* 获取DB对象 */
                DBOpr* dbOpr = new DBOpr(*vscIter, "root", "", "scidata", "/var/lib/mysql/mysql.sock");
                dbOpr->Init();
                ret = dbOpr->Connect();
                if (ret < 0) {
                        //没有同步到数据库中
                        LOG_ERROR("Connect Error! can't sysnc ip=" << *vscIter );
                        continue;
                }
                dbConnList.push_back(dbOpr);
        }
        return 0;
}

int DBMgr::ExecuteSqlToAllDb(const vector<string>& ipList, const string& sql)
{
        int ret = 0;
        /* 1. 获取ipList中的mysql链接 */
        vector<DBOpr*> dbList;
        ret = CreateAllDbConn(ipList, dbList);
        if (ret < 0) {
                LOG_ERROR("CreateAllDbConn Error!");
                return -1;
        }

        /* 2. 在所有节点中插入新节点信息 */
        vector<DBOpr*>::iterator vdIter = dbList.begin();
        for ( ; vdIter != dbList.end(); ++vdIter ) {
                ret = (*vdIter)->Query(sql);
                if (ret < 0) {
                        LOG_ERROR("Query Error! sql=" << sql);
                        return -2;
                }
                /* 释放资源 */
                (*vdIter)->Close();
                delete (*vdIter);
        }
        return 0;
}

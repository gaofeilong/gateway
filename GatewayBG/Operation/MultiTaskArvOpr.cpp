#include <vector>
#include <stdio.h>

#include "Utils/Log/Log.h"
#include "Utils/DB/DBOpr.h"
#include "Utils/DB/DBMgr.h"
#include "MultiTaskArvOpr.h"
#include "Config/IniParser.h"
#include "Config/PathPairParser.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

using std::vector;

MultiTaskArvOpr::MultiTaskArvOpr()
{

}

MultiTaskArvOpr::~MultiTaskArvOpr()
{

}

int MultiTaskArvOpr::GetMaxID(string& maxId)
{
        DBOpr* dbOpr = NULL;
        int ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                if (dbOpr != NULL) {
                        delete dbOpr;
                }
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        vector<vector<string> > arvInfoVec;
        string sql = "select ifnull(max(id), 0) as ID from sci_archive_info";
        ret = dbOpr->Query(sql, arvInfoVec);
        if (ret < 0) {
                LOG_ERROR("Query Error!");
                dbOpr->Close();
                delete dbOpr;
                return ret;
        }

        if (arvInfoVec[0][0] == "0") {
                maxId = "1";
        } else {
                char buf[10];
                int  max = atoi(arvInfoVec[0][0].c_str());

                max = max + 1;
                sprintf(buf, "%d", max);
                maxId = buf;
        }
        dbOpr->Close();
        delete dbOpr;

        return 0;
}

int MultiTaskArvOpr::CopyDefaultConfig(string& path)
{
        ChildProcessOpr cpo;
        string defaultConfig = "/etc/scigw/default/GWconfig";

        string cmd = "cp " + defaultConfig + " " + path;
        int ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
        }
        return ret;
}

int MultiTaskArvOpr::InsertArchiveInfo(const ArchiveInfo& archiveInfo)
{
        string configPath = "'" + archiveInfo.configPath + "'";
        string ip         = "'" + archiveInfo.ip         + "'";

        char sql[4096];
        sprintf(sql, "insert into sci_archive_info(id,ip,config_path) values(%d,%s,%s)", 
                                archiveInfo.id, ip.c_str(),configPath.c_str());

        DBOpr* dbOpr = NULL;
        int ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                if (dbOpr != NULL) {
                        delete dbOpr;
                }
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        ret = dbOpr->Query(sql);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sql);
                dbOpr->Close();
                delete dbOpr;
                return ret;
        }
        dbOpr->Close();
        delete dbOpr;

        return ret;
}

int MultiTaskArvOpr::UpdateArchiveInfo(const ArchiveInfo& archiveInfo)
{
        return 0;
}

int MultiTaskArvOpr::GetArchiveInfoMap(map<string, pair<string,string> >& arvInfoMap)
{
        DBOpr* dbOpr = NULL;
        int ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                if (dbOpr != NULL) {
                        delete dbOpr;
                }
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        BaseOpr baseOpr;
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error! ret=" << ret);
                return ret;
        }

        string  ip;
        ret = baseOpr.GetIp(networkId, ip);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                delete dbOpr;
                return ret;
        }

        vector< vector<string> > infoVec;
        string sql = "select id,config_path,ip from sci_archive_info where ip='" + ip + "'";
        ret = dbOpr->Query(sql, infoVec);
        if (ret < 0) {
                dbOpr->Close();
                delete dbOpr;
                LOG_ERROR("Query Error! sql=" << sql);
                return ret;
        }
        dbOpr->Close();
        delete dbOpr;

        for (size_t i=0; i<infoVec.size(); ++i) {
                arvInfoMap.insert(make_pair(infoVec[i][1], make_pair(infoVec[i][0], infoVec[i][2])));
        }
        return 0;
}

int MultiTaskArvOpr::DeleteArchiveInfo(const string& configPath)
{
        string cpath = "'" + configPath + "'";
        string sql = "delete from sci_archive_info where config_path=" + cpath;

        DBOpr* dbOpr = NULL;
        int ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                if (dbOpr != NULL) {
                        delete dbOpr;
                }
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        ret = dbOpr->Query(sql);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sql);
        }
        dbOpr->Close();
        delete dbOpr;

        /* 删除该配置文件对应的所有信息 */
        DirFileOpr dfOpr;
        ChildProcessOpr cpo;

        string cmd;
        string parPath;
        dfOpr.GetParPath(configPath, parPath);
        cmd = "rm -rf " + parPath;

        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
        }
        return ret;
}

int MultiTaskArvOpr::GetDBOpr(DBOpr** dbOpr)
{
        int ret = 0;
        DBOpr* db = new DBOpr;
        ret = db->Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        ret = db->Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error!");
        }
        *dbOpr = db;

        return ret;
}

int MultiTaskArvOpr::SyncToOtherNode(const string& syncPath)
{
        DBMgr dbMgr;
        BaseOpr baseOpr;
        ChildProcessOpr cpo;

        vector<string> ipList;
        int ret = dbMgr.GetIpListByLocal(ipList);
        if (ret < 0) {
                LOG_ERROR("GetIpListByLocal Error!");
                return ret;
        }

        /* 获取网络通路Id */
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error! ret=" << ret);
                return ret;
        }

        string curIp;
        ret = baseOpr.GetIp(networkId, curIp);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                return ret;
        }   

        string cmd;
        string cmd1 = "rsync -avz --password-file=/etc/rsyncd/passwd/passwd.inf ";
        string cmd2 = " root@";
        string cmd3 = "::scigw";
        for (size_t i=0; i<ipList.size(); ++i) {
                if (curIp == ipList[i]) {
                        continue;
                } 
                cmd = cmd1 + syncPath + cmd2 + ipList[i] + cmd3;
                LOG_INFO("cmd=" << cmd);
                ret = cpo.ExecuteCmd(cmd);
                if (ret < 0) {
                        LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
                        continue;
                }   
        }
        return ret;
}

int MultiTaskArvOpr::GetNodeIdByIp(const string& ip, string& nodeId)
{
        DBOpr* dbOpr;
        int ret = GetDBOpr(&dbOpr);
        if (ret < 0) {
                LOG_ERROR("GetDBOpr Error!");
                return ret;
        }

        vector<vector<string> > nodeIdList;
        string sqlCmd = "select node_id from sci_node_info where ip_addr=" + ip;
        ret = dbOpr->Query(sqlCmd, nodeIdList);
        if (ret < 0) {
                LOG_ERROR("Query Error!");
                return ret;
        }
        dbOpr->Close();
        delete dbOpr;
        nodeId = nodeIdList[0][0];

        return 0;
}


int MultiTaskArvOpr::CheckSrcPath(const string& srcPath, string& errInfo, const string& curConfig)
{
        map<string, pair<string,string> > arvInfoMap;
        int ret = GetArchiveInfoMap(arvInfoMap);
        if (ret < 0) {
                LOG_ERROR("GetArchiveInfoMap Error! srcPath=" << srcPath);
                errInfo = "该指定网卡为无效网卡!";
                return ret;
        }

        map<string, pair<string,string> >::iterator mssIter = arvInfoMap.begin();
        if (curConfig.empty()) {
                for (; mssIter != arvInfoMap.end(); ++mssIter) {
                        ret = FindSrcPathInConfig(mssIter->first, srcPath);
                        if (ret == PATH_EXIST) {
                                errInfo = "[" + srcPath + "], 已经在其他配置文件中使用！";
                                return ret;
                        } else if (ret < 0) {
                                LOG_ERROR("FindSrcPathInConfig Error!");
                                return ret;
                        }
                }
        } else {
                /* 跳过当前的配置文件不判断 */
                mssIter = arvInfoMap.begin();
                for (; mssIter != arvInfoMap.end(); ++mssIter) {
                        if (mssIter->first == curConfig) {
                                continue;
                        }

                        ret = FindSrcPathInConfig(mssIter->first, srcPath);
                        if (ret == PATH_EXIST) {
                                errInfo = "[" + srcPath + "], 已经在其他配置文件中使用！";
                                return ret;
                        } else if (ret < 0) {
                                LOG_ERROR("FindSrcPathInConfig Error!");
                                return ret;
                        }
                }
        }
        return 0;
}

int MultiTaskArvOpr::FindSrcPathInConfig(const string& configPath, const string& srcPath)
{
        /* 找到归档路径对存放位置 */
        IniParser ini(configPath);
        int ret = ini.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error! path=" << configPath);
                return ret;
        }

        string arvDevPath;
        ret = ini.GetVal("GatewayArchive", "arvDevPath", arvDevPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error!");
                return ret;
        }

        PathPairParser      ppParser(arvDevPath);
        map<string, string> arvPath;

        ret = ppParser.Read(arvPath);
        if (ret < 0) {
                LOG_ERROR("Read Error!");
                return ret;
        }

        /* 在以前的配置文件中存在 */
        map<string, string>::iterator mssIter = arvPath.begin();
        for (; mssIter != arvPath.end(); ++mssIter) {
                if (mssIter->first.find(srcPath) == 0
                                || srcPath.find(mssIter->first) == 0) {
                        ret = PATH_EXIST;
                }
        }
        return ret;
}

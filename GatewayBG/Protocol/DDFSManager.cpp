#include <vector>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "Config/IniParser.h"
#include "GatewayBG/Operation/DDFSInfo.h"
#include "GatewayBG/Operation/DiskMgr.h"
#include "GatewayBG/Protocol/DDFSManager.h"
#include "Utils/Log/Log.h"
#include "Utils/Json/include/json.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

//using std::cout;
//using std::endl;

const string configName = "ddfsrc";

DDFSManager::DDFSManager(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr) 
        :Protocol(ddfsMgr, baseOpr)
{
        m_PackMgr = packMgr;
}
        
int DDFSManager::Execute(const Pack& pack)
{
        //文件系统管理 0x6000000

        boost::mutex::scoped_lock lock(m_Mutex);
        int ret = 0;

        switch(pack.PkHead.Type & ASSISTANT_SECOND) {
        case MP_SECOND:
                ret = MountPointParse(pack);
                if (ret < 0) {
                        LOG_ERROR("MountPointParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case CONFIG_SECOND:
                ret = ConfigParse(pack);
                if (ret < 0) {
                        LOG_ERROR("ConfigParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case MP_OPR_SECOND:
                ret = OperatorMPParse(pack);
                if (ret < 0) {
                        LOG_ERROR("OperatorMPParse Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        //等待END包

        return 0;
}

//-----------
int DDFSManager::MountPointParse(const Pack& pack)
{
        int ret = 0;
        //0x610000
        switch(pack.PkHead.Type) {
        case CMD_MP_ALL_LIST:
                ret = ExecuteMPAllList(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPList Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_ONLINE_LIST:
                ret = ExecuteMPOnlineList(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPList Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_INFO:
                ret = ExecuteMPInfo(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int DDFSManager::ConfigParse(const Pack& pack)
{
        int ret = 0;
        //0x620000
        switch(pack.PkHead.Type) {
        case CMD_MP_ADD:
                ret = ExecuteMPAdd(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_MODIFY:
                ret = ExecuteMPModify(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_DELETE:
                ret = ExecuteMPDelete(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPDelete Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_LOOKUP:
                ret = ExecuteMPLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int DDFSManager::OperatorMPParse(const Pack& pack)
{
        int ret = 0;
        //0x630000
        switch(pack.PkHead.Type) {
        case CMD_MP_MOUNT:
                ret = ExecuteMPMount(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPMout Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_UMOUNT:
                ret = ExecuteMPUMount(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPUMount Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_FSCK:
                ret = ExecuteMPFsck(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMPFsck Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_CLASSIFY_MP:
                ret = ExecuteClassifyMp(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteClassifyMp Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int DDFSManager::ExecuteMPAllList(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteALLMPList!---", pack);

        std::vector<std::string> mpList;
        m_DDFSMgr->GetMPList(mpList);

        Json::Value path;
        Json::Value pathList;

        std::vector<std::string>::iterator beg=mpList.begin();
        for (; beg != mpList.end(); ++beg) {
                path["mp"]  = *beg;
                path["state"] = m_DDFSMgr->GetMPState((*beg).c_str());
                pathList["mplist"].append(path);
        }

        Pack mpListPack;
        mpListPack.PkHead.Type = CMD_OK;
        mpListPack.JsonValue   = pathList;
        if (!m_PackMgr->Send(mpListPack)) {
                LOG_ERROR("ExecuteMPList Send MPList Error!");
                return -1;
        }

        cout << mpListPack.JsonValue.toStyledString() << endl;
        
        return 0;
}

int DDFSManager::ExecuteMPOnlineList(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPOnlineList!---", pack);

        std::vector<std::string> mpList;
        m_DDFSMgr->GetMPList(mpList);

        Json::Value path;
        Json::Value pathList;

        ///在线的挂载点
        std::vector<std::string>::iterator beg=mpList.begin();
        for (; beg != mpList.end(); ++beg) {
                int state = m_DDFSMgr->GetMPState((*beg).c_str());
                if (state == MOUNTED) {
                        path["mp"]  = *beg;
                        path["state"] = state;
                        pathList["mplist"].append(path);
                }
        }

        Pack mpListPack;
        mpListPack.PkHead.Type = CMD_OK;
        mpListPack.JsonValue   = pathList;
        if (!m_PackMgr->Send(mpListPack)) {
                LOG_ERROR("ExecuteMPOnlineList Send MPList Error!");
                return -1;
        }
        cout << mpListPack.JsonValue.toStyledString() << endl;
        return 0;
}

int DDFSManager::ExecuteMPInfo(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPInfo!---", pack);

        int    ret = 0;

        std::string path = pack.JsonValue["mp"].asString();
        if (path.empty()) {
                if ( !SendErr("客户端传来的路径为空!") ) {
                        LOG_ERROR("SendErr Error!");
                        ret = -1;
                }
                return ret;
        }

        int64_t raw   = 0;
        int64_t real  = 0;
        float   dedup = 0;

        ret = m_DDFSMgr->GetDataSize(path.c_str(), raw, real);
        if (ret < 0) {
                if ( !SendErr("获取挂载点存储数据错误!") ) {
                        LOG_ERROR("SendErr Error!");
                        ret = -2;
                }
                return ret;
        }

        ret = m_DDFSMgr->GetDedupRatio(path.c_str(), dedup);
        if (ret < 0) {
                if ( !SendErr("获取挂载点消冗率错误!") ) {
                        LOG_ERROR("SendErr Error!");
                        ret = -3;
                }
                return ret;
        }

        char buffer[100];
        Pack data;

        Json::Value value;
        value["mp"]  = path;

        sprintf(buffer, "%.2f", dedup);
        value["ratio"] = buffer;

        sprintf(buffer, "%ld", raw);
        value["orisize"] = buffer;
        
        sprintf(buffer, "%ld", real);
        value["realsize"] = buffer;

        int64_t total = 0;
        int64_t left  = 0;
        ret = m_BaseOpr->GetLeftSpaceByMp(path, total, left);
        if (ret < 0) {
                if ( !SendErr("获取空间信息错误!") ) {
                        LOG_ERROR("SendErr Error!");
                        ret = -4;
                }
                return ret;
        }

        sprintf(buffer, "%ld", total);
        value["physicspace"]    = buffer;

        sprintf(buffer, "%ld", left);
        value["remainingspace"] = buffer;

        data.PkHead.Type = CMD_OK;
        data.JsonValue = value;
        if (!m_PackMgr->Send(data)) {
                LOG_ERROR("Send MpInfo Error!");
                return -5;
        }

        cout << data.JsonValue.toStyledString() << endl;
        return 0;
}

int DDFSManager::ExecuteMPAdd(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPAdd!---", pack);

        int ret = 0;

        std::string mp = pack.JsonValue["mp"].asString();
        if (mp.empty()) {
                if ( !SendErr("客户端传来的路径为空!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        /* 创建挂载点路径 */
        DirFileOpr dfOpr;
        if ( !dfOpr.HasPath(mp) ) {
                ret = dfOpr.MakeDir(mp.c_str());
                if (ret < 0) {
                        if ( !SendErr("创建挂载点目录失败!") ) {
                                LOG_ERROR("SendErr Error!");
                        }
                        return ret;
                }
        }
        vector<string> dataList;
        Json::Value array = pack.JsonValue["dataPath"];
        for (size_t i = 0; i<array.size(); ++i) {
                if ( array[i].asString().empty() ) {
                        continue;
                }
                if (dfOpr.HasPath(array[i].asString()) && 
                        dfOpr.IsBlockDevice(array[i].asString()) ) {
                        string errInfo = "[ " + array[i].asString() + 
                                " ] 是设备,路径不可以为设备!";
                        if ( !SendErr(errInfo) ) {
                                LOG_ERROR("SendErr Error!");
                        }
                        return 0;
                }
                dataList.push_back(array[i].asString());
        }
        
        /* 判断分区使用情况 */
        string existPath;
        ret = IsDevUsed(dataList, existPath);                   
        if (ret == -1) {
                if ( !SendErr("创建数据路径失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == -2) {
                if ( !SendErr("获取数据路径分区失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == DATA_PATH_EXIST) {
                string errInfo = "[ " + existPath + " ] 所在分区已经被使用，请换其他分区!";
                if ( !SendErr(errInfo) ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        /* 判断数据路径空间 */
        string leftPath;
        string minSpace   = pack.JsonValue["minSpace"].asString();
        int    iMinSpace  = atoi(minSpace.c_str());

        ret = IsSpaceLeft(dataList, leftPath, iMinSpace);
        if (ret < 0) {
                if ( !SendErr("获取路径剩余空间失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == DATA_PATH_NOT_HAVE_SPACE) {
                string errInfo = "[ " + leftPath + " ] 空间不足，小于最小空间!";
                if ( !SendErr(errInfo) ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        string dataPath = dataList[0];
        dfOpr.AppendBias(dataPath);
        const string configPath = dataPath + configName;

        /* 创建配置文件 */
        ChildProcessOpr   cpo;
        string cmd = "cp /etc/ddfs/ddfsrc " + configPath;
        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
                if ( !SendErr("创建挂载点配置文件失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        IniParser parser(configPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error! path=" << configPath);
                if (ret < 0) {
                        if ( !SendErr("创建挂载点配置文件失败!") ) {
                                LOG_ERROR("SendErr Error!");
                        }
                }
                return ret;
        }

        const string DPL      = "Dpl";
        const string PLDS     = "Plds";
        const string LDS      = "Lds";
        const string FSCONFIG = "FileSystem";
        
        /*dpl*/
        parser.SetVal(DPL, "dplCpuNum", pack.JsonValue["dplCpuNum"].asString());

        /*PLDS*/
        parser.SetVal(PLDS, "byteDedup", pack.JsonValue["byteDedup"].asString());
        parser.SetVal(PLDS, "chunkSize", pack.JsonValue["chunkSize"].asString());
        parser.SetVal(PLDS, "blockSize", pack.JsonValue["blockSize"].asString());
        parser.SetVal(PLDS, "dedupOption", pack.JsonValue["dedupOption"].asString());
        parser.SetVal(PLDS, "crcOption", pack.JsonValue["crcOption"].asString());
        parser.SetVal(PLDS, "byteCmpOption", pack.JsonValue["byteCmpOption"].asString());
        /* parser.SetVal(PLDS, "hashType", "1"); */
        /* parser.SetVal(PLDS, "compressType", "2"); */

        int  cnt = 1;
        char buffer[15];
        for (size_t i=0; i < dataList.size(); ++i, ++cnt) {
                sprintf(buffer, "dataPath%d", cnt);
                parser.SetVal(PLDS, buffer, dataList[i]);
        }
        
        string mapPath  = pack.JsonValue["mapPath"].asString();
        string metaPath = pack.JsonValue["metaPath"].asString();
        parser.SetVal(PLDS, "mapPath", mapPath);
        parser.SetVal(PLDS, "metaPath", metaPath);

        /* map */
        if ( !mapPath.empty()) {
                if ( !dfOpr.HasPath(mapPath) ) {
                        ret = dfOpr.MakeDir(mapPath);
                        if (ret < 0) {
                                if ( !SendErr("创建MAP目录失败!") ) {
                                        LOG_ERROR("SendErr Error!");
                                }
                                return ret;
                        }
                }
        }

        /* meta */
        if ( !metaPath.empty() ) {
                if ( !dfOpr.HasPath(metaPath) ) {
                        ret = dfOpr.MakeDir(metaPath);
                        if (ret < 0) {
                                if ( !SendErr("创建Meta目录失败!") ) {
                                        LOG_ERROR("SendErr Error!");
                                }
                                return ret;
                        }
                }
        }

        /*LDS*/
        parser.SetVal(LDS, "bucketCount", pack.JsonValue["ldsBucketCount"].asString());
        parser.SetVal(LDS, "minSpace", pack.JsonValue["minSpace"].asString());
        /* parser.SetVal(LDS, "fileTypeCount", "4"); */

        /* biPath */
        string biPath = pack.JsonValue["biPath"].asString();
        parser.SetVal(LDS, "biPath", biPath);
        if ( !dfOpr.HasPath(biPath) ) {
                ret = dfOpr.MakeDir(biPath.c_str());
                if (ret < 0) {
                        if ( !SendErr("创建挂载点目录失败!") ) {
                                LOG_ERROR("SendErr Error!");
                        }
                        return ret;
                }
        }

        string siPath = pack.JsonValue["siPath"].asString();
        parser.SetVal(LDS, "siPath", siPath);
        /* siPath*/
        if ( !siPath.empty() ) {
                if ( !dfOpr.HasPath(siPath) ) {
                        ret = dfOpr.MakeDir(siPath);
                        if (ret < 0) {
                                if ( !SendErr("创建SiPath目录失败!") ) {
                                        LOG_ERROR("SendErr Error!");
                                }
                                return ret;
                        }
                }
        }

        /*FSCONFIG*/
        parser.SetVal(FSCONFIG, "bucketCount", pack.JsonValue["fsBucketCount"].asString());
        parser.SetVal(FSCONFIG, "mmapSize", pack.JsonValue["mmapSize"].asString());
        parser.SetVal(FSCONFIG, "mountPoint", mp);

        ret = m_DDFSMgr->DDFSMkfs(mp.c_str(), configPath.c_str());
        if (ret < 0) {
                if ( !SendErr("执行消冗系统初始化失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == MOUNTED) {
                if ( !SendErr("对应的挂载点已经挂载!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return 0;
        } else if (ret == MP_IS_EXIST) {
                if ( !SendErr("对应的挂载点已经存在!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return 0;
        }

        if (!SendOk()) {
                LOG_ERROR("SendOK Error!");
        }
        return ret;
}

int DDFSManager::ExecuteMPModify(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPModify!---", pack);

        int ret = 0;

        const string DPL      = "Dpl";
        const string PLDS     = "Plds";
        const string LDS      = "Lds";
        const string FSCONFIG = "FileSystem";

        string mp = pack.JsonValue["mp"].asString();
        /* 获取配置文件路径 */
        string configPath;
        ret = m_DDFSMgr->GetConfigByMP(mp.c_str(), configPath);
        if (ret < 0) {
                LOG_ERROR("GetConfigByMP Error! ret=" << mp);
                if ( !SendErr("获取指定挂载点的配置文件失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == MP_NOT_HAS_CONFIG_FILE) {
                if ( !SendErr("该挂载点没有对应的配置文件!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if (configPath.empty()) {
                return 0;
        }

        DirFileOpr dfOpr;
        vector<string> dataList;
        set<string> dataSet;
        Json::Value array = pack.JsonValue["dataPath"];
        for (size_t i = 0; i<array.size(); ++i) {
                if ( dfOpr.HasPath(array[i].asString()) && dfOpr.IsBlockDevice(array[i].asString()) ) {
                        string errInfo = "[ " + array[i].asString() + " ] 是设备,数据路径路径不可以为设备!";
                        if ( !SendErr(errInfo) ) {
                                LOG_ERROR("SendErr Error!");
                        }
                        return 0;
                }
                dataList.push_back(array[i].asString());
                dataSet.insert(array[i].asString());
        }

        /* 判断分区使用情况 */
        string existPath;
        ret = IsDevUsed(dataList, existPath);
        if (ret == -1) {
                if ( !SendErr("创建数据路径失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == -2) {
                if ( !SendErr("获取数据路径分区失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == DATA_PATH_EXIST) {
                string errInfo = "[ " + existPath + " ] 所在分区已经被使用，请换其他分区!";
                if ( !SendErr(errInfo) ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        /* 获取已经在用的数据路径 */
        IniParser parser(configPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error! path=" << configPath);
                if ( !SendErr("获取配置文件信息失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        /* 数据路径 */
        int idx = 1;
        string dataPath;
        for ( ; ; ++idx) {
                char key[1024] = {0};
                sprintf(key, "dataPath%d", idx);
                ret = parser.GetVal(PLDS, key, dataPath);
                if (ret == -1) {
                        break;
                }
                if (dataSet.find(dataPath) != dataSet.end()) {
                        dataSet.erase(dataPath); 
                }
        }

        /* 剩余空间判断 */
        dataList.clear();
        for (set<string>::iterator it = dataSet.begin(); it != dataSet.end(); ++it) {
                dataList.push_back(*it); 
        }

        string leftPath;
        string minSpace   = pack.JsonValue["minSpace"].asString();
        int    iMinSpace  = atoi(minSpace.c_str());
        ret = IsSpaceLeft(dataList, leftPath, iMinSpace);
        if (ret < 0) {
                if ( !SendErr("获取路径空间失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == DATA_PATH_NOT_HAVE_SPACE) {
                string errInfo = "[ " + leftPath + " ] 空间不足，小于最小空间!";
                if ( !SendErr(errInfo) ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        /*dpl*/
        parser.SetVal(DPL, "dplCpuNum", pack.JsonValue["dplCpuNum"].asString());

        /*PLDS*/
        parser.SetVal(PLDS, "byteDedup", pack.JsonValue["byteDedup"].asString());
        parser.SetVal(PLDS, "chunkSize", pack.JsonValue["chunkSize"].asString());
        parser.SetVal(PLDS, "blockSize", pack.JsonValue["blockSize"].asString());
        parser.SetVal(PLDS, "dedupOption", pack.JsonValue["dedupOption"].asString());
        parser.SetVal(PLDS, "crcOption", pack.JsonValue["crcOption"].asString());
        parser.SetVal(PLDS, "byteCmpOption", pack.JsonValue["byteCmpOption"].asString());
        parser.SetVal(PLDS, "mapPath", pack.JsonValue["mapPath"].asString());
        parser.SetVal(PLDS, "metaPath", pack.JsonValue["metaPath"].asString());

        /*LDS*/
        parser.SetVal(LDS, "bucketCount", pack.JsonValue["ldsBucketCount"].asString());
        parser.SetVal(LDS, "minSpace", pack.JsonValue["minSpace"].asString());

        /*FSCONFIG*/
        parser.SetVal(FSCONFIG, "bucketCount", pack.JsonValue["fsBucketCount"].asString());
        parser.SetVal(FSCONFIG, "mmapSize", pack.JsonValue["mmapSize"].asString());

        
        map<string, string> onlineMap;
        m_DDFSMgr->GetOnlineMPMap(onlineMap);

        if (onlineMap.find(mp) != onlineMap.end()) {
                //cout << "******************* if ******************" << endl;
                string tcmd;
                ChildProcessOpr cpo;

                for (set<string>::iterator it = dataSet.begin(); it != dataSet.end(); ++it) {
                        tcmd = "util.ddfs -vol " + *it + " " + configPath;
                        ret = cpo.ExecuteCmd(tcmd);
                        if (ret < 0) {
                                LOG_ERROR("ExecuteCmd Error!");
                                if ( !SendErr("添加数据路径失败!") ) {
                                        LOG_ERROR("SendErr Error!");
                                }
                        }
                }
        } else {
                //cout << "******************* else *****************" << endl;
                for (size_t i=0; i < dataList.size(); ++i) {
                        char buffer[1024] = {0};
                        sprintf(buffer, "dataPath%d", idx);
                        parser.SetVal(PLDS, buffer, dataList[i]);
                        ++idx;
                }
        }

        if (!SendOk()) {
                LOG_ERROR("SendOK Error!");
        }
        return ret;
}

int DDFSManager::ExecuteMPDelete(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPDelete!---", pack);

        int ret = 0;

        std::string mp = pack.JsonValue["mp"].asString();
        if (mp.empty()) {
                if ( !SendErr("客户端传来的路径为空!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return 0;
        }

        ret = m_DDFSMgr->DeleteMountPoint(mp.c_str());
        if (ret < 0) {
                if ( !SendErr("删除挂载点对应数据路径出错!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if (!SendOk()) {
                LOG_ERROR("SendOK Error!");
        }
        return ret;
}

int DDFSManager::ExecuteMPMount(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPMount!---", pack);

        int ret = 0;

        std::string path = pack.JsonValue["mp"].asString();
        if (path.empty()) {
                if ( !SendErr("客户端传来的路径为空!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        ChildProcessOpr cmdOpr;
        if(cmdOpr.IsTaskRunning("DDFSArchive|scidata_backup.sh")) {
                if ( !SendErr("挂载失败，归档运行中!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return 0;
        }

        ret = m_DDFSMgr->Mount(path.c_str());
        if (ret == MP_NOT_EXIST) {
                if ( !SendErr("挂载点目录不存在!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == MP_IS_NOT_EMPTY) {
                if ( !SendErr("挂载点目录不为空!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret == MP_NOT_HAS_CONFIG_FILE) {
                if ( !SendErr("挂载点对应的配置文件不存在!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (ret < 0) {
                if ( !SendErr("挂载命令执行失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if (!SendOk()) {
                LOG_ERROR("SendOK Error!");
        }
        sleep(1);
        /* 修改配置文件 */
        ret = SetDDFSConfigForService();
        if (ret < 0) {
                LOG_ERROR("SetDDFSConfigForService Error!");
        }
        return ret;
}

int DDFSManager::ExecuteMPUMount(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPUMount!---", pack);

        int ret = 0;

        std::string path = pack.JsonValue["mp"].asString();
        if (path.empty()) {
                if ( !SendErr("客户端传来的路径为空!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        ChildProcessOpr cmdOpr;
        if(cmdOpr.IsTaskRunning("DDFSArchive|scidata_backup.sh")) {
                if ( !SendErr("卸载失败，归档运行中!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return 0;
        }

        ret = m_DDFSMgr->UnMount(path.c_str());
        if (ret < 0) {
                LOG_ERROR("UnMount Error!" << path);
                if ( !SendErr("卸载命令执行失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if (!SendOk()) {
                LOG_ERROR("SendOK Error!");
        }

        /* 修改配置文件 */
        ret = SetDDFSConfigForService();
        if (ret < 0) {
                LOG_ERROR("SetDDFSConfigForService Error!");
        }
        return ret;
}

int DDFSManager::ExecuteMPFsck(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMPFsck!---", pack);

        int ret = 0;
        std::string path = pack.JsonValue["mp"].asString();
        if (path.empty()) {
                if ( !SendErr("客户端传来的路径为空!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        std::string level = pack.JsonValue["level"].asString();

        ret = m_DDFSMgr->Fsck(path.c_str(), level);
        if (ret < 0) {
                LOG_ERROR("Fsck Error!" << path);
                if ( !SendErr("修复命令执行失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if (!SendOk()) {
                LOG_ERROR("SendOK Error!");
        }
        return ret;
}

int DDFSManager::ExecuteMPLookup(const Pack& pack)
{
        DisplayExecuteInfo("----ExecuteMPLookup----", pack);

        int ret = 0;

        const string DPL      = "Dpl";
        const string PLDS     = "Plds";
        const string LDS      = "Lds";
        const string FSCONFIG = "FileSystem";

        std::string mp = pack.JsonValue["mp"].asString();

        string configPath;
        ret = m_DDFSMgr->GetConfigByMP(mp.c_str(), configPath);
        if (ret < 0) {
                LOG_ERROR("GetConfigByMP Error! ret=" << mp);
                if ( !SendErr("获取指定挂载点的配置文件失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return 0;
        } else if (ret == MP_NOT_HAS_CONFIG_FILE) {
                if ( !SendErr("该挂载点没有对应的配置文件!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if (configPath.empty()) {
                return 0;
        }

        /* 打开配置文件 */
        IniParser parser(configPath);
        ret = parser.Init();
        if (ret != 0) {
                LOG_ERROR("config Read error!");
                if ( !SendErr("读取配置文件信息失败!") ) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        Pack configInfo;
        configInfo.PkHead.Type = CMD_OK;

        /*mp*/
        configInfo.JsonValue["mp"]  = mp;

        /*FSCONFIG*/
        string fsBucketCount;
        string mmapSize;
        parser.GetVal(FSCONFIG, "bucketCount", fsBucketCount);
        parser.GetVal(FSCONFIG, "mmapSize", mmapSize);

        configInfo.JsonValue["fsBucketCount"] = fsBucketCount;
        configInfo.JsonValue["mmapSize"]      = mmapSize;

        /*DPL*/
        string dplCpuNum;
        parser.GetVal(DPL, "dplCpuNum", dplCpuNum);
        configInfo.JsonValue["dplCpuNum"] = dplCpuNum;

        /*LDS*/
        string biPath;
        parser.GetVal(LDS, "biPath", biPath);
        configInfo.JsonValue["biPath"] = biPath;

        string siPath;
        parser.GetVal(LDS, "siPath", siPath);
        configInfo.JsonValue["siPath"] = siPath;

        string ldsBucketCount;
        string minSpace;
        parser.GetVal(LDS, "bucketCount", ldsBucketCount);
        parser.GetVal(LDS, "minSpace", minSpace);
        configInfo.JsonValue["ldsBucketCount"] = ldsBucketCount;
        configInfo.JsonValue["minSpace"] = minSpace;

        /*PLDS*/
        string chunkSize;
        string blockSize;
        string byteDedup;
        string dedupOption;
        string crcOption;
        string byteCmpOption;
        parser.GetVal(PLDS, "chunkSize", chunkSize);
        parser.GetVal(PLDS, "blockSize", blockSize);
        parser.GetVal(PLDS, "byteDedup", byteDedup);
        parser.GetVal(PLDS, "dedupOption", dedupOption);
        parser.GetVal(PLDS, "crcOption", crcOption);
        parser.GetVal(PLDS, "byteCmpOption", byteCmpOption);

        configInfo.JsonValue["chunkSize"]     = chunkSize;
        configInfo.JsonValue["blockSize"]     = blockSize;
        configInfo.JsonValue["byteDedup"]     = byteDedup;
        configInfo.JsonValue["dedupOption"]   = dedupOption;
        configInfo.JsonValue["crcOption"]     = crcOption;
        configInfo.JsonValue["byteCmpOption"] = byteCmpOption;

        int            cnt = 1;
        char           buffer[15];
        string         dataPath;
        /*获取数据路径*/
        while (true) {
                sprintf(buffer, "dataPath%d", cnt);
                ret = parser.GetVal(PLDS, buffer, dataPath);
                if (ret == -1) {
                        break;
                }
                configInfo.JsonValue["dataPath"].append(dataPath);
                ++cnt;
        }
        string mapPath;
        string metaPath;
        parser.GetVal(PLDS, "mapPath", mapPath);
        parser.GetVal(PLDS, "metaPath", metaPath);
        configInfo.JsonValue["mapPath"]  = mapPath;
        configInfo.JsonValue["metaPath"] = metaPath;

        /* 获取CPU个数 */
        int num = 0;
        m_BaseOpr->GetCpuCoreNum(num);
        configInfo.JsonValue["cpuNumMax"] = num;

        if (!m_PackMgr->Send(configInfo)) {
                LOG_ERROR("ExecuteMPAdd Send OK Error!");
                return -3;
        }

        cout << configInfo.JsonValue.toStyledString() << endl;
        return 0;
}

int DDFSManager::ExecuteClassifyMp(const Pack& pack)
{
        int ret = 0;
        /* 抓取消冗信息 */
        int64_t raw   = 0;
        int64_t real  = 0;

        map<string, string> onlineMap;
        m_DDFSMgr->GetOnlineMPMap(onlineMap);

        map<string, string>::iterator mssIter = onlineMap.begin();
        for (; mssIter != onlineMap.end(); ++mssIter) {
                ret = m_DDFSMgr->GetDataSize(mssIter->first.c_str(), raw, real);
                if (ret < 0) {
                        LOG_ERROR("GetDataSize Error!");
                }
        }

        char buffer[20];
        /* 组包 */
        Pack revPack;
        revPack.PkHead.Type = CMD_OK;

        sprintf(buffer, "%ld", raw);
        revPack.JsonValue["total"] = buffer;
        
        sprintf(buffer, "%ld", real);
        revPack.JsonValue["real"] = buffer;

        m_PackMgr->Send(revPack);

        return 0;
}

int DDFSManager::IsDevUsed(const vector<string>& dataList, string& existPath)
{
        int ret = 0;

        map<string, char> partitionMap;
        ChildProcessOpr   cpo;
        DirFileOpr        dfOpr;
        DiskMgr           diskMgr;
        string            partition;

        int flag = 0;
        for (size_t i=0; i < dataList.size(); ++i) {
                /* 创建数据目录 */
                if ( !dfOpr.HasPath(dataList[i]) ) {
                        //LOG_INFO("path=" << dataList[i]);
                        ret = dfOpr.MakeDir(dataList[i]);
                        if (ret < 0) {
                                LOG_ERROR("MakeDir Error! path=" << dataList[i]);
                                return -1;
                        }
                        flag = 1;
                }

                /* 查看数据路径分区 */
                ret = diskMgr.GetPartitionFromDir(dataList[i], partition);
                if (ret < 0) {
                        LOG_ERROR("GetPartitionFromDir Error! path=" << dataList[i]);
                        return -2;
                }

                /* 查看数据路径所在分区是否已经存在配置文件中 */
                if ( partitionMap.find(partition) != partitionMap.end() ) {
                        if (flag == 1) {
                                string tCmd = "rm -rf " + dataList[i];
                                ret = cpo.ExecuteCmd(tCmd);
                                if (ret < 0) {
                                        LOG_ERROR("ExecuteCmd Error!");
                                }
                                flag = 0;
                        }
                        existPath = dataList[i];
                        return DATA_PATH_EXIST;
                }
                partitionMap.insert(make_pair(partition, 'a'));
        }
        return 0;
}

int DDFSManager::IsSpaceLeft(const vector<string>& dataList, string& leftPath, int iMinSpace)
{
        for (size_t i=0; i < dataList.size(); ++i) {
                /* 查看数据路径空间是否够 */
                int64_t total = 0;
                int64_t left  = 0;
                m_BaseOpr->GetLeftSpaceByMp(dataList[i], total, left);
                if (iMinSpace > (left/1024/1024/1024)) {
                        leftPath = dataList[i];
                        return DATA_PATH_NOT_HAVE_SPACE;
                }
        }
        return 0;
}

int DDFSManager::SetDDFSConfigForService()
{
        int ret = 0;

        ChildProcessOpr cpo;
        ret = cpo.ExecuteCmd("sed -i '1,$d' /etc/sysconfig/ddfs");
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
                return ret;
        } 

        ret = cpo.ExecuteCmd("sed -i '1a [DDFS]' /etc/sysconfig/ddfs");
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
                return ret;
        } 

        //获取挂载状态的挂载点
        map<string, string> mpMap;
        m_DDFSMgr->GetOnlineMPMap(mpMap);

        IniParser iniOpr("/etc/sysconfig/ddfs");
        iniOpr.Init();

        char buffer[20];
        int cnt = 1;
        map<string, string>::iterator mssIter = mpMap.begin();
        for (; mssIter != mpMap.end(); ++mssIter, ++cnt) {
                sprintf(buffer, "MOUNT_POINT%d", cnt);
                iniOpr.SetVal("DDFS", buffer, mssIter->first);

                sprintf(buffer, "CONFIG_FILE%d", cnt);
                iniOpr.SetVal("DDFS", buffer, mssIter->second);
        }

        ret = cpo.ExecuteCmd("sed -i '1d' /etc/sysconfig/ddfs");
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
        }
        return ret;
}

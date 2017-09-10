#include <string.h>
#include "Utils/Log/Log.h"
#include "Utils/DB/DBMgr.h"
#include "Config/IniParser.h"
#include "Config/FilterParser.h"
#include "Archive/Cront/Cront.h"
#include "Config/PathPairParser.h"
#include "Utils/CommonOpr/TimeOpr.h"
#include "GatewayBG/Operation/DBInfo.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Protocol/StorageGW.h"
#include "GatewayBG/Operation/BaseInfo.h"
#include "GatewayBG/Operation/DDFSInfo.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"
#include "GatewayBG/Operation/MultiTaskArvOpr.h"

#define BUFFER_SIZE 4096

const string GATEWAY_ARCHIVE    = "GatewayArchive";
const string START_TIME         = "startTime";
const string END_TIME           = "endTime";
const string RUN_TIME           = "runTime";
const string INTERVAL_TIME      = "intervalTime";
const string CRON_TIME          = "cronTime";
const string IS_CHECK           = "isCheck";
const string MODIFY_TIME        = "modifyTime";
const string FILTER_PATH        = "filterPath";
const string FILTER_TYPE        = "filterType"; 
const string DEMOND_PATH        = "demandPath";
const string DEMOND_TYPE        = "demandType";
const string PID_PATH           = "pidPath";
const string MOVE_LOG_PATH      = "moveLogPath";

const string ARVERR_PATH        = "arvErrPath";
const string ARVTASK_PATH       = "arvTaskPath";
const string ARVFINISHED_PATH   = "arvFinishedPath";
const string ARVDEV_PATH        = "arvDevPath";
const string FILTER             = "filter";

StorageGW::StorageGW(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr)
:Protocol(ddfsMgr, baseOpr)
{
        m_PackMgr = packMgr;
}

int StorageGW::Execute(const Pack& pack)
{
        //存储网关管理 0x5000000
        boost::mutex::scoped_lock lock(m_Mutex);
        int ret = 0;

        switch(pack.PkHead.Type & ASSISTANT_SECOND) {
        case DEV_SECOND:
                ret = DevParse(pack);
                if (ret < 0) {
                        LOG_ERROR("DevParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case NETCARD_SECOND:
                ret = NetCardParse(pack);
                if (ret < 0) {
                        LOG_ERROR("NetCardParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case DDFS_SECOND:
                ret = DDFSParse(pack);
                if (ret < 0) {
                        LOG_ERROR("DDFSParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case ARCHIVE_SECOND:
                ret = ArchiveParse(pack);
                if (ret < 0) {
                        LOG_ERROR("BackupParse Error! ret=" << ret);
                        return ret;
                }
                break;
        }

        return 0;
}

int StorageGW::DevParse(const Pack& pack)
{
        int ret = 0;
        //0x510000
        switch(pack.PkHead.Type) {
        case CMD_DISK_DRIVER:
                ret = ExecuteDiskDriver(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDiskDriver Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DEV_MOUNT:
                ret = ExecuteMountDev(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDevInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DEV_UMOUNT:
                ret = ExecuteUnMountDev(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDevInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DEV_DELETE:
                ret = ExecuteDeleteDev(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDevInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_FROMAT_DEV:
                ret = ExecuteFromatDev(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteFromatDev Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_CREATE_PARTITION_DEV:
                ret = ExecuteCreatePartitionDev(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteCreatePartitionDev Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int StorageGW::NetCardParse(const Pack& pack)
{
        int ret = 0;
        //0x520000
        switch(pack.PkHead.Type) {
        case CMD_NETCARD_INFO:
                ret = ExecuteNetCardInfo(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNetCardInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NETCARD_TRUNK:
                ret = ExecuteNetCardTrunk(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNetCardTrunk Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NETCARD_UNTRUNK:
                ret = ExecuteNetCardUnTrunk(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNetCardUnTrunk Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NETPORT_CONFIG:
                ret = ExecuteNetPortConfig(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNetPortConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int StorageGW::DDFSParse(const Pack& pack)
{
        int ret = 0;
        //0x5300000
        switch(pack.PkHead.Type) {
        case CMD_DDFS_START:
                ret = ExecuteDDFSStart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDDFSStart Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DDFS_STOP:
                ret = ExecuteDDFSStop(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDDFSStop Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DDFS_LOOKUP:
                ret = ExecuteDDFSLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDDFSLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DDFS_RESTART:
                ret = ExecuteDDFSRestart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDDFSRestart Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DDFS_FORCE_STOP:
                ret = ExecuteDDFSForceStop(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDDFSForceStop Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DDFS_FORCE_RESTART:
                ret = ExecuteDDFSForceRestart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDDFSForceRestart Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_GWENGINE_STOP:
                ret = ExecuteGWEngineStop(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteGWEngineStop Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_GWENGINE_RESTART:
                ret = ExecuteGWEngineRestart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteGWEngineRestart Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int StorageGW::ArchiveParse(const Pack& pack)
{
        int ret = 0;
        //0x540000
        switch(pack.PkHead.Type) {
        case CMD_DATA_ARCHIVE:
                ret = ExecuteArchiveData(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteBackupService Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DATA_LOOKUP:
                ret = ExecuteDataLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDataLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DATA_STOP:
                ret = ExecuteDataStop(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDataStop Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DATA_START:
                ret = ExecuteDataStart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDataStart Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DATA_DELETE:
                ret = ExecuteDataDelete(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDataDelete Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DATA_CONFIGLIST:
                ret = ExecuteDataConfigList(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDataConfigList Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_DATA_MODIFY:
                ret = ExecuteDataModify(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteDataModify Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

//::EXE----------
int StorageGW::ExecuteDiskDriver(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDiskDriver!---", pack);

        int ret = 0;
        Json::Value driver;

        map<string, DevInfo> info;
        ret = m_BaseOpr->GetStorageInfo(info);
        if (ret < 0) {
                LOG_ERROR("GetStorageInfo Error! ret=" << ret);
                if (!SendErr("获取磁盘空间失败!")) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        char buffer[30];
        for (map<string, DevInfo>::iterator it = info.begin(); it != info.end(); ++it) {
                Json::Value dev;
                Json::Value diskDriver;

                diskDriver["driver"]    = it->first;
                diskDriver["commpany"]  = it->second.Vendor;

                sprintf(buffer, "%ld", it->second.TotalSize);
                diskDriver["size"]      = buffer;

                list<DiskInfo>::iterator beg = it->second.Disks.begin();
                for (; beg != it->second.Disks.end(); ++beg) {
                        dev["dev"]    = beg->DiskName;
                        dev["fstype"] = beg->FileSystem;

                        sprintf(buffer, "%ld", beg->TotalSize);
                        dev["size"]   = buffer;
                        dev["mp"]     = beg->MountPoint;
                        diskDriver["partition"].append(dev);   //.append(dev);
                }
                driver["diskdriver"].append(diskDriver);
        }

        Pack diskInfo;
        diskInfo.PkHead.Type = CMD_OK;
        diskInfo.JsonValue   = driver;
        if (!m_PackMgr->Send(diskInfo)) {
                LOG_ERROR("ExecuteDiskDriver Send DiskInfo Error!");
                return -1;
        }
        cout << diskInfo.JsonValue.toStyledString() << endl;
        return 0;
}

int StorageGW::ExecuteMountDev(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMountDev!---", pack);

        int ret = 0;

        std::string devPath   = pack.JsonValue["diskdriver"].asString();
        std::string mountPath = pack.JsonValue["mp"].asString();
        if (mountPath.empty()) {
                if (!SendErr("客户端传来的挂载点路径为空!")) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } else if (devPath.empty()) {
                if (!SendErr("客户端传来的设备路径为空!")) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        DirFileOpr dfOpr;
        if ( !dfOpr.HasPath(mountPath.c_str()) ) {
                ret = dfOpr.MakeDir(mountPath.c_str());
                if (ret < 0) {
                        if (!SendErr("创建挂载点目录失败!")) {
                                LOG_ERROR("SendErr Error!");
                        }
                        return ret;
                }
        }

        ChildProcessOpr cpo;
        string cmd = "sed -i '$a" + devPath +"\t" + mountPath + "\text3\tdefaults\t0 0' /etc/fstab";
        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) { 
                LOG_ERROR("ExecuteCmd Error!");
                if (!SendErr("添加到Fstab失败!")) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } 

        ret = m_BaseOpr->Mount(devPath, mountPath);
        if (ret < 0) {
                if (!SendErr("挂载失败!")) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        }

        if ( !SendOk() ) {
                LOG_ERROR("SendOk Error!");
        }
        return ret;
}

int StorageGW::ExecuteUnMountDev(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteUnMountDev!---", pack);

        int ret = 0;

        ChildProcessOpr cpo;
        if ( cpo.IsTaskRunning("scidata_backup.sh|DDFSArchive") ) { 
                string errInfo = "正在归档，无法卸载分区!";
                SendErr(errInfo);
                return ret;
        }

        std::string devPath   = pack.JsonValue["diskdriver"].asString();
        std::string mountPath = pack.JsonValue["mp"].asString();

        char cmd[BUFFER_SIZE];
        sprintf(cmd, "grep -n \"%s\" /etc/fstab", mountPath.c_str());

        FILE *file = popen(cmd, "r");
        if (file == NULL) {
                LOG_ERROR("popen error!");
                if (!SendErr("清除Fstab失败!")) {
                        LOG_ERROR("SendErr Error!");
                }
                return ret;
        } 

        char   dev[BUFFER_SIZE];
        char   mp[BUFFER_SIZE];
        string lineNum;
        while ( fgets(cmd, 255, file) != NULL) {
                sscanf(cmd, "%s %s", dev, mp);
                if (strcmp(mp, mountPath.c_str()) == 0) {
                        string sdev    = dev;
                        lineNum = sdev.substr(0, sdev.find(":"));
                        break;
                }
        }
        pclose(file);

        if (!lineNum.empty()) {
                string cmdLine = "sed -i '" + lineNum + "'d /etc/fstab";
                ret = cpo.ExecuteCmd(cmdLine);
                if (ret < 0) { 
                        LOG_ERROR("popen error!");
                        if (!SendErr("清除Fstab失败!")) {
                                LOG_ERROR("SendErr Error!");
                        }
                        return ret;
                }
        }

        ret = m_BaseOpr->UnMount(mountPath);
        if (ret < 0) {
                if (!SendErr("卸载失败!")) {
                        LOG_ERROR("GetStorageInfo Error!");
                }
                return ret;
        }

        if ( !SendOk() ) {
                LOG_ERROR("SendOk Error!");
        }
        return ret;
}

int StorageGW::ExecuteDeleteDev(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDeleteDev!---", pack);
        return 0;
}

int StorageGW::ExecuteFromatDev(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteFromatDev!---", pack);
        return 0;
}

int StorageGW::ExecuteCreatePartitionDev(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteCreatePartitionDev!---", pack);
        return 0;
}

int StorageGW::ExecuteNetCardInfo(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNetCardInfo!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteNetPortConfig(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNetPortConfig!---", pack);
        ChildProcessOpr cpo;
        if ( cpo.IsTaskRunning("scidata_backup|DDFSArchive") ) { 
                SendErr("设置IP失败!,归档运行中");
                return 0;
        }
        SendOk();
        return 0;
}

int StorageGW::ExecuteNetCardTrunk(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNetCardTrunk!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteNetCardUnTrunk(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNetCardUnTrunk!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDDFSStart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDDFSStart!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDDFSStop(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDDFSStop!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDDFSLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDDFSLookup!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDDFSRestart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDDFSRestart!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDDFSForceStop(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDDFSForceStop!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDDFSForceRestart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDDFSForceRestart!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteGWEngineRestart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteGWEngineRestart!---", pack);
        return 0;
}

int StorageGW::ExecuteGWEngineStop(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteGWEngineStop!---", pack);
        SendOk();
        return 0;
}

int StorageGW::ExecuteDataLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDataLookup!---", pack);

        string status;
        string id         = pack.JsonValue["id"].asString();
        string configPath = "/etc/scigw/config/" + id + "/GWconfig";
        string cmd        = "/usr/sbin/DDFSArchive " + configPath;

        Cront ct;
        ChildProcessOpr cpo;
        if (ct.Find(cmd) == 1) {
                if ( cpo.IsTaskRunning("scidata_backup.sh|DDFSArchive") ) {
                        status = "2";
                } else {
                        status = "1";
                }
        } else {
                status = "0";
        }

        DirFileOpr dfOpr;
        string     start, interval, isCheck, mdfyTime, run, end;

        IniParser  iParser(configPath);
        int ret = iParser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                SendErr("打开归档配置文件失败!");
        }

        iParser.GetVal(GATEWAY_ARCHIVE, START_TIME, start);
        iParser.GetVal(GATEWAY_ARCHIVE, INTERVAL_TIME, interval);
        iParser.GetVal(GATEWAY_ARCHIVE, IS_CHECK, isCheck);
        iParser.GetVal(GATEWAY_ARCHIVE, MODIFY_TIME, mdfyTime);
        iParser.GetVal(GATEWAY_ARCHIVE, END_TIME, end);

        Pack data;
        data.PkHead.Type = CMD_OK;
        data.JsonValue["time"]           = start;
        data.JsonValue["intervaltime"]   = interval;
        data.JsonValue["check"]          = isCheck;
        data.JsonValue["status"]         = status;
        data.JsonValue["modifyTime"]     = mdfyTime;
        data.JsonValue["end"]            = end;
        data.JsonValue["id"]             = id;

        //path pair parse
        PathPairParser      ppParser("/etc/scigw/config/" + id + "/archive.cnf");
        map<string, string> arvPath;

        ppParser.Read(arvPath);
        map<string, string>::iterator mssIter = arvPath.begin();
        for ( ; mssIter != arvPath.end(); ++mssIter) {
                data.JsonValue["archiveSrcdir"].append(mssIter->first);
                data.JsonValue["archiveDestdir"].append(mssIter->second);
        }

        //filter condition parse
        FilterParser                 fParser("/etc/scigw/config/" + id + "/filter.cnf");
        map<string, vector<string> > filter;

        fParser.Read(filter);
        map<string, vector<string> >::iterator msvIter = filter.begin();
        for ( ; msvIter != filter.end(); ++msvIter) {
                if (msvIter->first == FILTER_PATH) {
                        vector<string>::iterator vsIter = msvIter->second.begin();
                        for ( ; vsIter != msvIter->second.end(); ++vsIter) {
                                data.JsonValue["filterPath"].append(*vsIter);
                        }
                } else if (msvIter->first == FILTER_TYPE) {
                        vector<string>::iterator vsIter = msvIter->second.begin();
                        for ( ; vsIter != msvIter->second.end(); ++vsIter) {
                                data.JsonValue["filterType"].append(*vsIter);
                        }
                } else if (msvIter->first == DEMOND_PATH) {
                        vector<string>::iterator vsIter = msvIter->second.begin();
                        for ( ; vsIter != msvIter->second.end(); ++vsIter) {
                                data.JsonValue["demandPath"].append(*vsIter);
                        }
                } else if (msvIter->first == DEMOND_TYPE) {
                        vector<string>::iterator vsIter = msvIter->second.begin();
                        for ( ; vsIter != msvIter->second.end(); ++vsIter) {
                                data.JsonValue["demandType"].append(*vsIter);
                        }
                } else {
                        LOG_ERROR("no key for: " << msvIter->first); 
                }
        }

        if (!m_PackMgr->Send(data)) {
                LOG_ERROR("send data error!");
                return -1;
        }

        cout << data.JsonValue.toStyledString() << endl;
        return 0;
}

int StorageGW::ExecuteArchiveData(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteArchiveData!---", pack);

        DirFileOpr dfOpr;
        string errMsg;
        MultiTaskArvOpr mtao;

        int ret = 0;
        /* 判断源是否存在 */
        for (size_t i = 0; i < pack.JsonValue["archiveSrcdir"].size(); ++i) {
                string srcPath = pack.JsonValue["archiveSrcdir"][i].asString();
                if (!dfOpr.HasPath(srcPath)) {
                        errMsg = "归档源路径不存在: " + srcPath;
                        SendErr(errMsg);
                        return 0;
                }

                /* 判断源是否已经在其他配置文件中存在 */
                ret = mtao.CheckSrcPath(srcPath, errMsg, "");
                if (ret == PATH_EXIST) {
                        SendErr(errMsg);
                        return ret; 
                } else if (ret < 0) { 
                        LOG_ERROR("CheckSrcPath Error!");
                        if (!errMsg.empty()) {
                                SendErr(errMsg);
                        } else {
                                SendErr("校验源数据路径失败!");
                        }
                        return ret; 
                }

                if ( dfOpr.IsBlockDevice(srcPath) ) {
                        errMsg = "归档源路径不能为设备! [" + srcPath + " ]";
                        SendErr(errMsg);
                        return 0;
                }
        }

        /* 判断目的是否存在 */
        for (size_t i = 0; i < pack.JsonValue["archiveSrcdir"].size(); ++i) {
                string destPath = pack.JsonValue["archiveDestdir"][i].asString();
                if (!dfOpr.HasPath(destPath)) {
                        errMsg = "归档目标路径不存在: " + destPath;
                        SendErr(errMsg);
                        return 0;
                }
                if ( dfOpr.IsBlockDevice(destPath) ) {
                        errMsg = "归档目的不能为设备! [" + destPath + " ]";
                        SendErr(errMsg);
                        return 0;
                }
        }

        /* 先找归档任务目前的最大编号，没有找到就是1 */
        string maxId;
        mtao.GetMaxID(maxId);

        /* 通过编号获取当前配置文件路径 */
        string configPath = "/etc/scigw/config/" + maxId;
        string syncPath   = configPath;

        if (!dfOpr.HasPath(configPath)) {
                ret = dfOpr.MakeDir(configPath);
                if (ret < 0) {
                        LOG_ERROR("MakeDir Error!");
                        SendErr("创建归档配置文件目录失败");
                        return ret;
                }
        }

        /* copy默认配置文件到该路径 */
        ret = mtao.CopyDefaultConfig(configPath);
        if (ret < 0) {
                LOG_ERROR("CopyDefaultConfig Error!");
                SendErr("创建归档配置文件失败");
                return ret;
        }

        /* 编辑该配置文件 */
        string pidPath     = configPath + "/DDFSArchive.pid";
        string devPath     = configPath + "/archive.cnf";
        string filterPath  = configPath + "/filter.cnf";
        string moveLogPath = configPath + "/moveLog";
        configPath         = configPath + "/GWconfig";

        /* 插入数据库 */
        BaseOpr baseOpr;
        ArchiveInfo ai;
        ai.id         = atoi(maxId.c_str());
        ai.configPath = configPath;
        /* 获取网络通路Id */
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error!");
                return ret;
        }
        /* 获取IP */
        ret = baseOpr.GetIp(networkId, ai.ip);
        if (ret < 0) {
                LOG_ERROR("GetIp Error! ");
                SendErr("获取IP失败");
                return ret;
        }

        ret = mtao.InsertArchiveInfo(ai);
        if (ret < 0) {
                LOG_ERROR("InsertArchiveInfo Error!");
                SendErr("插入数据库出错");
                return ret;
        }

        IniParser parser(configPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                SendErr("打开配置文件失败");
        }
        /* 获取其他路径 */
        string arvPath    = "/var/spool/scigw/ArvTask/" + maxId + "/";
        string finishPath = "/var/spool/scigw/FinishedTask/" + maxId + "/";
        string arvErrPath = "/var/spool/scigw/ErrArvTask/" + maxId + "/";

        string start        = pack.JsonValue["time"].asString();
        string end          = pack.JsonValue["end"].asString();
        string check        = pack.JsonValue["check"].asString();
        string intervaltime = pack.JsonValue["intervaltime"].asString();
        string modifyTime   = pack.JsonValue["modifyTime"].asString();
        string runTime;

        TimeOpr tOpr;
        int s = tOpr.htom(start);
        int e = tOpr.htom(end);
        char r[2] = {0};
        if (e <= s) {
                e += 24*60; 
        }
        float run = (float)(e - s) / 60;
        //float run = ((e - s)%60 == 0)? ((e - s)/60): ((float)(e - s)/60);
        sprintf(r, "%0.2f", run);
        runTime = r;

        if (parser.SetVal(GATEWAY_ARCHIVE, START_TIME, start) != 0 
                || parser.SetVal(GATEWAY_ARCHIVE, RUN_TIME, runTime) != 0 
                || parser.SetVal(GATEWAY_ARCHIVE, END_TIME, end) != 0 
                || parser.SetVal(GATEWAY_ARCHIVE, INTERVAL_TIME, intervaltime) != 0 
                || parser.SetVal(GATEWAY_ARCHIVE, CRON_TIME, intervaltime) != 0 
                || parser.SetVal(GATEWAY_ARCHIVE, IS_CHECK, check) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, MODIFY_TIME, modifyTime) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, ARVERR_PATH, arvErrPath) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, ARVFINISHED_PATH, finishPath) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, ARVDEV_PATH, devPath) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, FILTER, filterPath) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, PID_PATH, pidPath) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, ARVTASK_PATH, arvPath) != 0
                || parser.SetVal(GATEWAY_ARCHIVE, MOVE_LOG_PATH, moveLogPath) != 0) {
                LOG_ERROR("Write config file Error!");
                if (!SendErr("生成归档配置文件失败!")) {
                        LOG_ERROR("Send Error!");
                }
                return 0;
        } 

        PathPairParser ppParser(devPath);
        FilterParser fParser(filterPath);
        string srcPath;
        string desPath;
        map<string, string> arvPathMap;                                 //srcPath and desPath map
        vector<string> filterCondition;
        map<string, vector<string> > filter;                            //field name and conditions map

        //source archive path and destination archive path
        for (size_t i = 0; i < pack.JsonValue["archiveSrcdir"].size(); ++i) {
                string srcPath = pack.JsonValue["archiveSrcdir"][i].asString();
                string desPath = pack.JsonValue["archiveDestdir"][i].asString();
                arvPathMap.insert(make_pair(srcPath, desPath));
        }
        ppParser.Write(arvPathMap);

        //condition filter path
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["filterPath"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["filterPath"][i].asString()); 
        }
        filter.insert(make_pair(FILTER_PATH, filterCondition));

        //condition filter type
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["filterType"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["filterType"][i].asString());
        }
        filter.insert(make_pair(FILTER_TYPE, filterCondition));

        //condition demand path
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["demandPath"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["demandPath"][i].asString());
        }
        filter.insert(make_pair(DEMOND_PATH, filterCondition));

        //condition demand type
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["demandType"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["demandType"][i].asString());
        }
        filter.insert(make_pair(DEMOND_TYPE, filterCondition));

        fParser.Write(filter);

        if (!SendOk()) {
                LOG_ERROR("Send Error!");
        }
        return 0;
}

int StorageGW::ExecuteDataStart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDataStart!---", pack);

        string configPath = pack.JsonValue["configPath"].asString();

        Cront ct;
        if (ct.AddCront(configPath) != 0) {
                LOG_ERROR("add to cront error"); 
                SendErr("添加归档任务到crontab中失败!");
                return -2;
        }
        SendOk();
        return 0;
}

int StorageGW::ExecuteDataStop(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDataStop!---", pack);

        int ret = 0;
        string configPath = pack.JsonValue["configPath"].asString();

        Cront ct;
        ret = ct.DelCront(configPath);
        if (ret != 0) {
                LOG_ERROR("DelCront Error!"); 
                SendErr("停止归档失败，无法停止crontab");
                return 0;
        }
        SendOk();
        return 0;
}

int StorageGW::ExecuteDataDelete(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDataDelete!---", pack);
        int ret = 0;

        Cront ct;
        MultiTaskArvOpr mtao;
        string configPath = pack.JsonValue["configPath"].asString();

        ret = ct.DelCront(configPath);
        if (ret != 0) {
                LOG_ERROR("DelCront Error!"); 
                SendErr("停止归档失败，无法停止crontab");
        }

        ret = mtao.DeleteArchiveInfo(configPath);
        if (ret != 0) {
                LOG_ERROR("DeleteArchiveInfo Error!"); 
                SendErr("删除数据库信息失败!");
        }
        SendOk();
        return 0;
}

int StorageGW::ExecuteDataConfigList(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDataConfigList!---", pack);

        int ret = 0;
        DirFileOpr dfOpr;
        ChildProcessOpr cpo;
        MultiTaskArvOpr mtao;

        map<string, pair<string,string> > arvInfoMap;
        ret = mtao.GetArchiveInfoMap(arvInfoMap);
        if (ret < 0) {
                LOG_ERROR("GetArchiveInfoMap Error!");
                SendErr("获取配置文件列表失败!");
                return ret;
        }

        Cront ct;
        string cmd;
        string status;

        Pack data;
        data.PkHead.Type = CMD_OK;
        map<string, pair<string,string> >::iterator mssIter = arvInfoMap.begin();

        for (; mssIter != arvInfoMap.end(); ++mssIter) {
                Json::Value config;
                string configPath  = mssIter->first;
                cmd = "/usr/sbin/DDFSArchive " + configPath;
                if (ct.Find(cmd) == 1) {
                        if ( cpo.IsTaskRunning(cmd) ) {
                                status = "2";
                        } else {
                                status = "1";
                        }
                } else {
                        status = "0";
                }

                config["id"]         = mssIter->second.first;
                config["configPath"] = configPath;
                config["ip"]         = mssIter->second.second;
                config["status"]     = status;

                data.JsonValue["configList"].append(config);
        }
        m_PackMgr->Send(data);

        cout << data.JsonValue.toStyledString() << endl;
        return 0;
}

int StorageGW::ExecuteDataModify(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteDataModify!---", pack);

        TimeOpr tOpr;
        DirFileOpr dfOpr;

        int ret = 0;
        /* 判断源是否存在 */
        for (size_t i = 0; i < pack.JsonValue["archiveSrcdir"].size(); ++i) {
                string srcPath = pack.JsonValue["archiveSrcdir"][i].asString();
                if (!dfOpr.HasPath(srcPath)) {
                        string errMsg = "归档源路径不存在: " + srcPath;
                        SendErr(errMsg);
                        return 0;
                }
                if ( dfOpr.IsBlockDevice(srcPath) ) {
                        string errMsg = "归档源路径不能为设备! [" + srcPath + " ]";
                        SendErr(errMsg);
                        return 0;
                }
        }

        /* 判断目的是否存在 */
        for (size_t i = 0; i < pack.JsonValue["archiveSrcdir"].size(); ++i) {
                string destPath = pack.JsonValue["archiveDestdir"][i].asString();
                if (!dfOpr.HasPath(destPath)) {
                        string errMsg = "归档目标路径不存在: " + destPath;
                        SendErr(errMsg);
                        return 0;
                }
                if ( dfOpr.IsBlockDevice(destPath) ) {
                        string errMsg = "归档目的不能为设备! [" + destPath + " ]";
                        SendErr(errMsg);
                        return 0;
                }
        }

        string start        = pack.JsonValue["time"].asString();
        string end          = pack.JsonValue["end"].asString();
        string check        = pack.JsonValue["check"].asString();
        string intervaltime = pack.JsonValue["intervaltime"].asString();
        string modifyTime   = pack.JsonValue["modifyTime"].asString();
        string runTime;

        /* 通过该字段来区分是迁移还是修改配置文件 */
        string destIp     = pack.JsonValue["destIp"].asString();
        string id         = pack.JsonValue["id"].asString();

        /* 通过编号获取当前配置文件路径 */
        string configPath = "/etc/scigw/config/" + id;
        string syncPath   = configPath;

        /* 编辑该配置文件 */
        string devPath     = configPath + "/archive.cnf";
        string filterPath  = configPath + "/filter.cnf";
        configPath         = configPath + "/GWconfig";

        IniParser parser(configPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                SendErr("打开配置文件失败");
                return ret;
        }

        int s = tOpr.htom(start);
        int e = tOpr.htom(end);
        char r[2] = {0};
        if (e <= s) {
                e += 24*60; 
        }
        float run = (float)(e - s) / 60;
        sprintf(r, "%0.2f", run);
        runTime = r;

        if (parser.SetVal(GATEWAY_ARCHIVE, START_TIME, start) != 0 
                                || parser.SetVal(GATEWAY_ARCHIVE, RUN_TIME, runTime) != 0 
                                || parser.SetVal(GATEWAY_ARCHIVE, END_TIME, end) != 0 
                                || parser.SetVal(GATEWAY_ARCHIVE, INTERVAL_TIME, intervaltime) != 0 
                                || parser.SetVal(GATEWAY_ARCHIVE, CRON_TIME, intervaltime) != 0 
                                || parser.SetVal(GATEWAY_ARCHIVE, IS_CHECK, check) != 0
                                || parser.SetVal(GATEWAY_ARCHIVE, MODIFY_TIME, modifyTime) != 0 ) {
                LOG_ERROR("Write config file Error!");
                SendErr("生成归档配置文件失败!");
                return 0;
        } 

        PathPairParser ppParser(devPath);
        FilterParser fParser(filterPath);
        string srcPath;
        string desPath;
        map<string, string> arvPathMap;                                 //srcPath and desPath map
        vector<string> filterCondition;
        map<string, vector<string> > filter;                            //field name and conditions map

        //source archive path and destination archive path
        for (size_t i = 0; i < pack.JsonValue["archiveSrcdir"].size(); ++i) {
                string srcPath = pack.JsonValue["archiveSrcdir"][i].asString();
                string desPath = pack.JsonValue["archiveDestdir"][i].asString();
                arvPathMap.insert(make_pair(srcPath, desPath));
        }
        ppParser.Write(arvPathMap);

        //condition filter path
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["filterPath"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["filterPath"][i].asString()); 
        }
        filter.insert(make_pair(FILTER_PATH, filterCondition));

        //condition filter type
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["filterType"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["filterType"][i].asString());
        }
        filter.insert(make_pair(FILTER_TYPE, filterCondition));

        //condition demand path
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["demandPath"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["demandPath"][i].asString());
        }
        filter.insert(make_pair(DEMOND_PATH, filterCondition));

        //condition demand type
        filterCondition.clear();
        for (size_t i = 0; i < pack.JsonValue["demandType"].size(); ++i) {
                filterCondition.push_back(pack.JsonValue["demandType"][i].asString());
        }
        filter.insert(make_pair(DEMOND_TYPE, filterCondition));

        fParser.Write(filter);

        SendOk();
        return 0;
}

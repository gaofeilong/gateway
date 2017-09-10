#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/Lock.h"
#include "Config/PathPairParser.h"
#include "Archive/Scanner/Scanner.h"
#include "Archive/Email/LogManager.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "GatewayBG/Operation/DDFSInfo.h"
#include "GatewayBG/Operation/DDFSMgr.h"
#include "GatewayBG/Operation/DDFSLogOpr.h"
#include "Archive/DataArchive/ArchiveMgr.h"
#include "GatewayBG/Operation/VersionMgr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

int CheckDDFSMpState(const string& arvDevPath);

int main(int argc, char* argv[])
{
        if (argc != 2) {
                printf("argv[0]=DDFSArchive\n");
                printf("argv[1]=配置文件路径\n");
                return 0;
        }

        int ret = 0;

        string configPath = argv[1];
        string intervalTime, cronTime, pidPath;
        string arvFinishedPath, arvDevPath;

        IniParser iniParser(configPath);
        iniParser.Init();
        
        iniParser.GetVal("GatewayArchive", "intervalTime", intervalTime);
        iniParser.GetVal("GatewayArchive", "cronTime", cronTime);
        iniParser.GetVal("GatewayArchive", "pidPath", pidPath);
        iniParser.GetVal("GatewayArchive", "arvFinishedPath", arvFinishedPath);
        iniParser.GetVal("GatewayArchive", "arvDevPath", arvDevPath);

        Lock lockFile;
        if (lockFile.LockFile(pidPath) == LOCK_FAIL) {
                LOG_INFO("archive is running! ");
                return 0;
        }

        /* 检查目标的状态 */
        if ( CheckDDFSMpState(arvDevPath) < 0) {
                //LOG_INFO("挂载点在非法状态");
                LOG_INFO("归档目标不是一个目录");
                return 0;
        }

        if (intervalTime == cronTime) {
                Scanner scan(configPath);
                ret = scan.GetAllArvFile();
                if(ret < 0) {
                        LOG_ERROR("GetAllArvFile Error!");
                        return ret;
                }

                iniParser.SetVal("GatewayArchive", "cronTime", "1");
                ArchiveMgr archiveMgr(configPath);
                archiveMgr.Start();
        } else {
                int cron = atoi(cronTime.c_str());
                cron  = cron + 1;
                char buf[10];
                sprintf(buf, "%d", cron);
                iniParser.SetVal("GatewayArchive", "cronTime", buf);
        }

        /* 获取DDFS日志信息并插入数据库 */
        DDFSLogOpr logOpr;
        ret = logOpr.InsertLogToDb();
        if (ret < 0) {
                LOG_ERROR("InsertLogToDb Error!");
        }
        return ret;
}

int CheckDDFSMpState(const string& arvDevPath)
{
        /* 判断挂载点目前的状态 */
        IniParser ini(arvDevPath);
        ini.Init();

        map<string, string> arvPath; PathPairParser ppParser(arvDevPath);
        ppParser.Read(arvPath);

        if (arvPath.empty()) {
                LOG_INFO("don't have mp!");
                return -1;
        }
        string mp = (arvPath.begin())->second;

        DirFileOpr dfOpr;
        if (!dfOpr.IsDir(mp)) {
                return -2;
        }
        return 0;

        /*
        DDFSMgr ddfsMgr;
        int ret = ddfsMgr.Init();
        if (ret < 0) {
                LOG_ERROR("ddfsMgr Init Error!");
                return -2;
        }

        if (ddfsMgr.GetMPState(mp.c_str()) != MOUNTED) {
                LOG_INFO("ddfs mountpoint don't mount, can't archive!");
                return -3;
        }
        */
}

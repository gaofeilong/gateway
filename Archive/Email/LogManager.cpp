#include <time.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "Utils/Log/Log.h"
#include "Utils/DB/DBOpr.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/TimeOpr.h"
#include "Archive/Email/LogManager.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

#define errLogPath      "/etc/scigw/Log/LogError/"
#define warLogPath      "/etc/scigw/Log/LogWarning/"
#define EmailTimeConfig "/etc/scigw/Log/Config/EmailTimeConfig.dat"

using std::ifstream;

LogManager::LogManager()
{

}

LogManager::~LogManager()
{

}

int LogManager::InitErrEmailTimeConfig()
{
        return 0;
}

int LogManager::InitEmailTimeConfig()
{
        int ret = 0;
        DirFileOpr dfOpr;
        if ( dfOpr.HasPath(EmailTimeConfig)) {
                ret = ReadEmailFile(m_SendType, m_SendTime, m_SendDay);
                if (ret < 0) {
                        LOG_ERROR("ReadEmailFile Error! path=" << EmailTimeConfig);
                }
        }
        return ret;
}

int LogManager::InitWarEmailTimeConfig()
{
        return 0;
}

int LogManager::WriteErrorLog(const string& content)
{
        int ret =0;
        DirFileOpr dfOpr;
        if ( !dfOpr.HasPath(errLogPath) ) {
                ret = dfOpr.MakeDir(errLogPath);
                if (ret < 0) {
                        LOG_ERROR("MkDir Error! path=" << errLogPath);
                        return ret;
                }
        }

        string errFileName;
        GetLogErrFileName(errFileName);

        //写入文件
        ret = WriteLog(errFileName, content);
        if (ret < 0) {
                LOG_ERROR("WriteLog Error! path=" << errFileName);
                return ret;
        }

        if (m_SendType == "1") {
                //插入数据库
                ret = Insert(1, content);
                if (ret < 0) {
                        LOG_ERROR("Insert Error");
                }
                //发送邮件
                string cmd = "/usr/sbin/send_error.sh";
                ChildProcessOpr cmdOpr;
                ret = cmdOpr.ExecuteCmd(cmd);
                if (ret < 0) {
                        LOG_ERROR("ExecuteCmd Error!");
                        return ret;
                }
        } else {
                //插入数据库
                ret = Insert(1, content);
                if (ret < 0) {
                        LOG_ERROR("Insert Error");
                        return ret;
                }
        }

        return 0;
}

int LogManager::WriteWaringLog(const string& content)
{
        int ret = 0;
        DirFileOpr dfOpr;
        if ( !dfOpr.HasPath(warLogPath) ) {
                ret = dfOpr.MakeDir(warLogPath);
                if (ret < 0) {
                        LOG_ERROR("MkDir Error! path=" << warLogPath);
                        return ret;
                }
        }

        string warFileName;
        GetLogWarFileName(warFileName);
        //写入文件
        ret = WriteLog(warFileName, content);
        if (ret < 0) {
                LOG_ERROR("WriteLog Error! path=" << warFileName);
                return ret;
        }

        if (m_SendType == "1") {
                //插入数据库
                ret = Insert(2, content);
                if (ret < 0) {
                        LOG_ERROR("Insert Error");
                }
                //发送邮件
                string cmd = "/usr/sbin/send_warning.sh";
                ChildProcessOpr cmdOpr;
                ret = cmdOpr.ExecuteCmd(cmd);
                if (ret < 0) {
                        LOG_ERROR("ExecuteCmd Error!");
                        return ret;
                }
        } else {
                //插入数据库
                ret = Insert(2, content);
                if (ret < 0) {
                        LOG_ERROR("Insert Error");
                        return ret;
                }
        }
        return 0;
}

int LogManager::SendErrorEmail()
{
        return 0;
}

int LogManager::SendWaringEmail()
{
        return 0;
}

void LogManager::GetLogErrFileName(string& errFileName)
{
        string curTime;
        TimeOpr tOpr;
        tOpr.GetCurDay(curTime);
        errFileName = errLogPath + curTime + ".dat";
}

void LogManager::GetLogWarFileName(string& warFileName)
{
        string curTime;
        TimeOpr tOpr;
        tOpr.GetCurDay(curTime);
        warFileName = warLogPath + curTime + ".dat";
}

int LogManager::WriteLog(const string& fileName, const string& content)
{
        int ret = 0;
        FILE* file = fopen(fileName.c_str(), "a");
        if (file == NULL) {
                LOG_ERROR("fopen error! fileName=" << fileName);
                return -1;
        }
        string curTime;
        TimeOpr tOpr;
        tOpr.GetCurSec(curTime);

        BaseOpr baseOpr;
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId() Error!");
                return ret;
        }

        string ip;
        ret = baseOpr.GetIp(networkId, ip);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                return ret;
        }

        fprintf(file, "[%s  %s  %s]\n", curTime.c_str(), ip.c_str(), content.c_str());

        ret = fclose(file);
        if (ret < 0) {
                LOG_ERROR("fclose error! ret=" << ret);
                return -2;
        }
        return 0;
}

int LogManager::GetCrontabContent(map<string, char>& crontabContentMap)
{
        int ret = 0;
        FILE* file = popen("crontab -l", "r");
        if (file == NULL) {
                return 0;
        }

        const  int len = 256;
        char   buffer[len];
        string line;

        while ( fgets(buffer, len, file) ) {
                line = buffer;
                if ( line.find("no") != string::npos ) {
                        pclose(file);
                        return 0;
                }
                crontabContentMap.insert( make_pair(line, 'c') );
        }
        ret = pclose(file);
        if (ret < 0) {
                LOG_ERROR("pclose error!");
                return ret;
        }
        return 0;
}

int LogManager::WriteCrontabContent(const string& path, const map<string, char>& crontabContentMap)
{
        int ret = 0;

        FILE* file = fopen(path.c_str(), "w+");
        if (file == NULL) {
                LOG_ERROR("fopen error!");
                return -1;
        }

        map<string, char>::const_iterator msIter = crontabContentMap.begin();

        for (; msIter != crontabContentMap.end(); ++msIter) {
                fprintf( file, "%s", (msIter->first).c_str() );
        }

        ret = fclose(file);
        if (ret < 0) {
                LOG_ERROR("fclose error!");
                return -2;
        }
        return 0;
}

int LogManager::AddCrontab(const string& cmd, map<string, char>& crontabContentMap, const string& sendType, 
        const string& sendTime, const string& sendDay)
{
        int ret = 0;

        string data;
        int    hour = 0;
        int    min  = 0;
        int    day  = 0;
        char buffer[256];

        if (sendType == "2") {
                ret = ParseSendTime(sendTime, hour, min);
                if (ret < 0) {
                        LOG_ERROR("ParseSendTime Error!");
                        return ret;
                }
                sprintf(buffer, "%d %d * * * %s\n", min, hour, cmd.c_str());
                data = buffer;
        } else if (sendType == "3") {
                ret = ParseSendTime(sendTime, hour, min);
                if (ret < 0) {
                        LOG_ERROR("ParseSendTime Error!");
                        return ret;
                }
                day = atoi(sendDay.c_str());
                if (day == 7) {
                        day = 0;
                }
                sprintf(buffer, "%d %d * * %d %s\n", min, hour, day, cmd.c_str());
                data = buffer;
        }

        size_t idx = 0;
        map<string, char>::iterator mscIter = crontabContentMap.begin();
        for ( ; mscIter != crontabContentMap.end(); ++mscIter) {
                idx = mscIter->first.find(cmd);
                if (idx != string::npos) {
                        crontabContentMap.erase(mscIter);
                        break; 
                }
        }
        crontabContentMap.insert( make_pair(data, 'c') );
        return 0;
}

int LogManager::ParseSendTime(const string& sendTime, int& hour, int& min)
{
        size_t idx = sendTime.find(":");
        if (idx == string::npos) {
                LOG_ERROR("sendTime format error!");
                return -1;
        }
        string strHour = sendTime.substr(0, idx);
        string strMin  = sendTime.substr(idx+1);

        hour = atoi(strHour.c_str());
        min  = atoi(strMin.c_str());

        return 0;
}

int LogManager::ExecWarCrontab()
{
        return 0;
}

int LogManager::ExecErrCrontab()
{
        return 0;
}

int LogManager::ExecCrontab()
{
        int ret = 0;
        string cmd = "/usr/sbin/run_sci_check.sh check && /usr/sbin/run_sci_check.sh send";
        ret = ExecCrontab(cmd, m_SendType, m_SendTime, m_SendDay);
        if (ret < 0) {
                LOG_ERROR("ExecCrontab Error! ret=" << ret);
        }
        return ret;
}

int  LogManager::ExecCrontab(const string& cmd, const string& warSendType, const string& warSendTime, const string& warSendDay)
{
        int ret = 0;
        map<string, char> crontabContentMap;
        ret = GetCrontabContent(crontabContentMap);
        if (ret < 0) {
                LOG_ERROR("GetCrontabContent Error!");
                return ret;
        }

        ret = AddCrontab(cmd, crontabContentMap, warSendType, warSendTime, warSendDay);
        if (ret < 0) {
                LOG_ERROR("AddCrontab Error!");
                return ret;
        }

        string tmp = "/tmp/crontab.tmp";
        ret = WriteCrontabContent(tmp, crontabContentMap);
        if (ret < 0) {
                LOG_ERROR("WriteCrontabContent Error!");
                return ret;
        }

        string cmdLine = "crontab " + tmp;
        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmdLine);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! cmd=" << cmdLine);
                return ret;
        }
        return 0;
}

int LogManager::ReadEmailFile(string& sendType)
{
        string sendTime;
        string sendDay;
        return ReadEmailFile(sendType, sendTime, sendDay);
}

int LogManager::ReadEmailFile(string& sendType, string& sendTime, string& sendDay)
{
        DirFileOpr dfOpr;
        if (!dfOpr.HasPath(EmailTimeConfig)) {
                return 0;
        }

        ifstream fin(EmailTimeConfig);
        if (!fin) {
                LOG_ERROR("open file error! path=" << EmailTimeConfig); 
                return 0; 
        }

        int  idx = 0;
        int  cnt = 1;
        string line;
        string data;

        while (getline(fin, line)) {
                idx  = line.find("="); 
                data = line.substr(idx+1);
                if (cnt == 1) {
                        sendType = data;
                } else if (cnt == 2) {
                        sendTime = data;
                } else if (cnt == 3) {
                        sendDay  = data;
                }
                ++cnt;
        }
        return 0;
}

int LogManager::Insert(int emailType, const string& content)
{
        int ret = 0;
        char buf[4096];
        string curTimeTmp;
        string contentTmp;

        //创建数据库操作对象
        DBOpr db;
        ret = db.Init();
        if (ret < 0) {
                LOG_ERROR("DB Init Error!");
                return ret;
        }

        ret = db.Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error!");
                return ret;
        }

        string curTime;
        TimeOpr tOpr;
        tOpr.GetCurSec(curTime);
        contentTmp = "'" + content + "'";
        curTimeTmp = "'" + curTime + "'";

        if (emailType == 1) {
                sprintf(buf, "insert into sci_email(type,date,info) values(%d,%s,%s)", 1, curTimeTmp.c_str(), contentTmp.c_str());
        } else {
                sprintf(buf, "insert into sci_email(type,date,info) values(%d,%s,%s)", 2, curTimeTmp.c_str(), contentTmp.c_str());
        }

        ret  = db.Query(buf);
        if (ret < 0) {
                LOG_ERROR("Query Error!");
                db.Close();
                return ret;
        }
        db.Close();

        return 0;
}

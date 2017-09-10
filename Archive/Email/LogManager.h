#ifndef _LOG_MANAGER_H_
#define _LOG_MANAGER_H_

#include <map>
#include <string>

using std::map;
using std::string;

class LogManager
{
public:
        LogManager();
        ~LogManager();

        int InitEmailTimeConfig();
        int InitErrEmailTimeConfig();
        int InitWarEmailTimeConfig();

        int ExecCrontab();
        int ExecErrCrontab();
        int ExecWarCrontab();
        int SendErrorEmail();
        int SendWaringEmail();
        int WriteErrorLog(const string& content);
        int WriteWaringLog(const string& content);

        int  ReadEmailFile(string& sendType);
        int  GetCrontabContent(map<string, char>& crontabContentMap);
private:
        int  Insert(int emailType, const string& content);

        int  ExecCrontab(const string& cmd, const string& warSendType, const string& warSendTime, const string& warSendDay);

        void GetLogWarFileName(string& errFileName);
        void GetLogErrFileName(string& warFileName);
        int  WriteLog(const string& fileName, const string& content);

        int  WriteCrontabContent(const string& path, const map<string, char>& crontabContentMap);
        int  AddCrontab(const string& cmd, map<string, char>& crontabContentMap, 
                const string& sendType, const string& sendTime, const string& sendDay);

        int  ParseSendTime(const string& sendTime, int& hour, int& min);
        int  ReadEmailFile(string& sendType, string& sendTime, string& sendDay);

private:
        string          m_SendType;
        string          m_SendTime;
        string          m_SendDay;
};

#endif

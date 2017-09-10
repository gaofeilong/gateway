#include <stdlib.h>
#include <iostream>
#include "Cront.h"
#include "errno.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"
#include "Utils/Log/Log.h"
using namespace std;

Cront::Cront()
{
}

Cront::~Cront()
{
}

int Cront::AddCront(const string& configPath)
{
        string time;
        vector<string> commands;
        const string tmpFileName = "/tmp/crontab.tmp";
        IniParser par(configPath); 

        int ret = par.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        if (par.GetVal("GatewayArchive", "startTime", time) !=0) {
                LOG_ERROR("get config info error"); 
                return -1;
        }

        FILE* stream = popen("crontab -l", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -2;
        }

        char   *lineBuf = NULL;
        size_t lineLen = 0;
        string line;
        string cmd = "source /root/.bash_profile && /usr/sbin/DDFSArchive " + configPath;
        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;
                line.erase(line.find('\n'));
                if (line.find(cmd) == string::npos) {
                        commands.push_back(line);
                }
        }
        free(lineBuf);

        pclose(stream);

        string min      = time.substr(time.find(":") + 1);
        string hour     = time.substr(0, time.find(":"));
        string strategy = min + " " + hour +  " * * * ";
        string arcline  = strategy + cmd;
        commands.push_back(arcline);

        FILE* fout = fopen(tmpFileName.c_str(), "w");
        if (fout == NULL) {
                LOG_ERROR("open stream error");
		return -1;
        }

        vector<string>::iterator iter = commands.begin();
        for ( ; iter != commands.end(); ++iter) {
                fprintf(fout, "%s\n", iter->c_str());
        }
        if (fclose(fout) != 0) {
                LOG_ERROR("close fout error"); 
                return -2;
        }

        ChildProcessOpr cmdOpr;
        cmd = "crontab " + tmpFileName;
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR("crontab error");
                return -3;
        }
        return 0;
}

int Cront::DelCront(const string& configPath)
{
        const string tmpFileName = "/tmp/crontab.tmp";
        vector<string> commands;

        FILE* stream = popen("crontab -l", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -2;
        }

        char   *lineBuf = NULL;
        size_t lineLen  = 0;
        string line;
        string cmd;
        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf; 
                line.erase(line.find('\n'));
                cmd = "/usr/sbin/DDFSArchive " + configPath;
                if (line.find(cmd) == string::npos) {
                        commands.push_back(line);
                }
        }
        free(lineBuf);

        if (pclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
                return -3;
        }

        FILE* fout = fopen(tmpFileName.c_str(), "w");
        if (fout == NULL) {
                LOG_ERROR("open stream error");
		return -1;
        }

        vector<string>::iterator iter = commands.begin();
        for ( ; iter != commands.end(); ++iter) {
                fprintf(fout, "%s\n", iter->c_str());
        }
        if (fclose(fout) != 0) {
                LOG_ERROR("close fout error"); 
                return -2;
        }

        ChildProcessOpr cmdOpr;
        cmd = "crontab " + tmpFileName;
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR("crontab error");                 
                return -3;
        }
        return 0;
}

int Cront::Find(const string& cmd)
{
        int ret = 0;
        vector<string> commands;
        FILE* stream = popen("crontab -l", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -2;
        }

        char   *lineBuf = NULL;
        size_t lineLen  = 0;
        string line;
        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf; 
                line.erase(line.find('\n'));
                if (line.find(cmd) != string::npos) {
                        ret = 1;
                        break;
                }
        }
        free(lineBuf);

        if (pclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
                return -3;
        }
        return ret;
}

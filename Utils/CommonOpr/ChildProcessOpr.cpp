#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

ChildProcessOpr::ChildProcessOpr()
{

}

ChildProcessOpr::~ChildProcessOpr()
{

}

int ChildProcessOpr::ExecuteCmd(const string& cmd)
{
        int ret = 0;
        int status = 0;

        int pid = fork();
        if (pid < 0) {
                LOG_ERROR("fork error!");
                return -1; 
        } else if (pid == 0) { 
                execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)0);
        }
        ret = waitpid(pid, &status, 0);
        if(ret == -1 || (WIFEXITED(status)&&(WEXITSTATUS(status)!=0))) {
                LOG_ERROR("exec error! cmd=" << cmd << " ,errInfo=" << strerror(errno));
                return -2;
        } 

        return 0;
}

bool ChildProcessOpr::IsServiceRunning(const string& cmd)
{
        int ret = 0;
        int status = 0;

        int pid = fork();
        if (pid == 0) { 
                execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)0);
        }
        ret = waitpid(pid, &status, 0);
        if(!WIFEXITED(status)) {    
                LOG_ERROR("get service status error! cmd=" << cmd);
        } 
        return WEXITSTATUS(status) == 0? true: false;
}


bool ChildProcessOpr::IsTaskRunning(const string& taskKeyWord)
{
        bool status = false;
        string cmd = "ps aux | grep -v 'grep' | egrep \"" + taskKeyWord + "\"";

        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return status;
        }

        const int LEN = 256;
        char buffer[LEN] = {0};

        if (fgets(buffer, LEN, stream)) {
                status = true;
        }
        pclose(stream);
        return status;
}

int ChildProcessOpr::Popen(const string& cmd, vector<string>& infoList)
{
        FILE* file = popen(cmd.c_str(), "r");
        if (file == NULL) {
                LOG_ERROR("popen error! errInfo:" << strerror(errno));
                return -1;
        }

        int    idx = 0;
        string line;
        char   buffer[1024];
        while (fgets(buffer, 1024, file) != NULL) {
                line = buffer;
                idx = line.find("\n");
                line.erase(idx);
                infoList.push_back(line);   
        }
        pclose(file);
        return 0;
}

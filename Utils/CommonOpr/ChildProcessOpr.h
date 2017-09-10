#ifndef _CHILD_PROCESS_OPR_H_
#define _CHILD_PROCESS_OPR_H_

#include <string>
#include <vector>

using std::vector;
using std::string;

class ChildProcessOpr {
public:
        ChildProcessOpr();
        ~ChildProcessOpr();

public:
        int ExecuteCmd(const string& cmd);
        int Popen(const string& cmd, vector<string>& infoList);
        bool IsServiceRunning(const string& cmd);
        bool IsTaskRunning(const string& taskKeyWord);
};

#endif

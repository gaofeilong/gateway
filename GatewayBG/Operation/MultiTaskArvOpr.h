#ifndef _MULTI_TASK_ARV_OPR_H_
#define _MULTI_TASK_ARV_OPR_H_
#include <map>
#include <string>
#include "DBInfo.h"

#define  PATH_EXIST 1

using std::map;
using std::string;

class MultiTaskArvOpr {
public:
        MultiTaskArvOpr();
        ~MultiTaskArvOpr();

public:
        int GetDBOpr(DBOpr** dbOpr);
        int GetMaxID(string& maxId);
        int CopyDefaultConfig(string& path);
        int SyncToOtherNode(const string& syncPath);
        int InsertArchiveInfo(const ArchiveInfo& archiveInfo);
        int UpdateArchiveInfo(const ArchiveInfo& archiveInfo);
        int DeleteArchiveInfo(const string& configPath);
        int GetNodeIdByIp(const string& ip, string& nodeId);

        //int GetArchiveInfoMap(map<string, string>& arvInfoMap);
        int GetArchiveInfoMap(map<string, std::pair<string,string> >& arvInfoMap);

        int FindSrcPathInConfig(const string& configPath, const string& srcPath);
        int CheckSrcPath(const string& srcPath, string& errInfo, const string& curConfig);

private:

};

#endif

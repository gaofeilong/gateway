#ifndef _DDFS_MANAGER_H_
#define _DDFS_MANAGER_H_

#include "GatewayBG/Protocol/Protocol.h"

#define  DATA_PATH_EXIST            1
#define  DATA_PATH_NOT_HAVE_SPACE   2

class DDFSManager : public Protocol
{
public:
        DDFSManager(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr);
        int Execute(const Pack& pack);
private:
        int MountPointParse(const Pack& pack);
        int ConfigParse(const Pack& pack);
        int OperatorMPParse(const Pack& pack);

        int ExecuteMPList(const Pack& pack);
        int ExecuteMPAllList(const Pack& pack);
        int ExecuteMPOnlineList(const Pack& pack);

        int ExecuteMPInfo(const Pack& pack);

        int ExecuteMPAdd(const Pack& pack);
        int ExecuteMPModify(const Pack& pack);
        int ExecuteMPLookup(const Pack& pack);

        int ExecuteMPMount(const Pack& pack);
        int ExecuteMPUMount(const Pack& pack);
        int ExecuteMPFsck(const Pack& pack);

        int ExecuteMPDelete(const Pack& pack);
        int ExecuteClassifyMp(const Pack& pack);

private:
        int SetDDFSConfigForService();
        int IsDevUsed(const vector<string>& dataList, string& existPath);
        int IsSpaceLeft(const vector<string>& dataList, string& leftPath, int iMinSpace);
};

#endif

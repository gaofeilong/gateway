#ifndef _LOG_MGR_H_
#define _LOG_MGR_H_

#include "GatewayBG/Protocol/Protocol.h"

class LogMgr : public Protocol
{
public:
        LogMgr(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr);
        int Execute(const Pack& pack);
private:
        int SystemLogParse(const Pack& pack);
        int SystemWarParse(const Pack& pack);

        int ExecuteSystemLog(const Pack& pack);
        int ExecuteSystemWar(const Pack& pack);
};

#endif

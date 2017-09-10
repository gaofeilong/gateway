#ifndef _GW_BASE_INFO_H_
#define _GW_BASE_INFO_H_

#include <list>
#include <string>
#include "GatewayBG/Protocol/Protocol.h"
#include "GatewayBG/Operation/NetMgr.h"

using std::list;
using std::string;

class GWBaseInfo : public Protocol
{
public:
        GWBaseInfo(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr);
        int Execute(const Pack& pack);
private:
        int NetInfoParse(const Pack& pack);
        int LogParse(const Pack& pack);
        int EmailParse(const Pack& pack);
        int NFSParse(const Pack& pack);
        int MultiPathParse(const Pack& pack);
        int SSDCacheParse(const Pack& pack);

        int ExecuteLogInfo(const Pack& pack);
        int ExecuteLogWar(const Pack& pack);
        int ExecuteLogErr(const Pack& pack);
        int ExecuteLogErrLookup(const Pack& pack);
        int ExecuteLogWarLookup(const Pack& pack);

        int ExecuteEmailSrvLookup(const Pack& pack);
        int ExecuteEmailSrvSetting(const Pack& pack);
        int ExecuteEmailConfig(const Pack& pack);
        int ExecuteEmailLookup(const Pack& pack);

        int ExecuteNetInfoLookup(const Pack& pack);
        int ExecuteNetInfoConfig(const Pack& pack);

        int ExecuteNFSSetting(const Pack& pack);
        int ExecuteNFSStart(const Pack& pack);
        int ExecuteNFSStop(const Pack& pack);
        int ExecuteNFSLookup(const Pack& pack);

        int ExecuteMultiPathStart(const Pack& pack);
        int ExecuteMultiPathStop(const Pack& pack);
        int ExecuteMultiPathLookup(const Pack& pack);

        int ExecuteSSDCacheCreate(const Pack& pack);
        int ExecuteSSDCacheDestroy(const Pack& pack);
        int ExecuteSSDCacheLoad(const Pack& pack);
        int ExecuteSSDCacheUnload(const Pack& pack);
        int ExecuteSSDCacheLookup(const Pack& pack);

};

#endif

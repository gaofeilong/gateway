#ifndef _GW_STATUS_INFO_H_
#define _GW_STATUS_INFO_H_

#include "GatewayBG/Protocol/Protocol.h"

class GWStatusInfo : public Protocol
{
public:
        GWStatusInfo(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr);
        int Execute(const Pack& pack);
private:
        int GateWayParse(const Pack& pack);
        int AutorizationParse(const Pack& pack);

        int ExecuteGateWayInfo(const Pack& pack);
        int ExecuteMountPointInfo(const Pack& pack);
        int ExecuteAutorization(const Pack& pack);

        int ExecuteLicenseImport(const Pack& pack);
        int ExecuteLicenseExport(const Pack& pack);
        int ExecuteLicenseLookup(const Pack& pack);
        int ExecuteAddNode(const Pack& pack);
};

#endif

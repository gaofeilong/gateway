#ifndef _STORAGE_GW_H_
#define _STORAGE_GW_H_

#include "GatewayBG/Protocol/Protocol.h"

class StorageGW : public Protocol
{
public:
        StorageGW(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr);
        int Execute(const Pack& pack);
private:
        int DevParse(const Pack& pack);
        int NetCardParse(const Pack& pack);
        int DDFSParse(const Pack& pack);
        int ArchiveParse(const Pack& pack);

        int ExecuteDataStart(const Pack& pack);
        int ExecuteDataStop(const Pack& pack);
        int ExecuteDataLookup(const Pack& pack);
        int ExecuteArchiveData(const Pack& pack);
        int ExecuteDataDelete(const Pack& pack);
        int ExecuteDataModify(const Pack& pack);
        int ExecuteDataConfigList(const Pack& pack);

        int ExecuteDiskDriver(const Pack& pack);
        int ExecuteMountDev(const Pack& pack);
        int ExecuteUnMountDev(const Pack& pack);
        int ExecuteDeleteDev(const Pack& pack);

        int ExecuteFromatDev(const Pack& pack);
        int ExecuteCreatePartitionDev(const Pack& pack);

        int ExecuteNetCardInfo(const Pack& pack);
        int ExecuteNetPortConfig(const Pack& pack);
        int ExecuteNetCardTrunk(const Pack& pack);
        int ExecuteNetCardUnTrunk(const Pack& pack);

        int ExecuteDDFSStart(const Pack& pack);
        int ExecuteDDFSStop(const Pack& pack);
        int ExecuteDDFSLookup(const Pack& pack);

        int ExecuteDDFSRestart(const Pack& pack);
        int ExecuteDDFSForceStop(const Pack& pack);
        int ExecuteDDFSForceRestart(const Pack& pack);

        int ExecuteGWEngineRestart(const Pack& pack);
        int ExecuteGWEngineStop(const Pack& pack);
};

#endif

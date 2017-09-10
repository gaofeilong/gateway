#ifndef _SYSTEM_TOOL_H_
#define _SYSTEM_TOOL_H_

#include "Protocol.h"

class SystemTool : public Protocol
{
public:
        SystemTool(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr);
        int Execute(const Pack& pack);
private:
        int GWParse(const Pack& pack);

        int ExecuteReboot(const Pack& pack);
        int ExecuteShutDown(const Pack& pack);
        
};

#endif //_SYSTEM_TOOL_H_

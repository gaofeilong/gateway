#include <stdio.h>

#include "GatewayBG/Protocol/LogMgr.h"
#include "Utils/Log/Log.h"

LogMgr::LogMgr(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr)
        :Protocol(ddfsMgr, baseOpr)
{
        m_PackMgr = packMgr;
}

int LogMgr::Execute(const Pack& pack)
{
        //日志管理 0x4000000
        int ret = 0;

        switch(pack.PkHead.Type & ASSISTANT_SECOND) {
        case SYSTEM_LOG_SECOND:
                ret = SystemLogParse(pack);
                if (ret < 0) {
                        LOG_ERROR("SystemLogParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case SYSTEM_WAR_SECOND:
                ret = SystemWarParse(pack);
                if (ret < 0) {
                        LOG_ERROR("SystemWarParse Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int LogMgr::SystemLogParse(const Pack& pack)
{
        int ret = 0;
        //0x410000
        switch(pack.PkHead.Type) {
        case CMD_SYSTEM_LOG:
                ret = ExecuteSystemLog(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSystemLog Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int LogMgr::SystemWarParse(const Pack& pack)
{
        int ret = 0;
        //0x430000
        switch(pack.PkHead.Type) {
        case CMD_SYSTEM_WAR:
                ret = ExecuteSystemWar(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSystemWar Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int LogMgr::ExecuteSystemLog(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSystemLog!---", pack);
        return 0;
}

int LogMgr::ExecuteSystemWar(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSystemWar!---", pack);
        return 0;
}

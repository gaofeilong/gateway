#include <unistd.h>
#include <sys/wait.h>

#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "GatewayBG/Operation/DDFSInfo.h"
#include "GatewayBG/Protocol/SystemTool.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

SystemTool::SystemTool(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr)
        :Protocol(ddfsMgr, baseOpr)
{
        m_PackMgr = packMgr;
}

int SystemTool::Execute(const Pack& pack)
{
        boost::mutex::scoped_lock lock(m_Mutex);
        int ret = 0;

        switch(pack.PkHead.Type & ASSISTANT_SECOND) {
        case GW_SECOND:
                ret = GWParse(pack);
                if (ret < 0) {
                        LOG_ERROR("MountPointParse Error! ret=" << ret);
                        return ret;
                }
                break;
        }

        return 0;
}

int SystemTool::GWParse(const Pack& pack) 
{
        int ret = 0;

        switch(pack.PkHead.Type) {
        case CMD_GW_RESTART:
                ret = ExecuteReboot(pack);
                if (ret < 0) {
                        LOG_ERROR("MountPointParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_GW_SHUTDOWN:
                ret = ExecuteShutDown(pack);
                if (ret < 0) {
                        LOG_ERROR("MountPointParse Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int SystemTool::ExecuteReboot(const Pack& pack)
{
        DisplayExecuteInfo("----ExecuteReboot----", pack);
        int ret = 0;

        BaseOpr baseOpr;
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error!");
                return ret;
        }

        string ip;
        ret = baseOpr.GetIp(networkId, ip);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                ip = "unkonw ip";
        }

        ChildProcessOpr cmdOpr;
        if ( cmdOpr.IsTaskRunning("scidata_backup.sh|DDFSArchive") ) { 
                string errInfo = "[ " + ip + " ] 正在归档，无法重启服务端!";
                SendErr(errInfo);
                return ret;
        }

        SendOk();
        return ret;
}

int SystemTool::ExecuteShutDown(const Pack& pack)
{
        DisplayExecuteInfo("----ExecuteShutDown----", pack);
        
        int ret = 0;

        BaseOpr baseOpr;
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error!");
                return ret;
        }

        string ip;
        ret = baseOpr.GetIp(networkId, ip);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                ip = "unkonw ip";
        }

        ChildProcessOpr cmdOpr;
        if ( cmdOpr.IsTaskRunning("scidata_backup.sh|DDFSArchive") ) { 
                string errInfo = "[ " + ip + " ] 正在归档，无法关闭服务端!";
                SendErr(errInfo);
                return ret;
        }

        SendOk();
        return ret;
}

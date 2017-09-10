#include <stdio.h>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>

#include "Utils/Log/Log.h"
#include "Utils/DB/DBOpr.h"
#include "Utils/CommonOpr/TimeOpr.h"
#include "GatewayBG/Protocol/GWBaseInfo.h"
#include "Archive/Email/LogManager.h"
#include "GatewayBG/Operation/NFSMgr.h"
#include "GatewayBG/Operation/MultiPathMgr.h"
#include "GatewayBG/Operation/MailMgr.h"
#include "GatewayBG/Operation/SSDCacheMgr.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

#define LogConfigPath      "/etc/scigw/Log/Config/"
#define LogEmailConfig     "/etc/scigw/Log/Config/LogEmailConfig.dat"
#define EmailTimeConfig    "/etc/scigw/Log/Config/EmailTimeConfig.dat"

GWBaseInfo::GWBaseInfo(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr)
        :Protocol(ddfsMgr, baseOpr)
{
        m_PackMgr = packMgr;
}

int GWBaseInfo::Execute(const Pack& pack)
{
        //网关基础信息配置 0x2000000
        boost::mutex::scoped_lock lock(m_Mutex);
        int ret = 0;

        switch(pack.PkHead.Type & ASSISTANT_SECOND){ 
        case NETINFO_SECOND:
                ret = NetInfoParse(pack);
                if (ret < 0) {
                        LOG_ERROR("NetInfoParse Error! ret=" << ret);
                        return ret;
                }   
                break;
        case LOG_SECOND:
                ret = LogParse(pack);
                if (ret < 0) {
                        LOG_ERROR("LogParse Error! ret=" << ret);
                        return ret;
                }   
                break;
        case EMAIL_SECOND:
                ret = EmailParse(pack);
                if (ret < 0) {
                        LOG_ERROR("EmailParse Error! ret=" << ret);
                        return ret;
                }   
                break;
        case NFS_SECOND:
                ret = NFSParse(pack);
                if (ret < 0) {
                        LOG_ERROR("NFSParse Error! ret=" << ret);
                        return ret;
                }   
                break;
        case MULTIPATH_SECOND:
                ret = MultiPathParse(pack);
                if (ret < 0) {
                        LOG_ERROR("MultiPathParse Error! ret=" << ret);
                        return ret;
                }   
                break;
        case SSDCACHE_SECOND:
                ret = SSDCacheParse(pack);
                if (ret < 0) {
                        LOG_ERROR("SSDCacheParse Error! ret=" << ret);
                        return ret;
                }   
                break;
        }

        return 0;
}

int GWBaseInfo::NetInfoParse(const Pack& pack)
{
        int ret = 0;
        //0x2200000
        switch(pack.PkHead.Type) {
        case CMD_NETINFO_CONFIG:
                ret = ExecuteNetInfoConfig(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNetInfoConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NETINFO_LOOKUP:
                ret = ExecuteNetInfoLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNetInfoConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        }

        return 0;
}

int GWBaseInfo::LogParse(const Pack& pack)
{
        int ret = 0;
        //0x2500000
        switch(pack.PkHead.Type) {
        case CMD_LOG_INFO:
                ret = ExecuteLogInfo(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLogInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LOG_WAR:
                ret = ExecuteLogWar(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLogWar Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LOG_ERR:
                ret = ExecuteLogErr(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLogErr Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LOGERR_LOOKUP:
                ret = ExecuteLogErrLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLogLook Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LOGWAR_LOOKUP:
                ret = ExecuteLogWarLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLogLook Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int GWBaseInfo::EmailParse(const Pack& pack)
{
        int ret = 0;
        //0x2600000
        switch(pack.PkHead.Type) {
        case CMD_EMAIL_SRV_CONFIG:
                ret = ExecuteEmailSrvSetting(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteEmailSrvSetting Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_EMAIL_SRV_LOOKUP:
                ret = ExecuteEmailSrvLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteEmailSrvLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_EMAIL_CONFIG:
                ret = ExecuteEmailConfig(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteEmailConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_EMAIL_LOOKUP:
                ret = ExecuteEmailLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteEmailConfig Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int GWBaseInfo::NFSParse(const Pack& pack)
{
        int ret = 0;
        //0x27000000
        switch(pack.PkHead.Type) {
        case CMD_NFS_SAVE:
                ret = ExecuteNFSSetting(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNFSSAVE Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NFS_START:
                ret = ExecuteNFSStart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNFSStart Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NFS_STOP:
                ret = ExecuteNFSStop(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNFSStop Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_NFS_LOOKUP:
                ret = ExecuteNFSLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteNFSLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int GWBaseInfo::MultiPathParse(const Pack& pack)
{
        int ret = 0;
        //0x28000000
        switch(pack.PkHead.Type) {
        case CMD_MULTIPATH_START:
                ret = ExecuteMultiPathStart(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMultiPathStart Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MULTIPATH_STOP:
                ret = ExecuteMultiPathStop(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMultiPathStop Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MULTIPATH_LOOKUP:
                ret = ExecuteMultiPathLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMultiPathLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int GWBaseInfo::SSDCacheParse(const Pack& pack)
{
        int ret = 0;
        //0x29000000
        switch(pack.PkHead.Type) {
        case CMD_SSDCACHE_CREATE:
                ret = ExecuteSSDCacheCreate(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSSDCacheCreate Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_SSDCACHE_DESTROY:
                ret = ExecuteSSDCacheDestroy(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSSDCacheDestroy Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_SSDCACHE_LOAD:
                ret = ExecuteSSDCacheLoad(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSSDCacheLoad Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_SSDCACHE_UNLOAD:
                ret = ExecuteSSDCacheUnload(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSSDCacheUnload Create Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_SSDCACHE_LOOKUP:
                ret = ExecuteSSDCacheLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteSSDCacheLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int GWBaseInfo::ExecuteNetInfoConfig(const Pack& pack)
{
        DisplayExecuteInfo("---NetInfoConfig!---", pack);

        ChildProcessOpr cpo;
        if ( cpo.IsTaskRunning("scidata_backup|DDFSArchive") ) { 
                SendErr("设置IP失败!,归档运行中");
                return 0;
        }
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteLogInfo(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteLogInfo!---", pack);
        return 0;
}

int GWBaseInfo::ExecuteLogWar(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteLogWar!---", pack);
        return 0;
}

int GWBaseInfo::ExecuteLogErr(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteLogErr!---", pack);
        return 0;
}

int GWBaseInfo::ExecuteEmailSrvSetting(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteEmailSrvSetting!---", pack);

        SendOk();

        return 0;
}

int GWBaseInfo::ExecuteEmailSrvLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteEmailSrvLookup!---", pack);

        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteEmailConfig(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteEmailConfig!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteEmailLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteEmailLookup!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteNetInfoLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNetInfoLookup!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteLogErrLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteErrLogLookup!---", pack);
        return 0;
}

int GWBaseInfo::ExecuteLogWarLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteWarLogLookup!---", pack);
        return 0;
}

int GWBaseInfo::ExecuteNFSSetting(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNFSSetting!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteNFSStart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNFSStart!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteNFSStop(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNFSStop!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteNFSLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteNFSLookup!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteMultiPathLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMultiPathLookup!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteMultiPathStart(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMultiPathStart!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteMultiPathStop(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteMultiPathStop!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteSSDCacheCreate(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSSDCacheCreate!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteSSDCacheDestroy(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSSDCacheDestroy!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteSSDCacheLoad(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSSDCacheLoad!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteSSDCacheUnload(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSSDCacheUnload!---", pack);
        SendOk();
        return 0;
}

int GWBaseInfo::ExecuteSSDCacheLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteSSDCacheLookup!---", pack);
        SendOk();
        return 0;
}

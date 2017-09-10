#include <stdio.h>
#include <boost/bind.hpp>

#include "GatewayBG/Socket/Pack.h"
#include "Utils/Log/Log.h"
#include "GatewayBG/Socket/GateWayMgr.h"

#include "GatewayBG/Protocol/LogMgr.h"
#include "GatewayBG/Protocol/StorageGW.h"
#include "GatewayBG/Protocol/SystemTool.h"
#include "GatewayBG/Protocol/GWBaseInfo.h"
#include "GatewayBG/Protocol/DDFSManager.h"
#include "GatewayBG/Protocol/GWStatusInfo.h"

GateWayMgr::GateWayMgr(int port):m_Link(port), m_ThreadPool(10, 10)
{
        m_ListenCnt = 10;
        m_DDFSMgr   = NULL;
}

GateWayMgr::~GateWayMgr()
{
        if ( m_DDFSMgr != NULL ) {
                delete m_DDFSMgr;
                m_DDFSMgr = NULL;
        }
        if (m_BaseOpr != NULL) {
                delete m_BaseOpr;
                m_BaseOpr = NULL;
        }
}

int GateWayMgr::Init()
{
        int ret = 0;
        m_DDFSMgr = new DDFSMgr();
        ret = m_DDFSMgr->Init();
        if (ret < 0) {
                LOG_ERROR("m_DDFSMgr Init Error! ret=" << ret);
        }
        m_BaseOpr = new BaseOpr();

        /* 初始化服务端 */
        ret = m_Link.InitServer();
        if (ret < 0) {
                LOG_ERROR("InitServer Error!");
        }
        return ret;
}

int GateWayMgr::Listen()
{
        m_Link.Listen(m_ListenCnt);
        while (true) {
                int fd = m_Link.Accept(); 
		//LOG_INFO("accept success, fd=" << fd);	
                if (fd < 0) {
                        LOG_ERROR("m_Link->Accept error! fd=" << fd);
                        break;
                }
                if (fd == 0) {
                        continue;
                }
                m_ThreadPool.Schedule(boost::bind(&GateWayMgr::CmdParse, this, fd));
        }

        return 0;
}

int GateWayMgr::CmdParse(int fd)
{
        PackMgr* pmgr = new PackMgr(fd);
        Pack pack;
        if (!pmgr->Recv(pack)) {
                close(fd);
                m_Link.FDClear(fd);
                delete pmgr;
                return 0;
        }

        Protocol* pro = NULL;

        switch(pack.PkHead.Type & ASSISTANT_FIRST) {
        case NETINFO_MAIN: //网关基础信息配置
                pro = new GWBaseInfo(pmgr, m_DDFSMgr, m_BaseOpr);
                //LOG_INFO("NETINFO_MAIN");
                break;
        case GATEWAY_MAIN: //网关状态查看
                pro = new GWStatusInfo(pmgr, m_DDFSMgr, m_BaseOpr);
                //LOG_INFO("GATEWAY_MAIN");
                break;
        case SYSTEM_MAIN: //日志管理
                pro = new LogMgr(pmgr, m_DDFSMgr, m_BaseOpr);
                //LOG_INFO("SYSTEM_MAIN");
                break;
        case DEV_MAIN: //存储网关管理
                pro = new StorageGW(pmgr, m_DDFSMgr, m_BaseOpr);
                //LOG_INFO("DEV_MAIN");
                break;
        case MP_MAIN: //文件系统管理
                pro = new DDFSManager(pmgr, m_DDFSMgr, m_BaseOpr);
                //LOG_INFO("MP_MAIN");
                break;
        case GW_MAIN: //系统工具
                pro = new SystemTool(pmgr, m_DDFSMgr, m_BaseOpr);
                //LOG_INFO("GW_MAIN");
                break;
        default:
                delete pro;
                delete pmgr;
		if (close(fd) < 0) {
			LOG_ERROR("close fd error");
		}
                return 0;
        }
        pro->Execute(pack);
        pro->RecvEnd();
        if (close(fd) < 0) {
                LOG_ERROR("close fd error, fd=" << fd); 
                return -1;
        }
        delete pro;
        delete pmgr;

        return 0;
}

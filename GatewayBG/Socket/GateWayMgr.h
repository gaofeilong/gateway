#ifndef _GATE_WAY_MGR_H_
#define _GATE_WAY_MGR_H_

#include <stdint.h>

#include "GatewayBG/Socket/Link.h"
#include "GatewayBG/Socket/Pack.h"
#include "GatewayBG/Socket/PackMgr.h"
#include "GatewayBG/Operation/DDFSMgr.h"
#include "GatewayBG/Protocol/Protocol.h"
#include "Utils/ThreadPoolEx/ThreadPool.h"

class GateWayMgr
{
public:
        GateWayMgr(int port);
        ~GateWayMgr();

public:
        int Init();
        /**
         * @note 把接受到的包分发出去
         */
        int CmdParse(int fd);

        /**
         * @note 监听客户端发送来的数据包
         */
        int Listen();
        
        /**
         * @note 设置监听队列大小
         */
        void SetListenCnt(int cnt);

private:
        Link       m_Link;
        DDFSMgr*   m_DDFSMgr;
        BaseOpr*   m_BaseOpr;
        int        m_ListenCnt;
        ThreadPool m_ThreadPool;
};

#endif


#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <boost/thread/mutex.hpp>

#include "GatewayBG/Socket/Pack.h"
#include "GatewayBG/Socket/PackMgr.h"
#include "GatewayBG/Operation/DDFSMgr.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "Archive/DataArchive/Archive.h"

class Protocol
{
public:
        Protocol(DDFSMgr* ddfsMgr, BaseOpr* baseOpr)
        {
                m_DDFSMgr = ddfsMgr;
                m_BaseOpr = baseOpr;
        }
        virtual ~Protocol(){}
        virtual int Execute(const Pack& pack) = 0;

        void DisplayExecuteInfo(const string& info, const Pack& pack);
        bool SendErr(const string& errMsg);
        bool SendOk();
        void RecvEnd();
protected:
        BaseOpr*     m_BaseOpr;
        PackMgr*     m_PackMgr;
        DDFSMgr*     m_DDFSMgr;
        boost::mutex m_Mutex;
};

#endif //_PROTOCOL_H_

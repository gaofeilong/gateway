#include <stdio.h>
#include "Utils/Log/Log.h"
#include "GatewayBG/Protocol/Protocol.h"
#include "GatewayBG/Socket/Pack.h"
#include "Utils/CommonOpr/TimeOpr.h"

bool Protocol::SendOk()
{
        Pack ok;
        ok.PkHead.Type = CMD_OK;
        return m_PackMgr->Send(ok)? true: false;
}

bool Protocol::SendErr(const string& errMsg)
{
        Pack err;
        err.PkHead.Type = CMD_ERR;
        err.JsonValue["info"] = errMsg;
        return m_PackMgr->Send(err)? true: false;
}

void Protocol::DisplayExecuteInfo(const string& info, const Pack& pack)
{
        string curTime;
        TimeOpr tOpr;
        tOpr.GetDetailTime(curTime);
        if (pack.JsonValue.empty()) {
                printf("%-30s%30s\n", info.c_str(), curTime.c_str());
        } else {
                printf("%-30s%30s\n%s\n", info.c_str(), curTime.c_str(), pack.JsonValue.toStyledString().c_str());
        }
}

void Protocol::RecvEnd()
{
        Pack end;
        if (!m_PackMgr->Recv(end)) {
                LOG_ERROR("Recv End Error!");
                printf("Recv End Error\n\n"); 
        } else {
                printf("Recv End OK\n\n"); 
        }
}

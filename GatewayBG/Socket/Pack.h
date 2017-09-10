#ifndef _PACK_H_
#define _PACK_H_

#include "Utils/Json/include/json.h"
#include "GatewayBG/Protocol/CmdType.h"

const int MAGIC_NUM = 363636;

#pragma pack(1)
typedef struct _PackHead
{
        int Type;
        int PackSize;
        int MagicNum;
}PackHead;
#pragma pack()


typedef struct _Pack
{
        PackHead    PkHead;
        Json::Value JsonValue;
        _Pack& operator=(const _Pack& pack)
        {
                if(this != &pack) {
                        PkHead.Type     = pack.PkHead.Type;
                        PkHead.PackSize = pack.PkHead.PackSize;
                        JsonValue       = pack.JsonValue;
                }
                return *this;
        }
}Pack;

#endif


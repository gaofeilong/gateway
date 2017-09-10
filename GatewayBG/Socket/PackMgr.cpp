#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "GatewayBG/Socket/PackMgr.h"
#include "Utils/Log/Log.h"
#include "Utils/Json/include/json.h"

using namespace std;

PackMgr::PackMgr(int fd)
{
        m_HeadSize = sizeof(PackHead);
        m_CurFD    = fd;
}

PackMgr::~PackMgr()
{
}

bool PackMgr::Recv(Pack& pack)
{
        bool bret = true;
        int ret = 0;

        PackHead packHead;
        int dataBufSize = 0;
        int readed = 0;

        //读包头
        while (true) {
                //取得包类型和包体长度
                ret = Read(m_CurFD, (&packHead)+readed, m_HeadSize, Timeout);
                if (ret <= 0) {
                        bret = false;
                        break;
                }
                readed += ret;

                if (readed == m_HeadSize) {
                        pack.PkHead.Type     = packHead.Type;
                        dataBufSize          = packHead.PackSize;
                        pack.PkHead.PackSize = dataBufSize;
                        pack.PkHead.MagicNum = packHead.MagicNum;
                        break;
                }
        }

        if (pack.PkHead.MagicNum != MAGIC_NUM) {
                return false;
        }

        if (dataBufSize == 0) {
                return bret;
        }

        //读包体
        readed = 0;
        while (true) {
                //取得包体内容
                char dataBuffer[dataBufSize+1];
                ret = Read(m_CurFD, (dataBuffer+readed), dataBufSize, Timeout);
                if (ret <=0) {
                        LOG_ERROR("body Error! ret=" << ret);
                        bret = false;
                        break;
                }
                readed += ret;

                if (readed == dataBufSize) {
                        dataBuffer[dataBufSize] = '\0';
                        
                        Json::Value value;
                        Json::Reader reader;

                        if (reader.parse(dataBuffer, value)) {
                                pack.JsonValue = value;
                        }
                        break;
                }
        }
        
        if (bret) {
                m_CurPack = pack;
        }

        return bret;
}

bool PackMgr::Send(const Pack& pack)
{
        int ret = 0;
        int valueSize = 0;
        int dataLen = 0;
        int dataType = 0;

        std::string strValue; 
        if(!pack.JsonValue.empty()){
                strValue  = pack.JsonValue.toStyledString();
                valueSize = strValue.size();

                //数据长度
                dataLen = valueSize;
        } else {
                dataLen = 0;
        }

        //包类型
        dataType = pack.PkHead.Type;

        //幻数
        int magicNum = MAGIC_NUM;

        PackHead ph;
        ph.Type     = dataType;
        ph.PackSize = dataLen;
        ph.MagicNum = magicNum;

        int bufSize = m_HeadSize + valueSize;

        char dataBuf[bufSize];

        memcpy(dataBuf, &ph, m_HeadSize);

        if (valueSize != 0) {
                memcpy(dataBuf+m_HeadSize, strValue.c_str(), valueSize);
        }

        //发送包内容
        ret = send(m_CurFD, dataBuf, bufSize, MSG_WAITALL);
        if (ret < 0) {
                LOG_ERROR("send error! ret=" << ret);
                return false;
        }

        return true;
}

int PackMgr::Read(int fd, void* buf, int size, unsigned int timeout)
{
        int            nfds; 
        fd_set         readfds;
        struct timeval tv;

        tv.tv_sec  = timeout; 
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        nfds = select(fd+1, &readfds, NULL, NULL, &tv);
        if (nfds <= 0) {
                return(-1);
        }

        return read(fd, buf, size);
}

int PackMgr::GetCurFD()
{
        return m_CurFD;
}

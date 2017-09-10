#ifndef _PACK_MGR_H_
#define _PACK_MGR_H_

#include "GatewayBG/Socket/Pack.h"

const int Timeout = 60 * 10;

class PackMgr
{
public:
        PackMgr(int fd);
        ~PackMgr();

public:
        /**
         * @note   接受客户端发送来的数据包，存放到pack中
         * @return ture  接受成功
         *         false 接受失败
         */
        bool Recv(Pack& pack);

        /**
         * @note   发送数据包到客户端
         * @return ture  发送成功
         *         false 发送失败
         */
        bool Send(const Pack& pack);
        int GetCurFD();

private:
        /**
         * @note 读取指定文件描述符，读取size大小的数据，存放到buf中，等待timeout时间
         */
        int Read(int fd, void* buf, int size, unsigned int timeout);

private:
        int  m_CurFD;           //当前操作的文件描述符
        int  m_HeadSize;        //包头的大小
        int  m_DataTypeLen;     //包类型的长度
        int  m_DataSizeLen;     //包中数据的长度
        Pack m_CurPack;         //当前操作的包
};

#endif


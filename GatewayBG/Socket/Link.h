#ifndef _LINK_H_
#define _LINK_H_

#include <sys/socket.h>

class Link
{
public:
        Link(int port);
        ~Link();

public:
        /**
         * @note  激活服务端，让其处于能够监听客户端请求的状态
         * @param cnt 设置监听队列的大小，队列满了将拒绝新的链接请求
         * @return =0 成功
         *         <0 失败
         */
        int Listen(int cnt);

        /**
         * @note   接受客户端的请求
         * @return =0 继续监听
         *         >0 可以读的文件描述符
         *         <0 失败
         */
        int Accept();

        /**
         * @note 清除文件句柄fd于fdset的联系
         */
        int FDClear(int fd);

        /**
         * @note 初始化服务端并绑定
         */
        int InitServer();

private:
        int m_FD;      //当前服务端socket，文件描述符号
        int m_Port;    //服务端端口号
        int m_Nfds;    //改进程可以打开的最大文件数

        fd_set m_Rfds; // read file descriptor set
        fd_set m_Afds; // active file descriptor set
};

#endif


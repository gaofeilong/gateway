#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "GatewayBG/Socket/Link.h"
#include "GatewayBG/Socket/Pack.h"
#include "Utils/Log/Log.h"
#include "GatewayBG/Socket/PackMgr.h"

Link::Link(int port)
{
        m_FD = 0;
        m_Port = port;

        FD_ZERO(&m_Afds);
        FD_SET(m_FD, &m_Afds);

        m_Nfds = 1024;
}

Link::~Link()
{
        close(m_FD);
}

int Link::InitServer()
{
        struct sockaddr_in my_addr;

        my_addr.sin_family=AF_INET;
        my_addr.sin_port=htons(m_Port);
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");//
        bzero(&(my_addr.sin_zero),8);
        struct sockaddr *addr = (sockaddr *)&my_addr;

        //创建socket
        m_FD = socket(addr->sa_family, SOCK_STREAM, 0);
        if(m_FD < 0) {
                LOG_ERROR("socket error! errinfo: " << strerror(errno));
                return -1;
        }
        printf("socket success!\n");

        int reuse = 1;
        int ret = 0;
        ret = setsockopt(m_FD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
        if(ret < 0){
                close(m_FD);
                LOG_ERROR("setsockopt error! errinfo: " << strerror(errno));
                return -2;
        }
        printf("setscokopt success!\n");

        ret = fcntl(m_FD, F_SETFD, 1);
        if (ret < 0) {
                close(m_FD);
                LOG_ERROR("fcntl error! errinfo: " << strerror(errno));
                return -3;
        }
        printf("fcntl success!\n");
        
        //绑定socket
        ret = bind(m_FD, addr, sizeof(sockaddr_in));
        if(ret < 0) {
                close(m_FD);
                LOG_ERROR("bind error! errinfo: " << strerror(errno));
                return -4;
        }
        printf("bind success!\n");

        return 0;
}

int Link::Listen(int cnt)
{
        int ret = 0;
        ret = listen(m_FD, cnt);
        if(ret < 0) {
                close(m_FD);
                LOG_ERROR("listen error! ret=" << ret);
                return -3;
        }
        printf("listen success!\n");

        return ret;
}

int Link::Accept()
{
        /*
        memcpy(&m_Rfds, &m_Afds, sizeof(m_Rfds));
        int numready = select(m_Nfds, &m_Rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

        if (numready == -1) {
                if (errno == EINTR){
                        LOG_ERROR("select error, error=" << EINTR);
                        return 0; 
                }  
                return -1;
        }

        if(FD_ISSET(m_FD, &m_Rfds)){
                struct sockaddr peer_addr;
                socklen_t peer_addr_len = sizeof(sockaddr);
                int ssock = accept(m_FD, &peer_addr, &peer_addr_len);
                FD_SET(ssock, &m_Afds);
        }

        for(int fd=0; fd<m_Nfds; fd++) {
                if(fd == m_FD) {
                        continue;
                }

                if(FD_ISSET(fd, &m_Rfds)){ 
                        return fd;
                }
        }
        */
        struct sockaddr_in my_addr;
        socklen_t addrLen;
        bzero(&(my_addr.sin_zero),8);

        int ssock = accept(m_FD, (struct sockaddr*)&my_addr, &addrLen);
        return ssock;
}

int Link::FDClear(int fd)
{
        FD_CLR(fd, &m_Afds);
        return 0;
}

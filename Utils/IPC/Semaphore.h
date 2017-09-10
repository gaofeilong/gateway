#ifndef _IPC_SEMAPHORE_H_
#define _IPC_SEMAPHORE_H_

#define IPC_SUCCESS      0  //成功
#define IPC_TIMEOUT     -1  //超时
#define IPC_BUSY        -2  //忙，已被占用
#define IPC_ERROR       -3  //错误
#define IPC_INVAL       -4  //非法参数

#define _LINUX 

#ifdef _LINUX

#include <semaphore.h>
#include <errno.h>

#else

#include <windows.h>

#endif

class Semaphore
{
public:

        Semaphore();
        Semaphore(unsigned int initialCount);
        ~Semaphore(void);

public:

        int Wait();
        int TryWait();
        int TimedWait(int millsec);

        int Signal();
        int GetValue();

        int Init(unsigned int initialCount);

private:
        void Destroy();
        int CheckReturn(int nRet);

private:

#ifdef _LINUX
        sem_t m_Sem;
#else
        HANDLE m_Sem;
#endif
};

#endif


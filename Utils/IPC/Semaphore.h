#ifndef _IPC_SEMAPHORE_H_
#define _IPC_SEMAPHORE_H_

#define IPC_SUCCESS      0  //�ɹ�
#define IPC_TIMEOUT     -1  //��ʱ
#define IPC_BUSY        -2  //æ���ѱ�ռ��
#define IPC_ERROR       -3  //����
#define IPC_INVAL       -4  //�Ƿ�����

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


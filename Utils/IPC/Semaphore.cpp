#include "Semaphore.h"
#include <cassert>
#include <ctime>
//#include "Util/Log/Log.h"

Semaphore::Semaphore(unsigned int initialCount)
{
        Init(initialCount);
}

Semaphore::Semaphore()
{
#ifdef _LINUX
#else
        m_Sem = NULL;
#endif
}

Semaphore::~Semaphore(void)
{
        Destroy();
}

#ifdef _LINUX

int Semaphore::Wait()
{
        while (sem_wait(&m_Sem) != 0) {
                int sval = 0;
                sem_getvalue(&m_Sem, &sval);
                //LOG_INFO("sem_wait interrupted, eno="<<errno<<",sval="<<sval);
        }
        return IPC_SUCCESS;
}

int Semaphore::TryWait()
{
        int nRet = sem_trywait(&m_Sem);
        return CheckReturn(nRet);
}

int Semaphore::TimedWait( int millsec )
{
        struct timespec abstime = {
                time(0) + millsec/1000,
                (millsec % 1000) * 1000000
        };
        int nRet = 0;
        for ( ;; ) {
                nRet = sem_timedwait(&m_Sem,&abstime);
                if ( nRet == IPC_SUCCESS || nRet == IPC_TIMEOUT ) {
                        break;
                }
                int sval = 0;
                sem_getvalue(&m_Sem, &sval);
                //LOG_INFO("sem_timedwait interrupted, eno="<<errno<<",sval="<<sval);
        }

        return CheckReturn(nRet);
}

int Semaphore::Signal()
{
        int nRet = sem_post(&m_Sem);
        return CheckReturn(nRet);
}

int Semaphore::GetValue()
{
        int sval = 0;
        int nRet = sem_getvalue(&m_Sem, &sval);
        if ( nRet != 0 ) {
                return -1;
        }
        return sval;
}

int Semaphore::Init( unsigned int initialCount )
{
        int nRet = sem_init(&m_Sem, 0, initialCount);
        return CheckReturn(nRet);
}

void Semaphore::Destroy()
{
        sem_destroy(&m_Sem);
}

#else

int Semaphore::Wait()
{
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_Sem, INFINITE)) {
                return IPC_SUCCESS;
        }
        return IPC_ERROR;
}

int Semaphore::TryWait()
{
        int nRet = WaitForSingleObject(m_Sem, 0);
        if (WAIT_OBJECT_0 == nRet) {
                return IPC_SUCCESS;
        } else if (WAIT_TIMEOUT == nRet) {
                return IPC_BUSY;
        } else {
                return IPC_ERROR;
        }
}

int Semaphore::TimedWait( int millsec )
{
        assert(millsec > 0);
        int nRet = WaitForSingleObject(m_Sem, millsec);
        if (WAIT_OBJECT_0 == nRet) {
                return IPC_SUCCESS;
        } else if (WAIT_TIMEOUT == nRet) {
                return IPC_TIMEOUT;
        } else {
                return IPC_ERROR;
        }
}

int Semaphore::Signal()
{
        long value;
        if (0 != ReleaseSemaphore(m_Sem, 1, &value)) {
                return value;
        } else {
                return IPC_ERROR;
        }
}

int Semaphore::Init( unsigned int initialCount )
{
        m_Sem = CreateSemaphore(NULL, initialCount, 1<<(sizeof(int) * 8 -2), NULL);
        if (NULL != m_Sem) {
                return IPC_SUCCESS;
        }
        return IPC_ERROR;
}

void Semaphore::Destroy()
{
        if (m_Sem != NULL) {
                CloseHandle(m_Sem);
                m_Sem = NULL;
        }
}

#endif

inline int Semaphore::CheckReturn( int nRet )
{
#ifdef _LINUX
        if (0 == nRet) {
                return IPC_SUCCESS;
        } else if (EINVAL == errno) {
                return IPC_INVAL;
        } else if (ETIMEDOUT == nRet) {
                return IPC_TIMEOUT;
        } else {
                return IPC_ERROR;
        }
#else
        return nRet;
#endif
}

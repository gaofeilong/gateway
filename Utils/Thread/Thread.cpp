#include "Thread.h"
#include "Utils/Log/Log.h"

Thread::Thread(pf func, void *args)
{
        m_Func   = func;
        m_Args   = args;
        m_Status = THREAD_STATUS_NEW;

        pthread_attr_init(&m_Attr);
}

Thread::~Thread()
{
        pthread_attr_destroy(&m_Attr);
}

int Thread::Start()
{
        int ret = pthread_create(&m_Tid, &m_Attr, Run, this);
        if(ret != 0) {
                LOG_INFO("pthread_create error! errorInfo= " << strerror(errno));
        }
        return ret;
}

void *Thread::Run(void *obj)
{
        Thread *thread = static_cast<Thread *>(obj);
        thread->RunFun();
        return NULL;
}

void *Thread::RunFun()
{
        m_Status = THREAD_STATUS_RUNNING;
        (*m_Func)(m_Args); 
        m_Status = THREAD_STATUS_EXIT;

        pthread_exit(NULL);
}


int Thread::Join()
{
        if (m_Tid > 0) {
                void *tret;
                pthread_join(m_Tid, &tret);
                return *(int *)tret;
        }
        return -1;
}

int Thread::Join(unsigned long ms)
{
        if (m_Tid == 0) {
                return -1;
        }

        if (ms == 0) {
                return Join();
        } else {
                unsigned long k = 0;

                while (m_Status != THREAD_STATUS_EXIT && k <= ms) {
                        usleep(100);
                        k++;
                } if (m_Status == THREAD_STATUS_EXIT) {
                      return 0;
                } else if (m_Status == THREAD_STATUS_RUNNING) {
                      return 1;
                } else {
                      return -1;
                }
        }
}

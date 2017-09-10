
#ifndef _BOUNDED_THREAD_POOL_H
#define _BOUNDED_THREAD_POOL_H

#include <boost/bind.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "Utils/ThreadPoolEx/boost_ex/threadpool.hpp"
#include "Utils/IPC/Semaphore.h"

class ThreadPool
{
        typedef boost::function<int()>  IntCallback;
public:
        ThreadPool(int threadCount, int maxTaskCount)
          : m_TP(threadCount)
          , m_TaskCount(maxTaskCount)
        {
        }

        ~ThreadPool()
        {
                Wait();
        }

        void Schedule(IntCallback cb);
        void Wait();
       
private:
        void IntWrap(IntCallback cb);

private:
        boost::threadpool::pool m_TP;
//        boost::interprocess::interprocess_semaphore  m_TaskCount;
        Semaphore m_TaskCount;
};
#endif


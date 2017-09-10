#include "Utils/ThreadPoolEx/ThreadPool.h"

void ThreadPool::Schedule(IntCallback cb)
{
        m_TaskCount.Wait();
        m_TP.schedule(boost::bind(&ThreadPool::IntWrap, this, cb));
}

void ThreadPool::IntWrap(IntCallback cb)
{
        cb();
        m_TaskCount.Signal();
}

void ThreadPool::Wait()
{
        m_TP.wait();
}


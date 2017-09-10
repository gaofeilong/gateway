#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

typedef void*  (*pf) (void *args);

class Thread
{
public:
        Thread(pf func, void *args);
        ~Thread();

        /*
         * @func 线程启动调用的函数
         * @args 函数需要的参数
         * @return : 0 正常结束 -1 错误结束
         */
        int Start();

        int Join();

        /*
         * @等待线程结束，超时时间ms毫秒
         * @return: -1:线程不存在 0 线程运行结束 1线程正在运行
         */
        int Join(unsigned long ms);

private:
        void *RunFun();
        static void *Run(void *obj);

        pthread_attr_t  m_Attr;
        pthread_t       m_Tid;
        pf              m_Func;

        void*           m_Args;
        int             m_Status;

        //线程的状态－新建
        static const int THREAD_STATUS_NEW = 0;
        //线程的状态－正在运行
        static const int THREAD_STATUS_RUNNING = 1;
        //线程的状态－运行结束
        static const int THREAD_STATUS_EXIT = -1;
};
#endif

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "Utils/Log/Log.h"

std::auto_ptr<Log> Log::m_pSelf;
pthread_mutex_t Log::m_Mutex = PTHREAD_MUTEX_INITIALIZER; 

Log* Log::GetInstance()
{
        if ( NULL == m_pSelf.get() ) {
                LOG_LOCK;
                m_pSelf.reset(new Log());
                LOG_UNLOCK;
        }
        return m_pSelf.get();
}

Log::Log()
{
        char fpath[512]={0};
        char oprpath[512]={0};
        sprintf(fpath, "%s/%s", LOG_PATH, LOG_NAME);
        sprintf(oprpath, "%s/%s", LOG_PATH, LOG_OPR_NAME);
        m_Stream.open(fpath, std::ios::app);
        m_OprStream.open(oprpath, std::ios::app);
}

Log::~Log()
{
        if (m_Stream) {
                m_Stream.close();
        }
        if (m_OprStream) {
                m_OprStream.close();
        }
}

std::ofstream& Log::ErrorStream()
{
        time_t ct = time(0);
        char temp[50];
        strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S ", localtime(&ct));
        m_Stream << std::setw(7) << "[ERROR] ";
        m_Stream << temp;
        return m_Stream;
}

std::ofstream& Log::InfoStream()
{
        time_t ct = time(0);
        char temp[50];
        strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S ", localtime(&ct));
        m_Stream << std::setw(7) << "[INFO] ";
        m_Stream << temp;
        return m_Stream;
}

std::ofstream& Log::OprStream()
{
        time_t ct = time(0);
        char temp[50];
        strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S ", localtime(&ct));
        m_OprStream << std::setw(7) << "[OPR] ";
        m_OprStream << temp;
        return m_OprStream;
}

#ifndef _LOG_H_
#define _LOG_H_

#include <pthread.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <memory>

#define MAKESUFFIX " - @ "<<__FILE__<<"("<<__LINE__<<")"

#define LOG_NAME "gateway.log"
#define LOG_OPR_NAME "gwopr.log"
#define LOG_PATH "/var/log/"

#define LOG_LOCK   pthread_mutex_lock(&m_Mutex);
#define LOG_UNLOCK pthread_mutex_unlock(&m_Mutex);

#define LOG_ERROR(stream) {Log *log = Log::GetInstance(); log->ErrorStream() << stream << MAKESUFFIX << std::endl;}
#define LOG_INFO(stream) {Log *log = Log::GetInstance(); log->InfoStream()  << stream << MAKESUFFIX << std::endl;}
#define LOG_OPR(stream) {Log *log = Log::GetInstance(); log->OprStream()  << stream << MAKESUFFIX << std::endl;}

class Log
{
public:
        ~Log();
        static  Log * GetInstance();
        
        std::ofstream& ErrorStream();
        std::ofstream& InfoStream();
        std::ofstream& OprStream();
private:
        Log();

private:
        std::ofstream m_Stream;
        std::ofstream m_OprStream;
        static pthread_mutex_t m_Mutex;
        static std::auto_ptr<Log> m_pSelf;
};

#endif


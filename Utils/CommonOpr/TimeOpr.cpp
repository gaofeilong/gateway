#include <time.h>
#include <string.h>
#include <sys/time.h>

#include "Archive/DataArchive/ArvLogMgr.h"
#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/TimeOpr.h"
#include "Utils/CommonOpr/DirFileOpr.h"

const int BufLen = 32;

TimeOpr::TimeOpr()
{

}

TimeOpr::~TimeOpr()
{

}

void TimeOpr::ParseTime(const string& settingTime, int& hour, int& min)
{
        sscanf(settingTime.c_str(), "%d:%d", &hour, &min);
}

bool TimeOpr::IsRunTimeOver(int64_t begTime, int runTime)
{
        bool bret = false;
        int64_t runSec = runTime * 3600;
        int64_t endSec = begTime + runSec;
        int64_t curSec = time(NULL);

        if (curSec >= endSec) {
                bret = true;
        }
        return bret;
}

void TimeOpr::GetCurDay(int& year, int& mon, int& day)
{
        string date;
        GetCurMicroSec(date);
        sscanf(date.c_str(), "%d_%d_%d", &year, &mon, &day);
}

void TimeOpr::GetCurDay(string& date)
{
        //YYYY_MM_DD
        const int len = strlen("yyyy_mm_dd");
        GetCurMicroSec(date);
        date = date.substr(0, len);           
}

void TimeOpr::GetCurSec(string& sec)
{
        //YYYY_MM_DD_HH_MM_SS
        GetCurMicroSec(sec);
        sec = sec.substr(0, sec.find_last_of('_'));     
}

void TimeOpr::GetCurMicroSec(string& mSec)
{
        struct timeval timer;
        struct tm *tblock;
        char buffer[BufLen];

        memset(buffer, 0, BufLen);
        timerclear(&timer);

        //YYYY_MM_DD_HH_MM_SS_MS
        gettimeofday(&timer, NULL);
        tblock = localtime(&timer.tv_sec);
        strftime(buffer, BufLen, "%Y_%m_%d_%H_%M_%S", tblock);
        sprintf(buffer, "%s_%ld", buffer, timer.tv_usec);       
        mSec = buffer;
}

void TimeOpr::GetDetailTime(string& time)
{

        struct timeval timer;
        struct tm *tblock;
        char buffer[BufLen];

        memset(buffer, 0, BufLen);
        timerclear(&timer);

        //YYYY_MM_DD_HH_MM_SS_MS
        gettimeofday(&timer, NULL);
        tblock = localtime(&timer.tv_sec);
        strftime(buffer, BufLen, "%Y-%m-%d %H:%M:%S", tblock);
        time = buffer;
}

bool TimeOpr::IsDay()
{
        bool bret = false;
        time_t timer = time(NULL);
        struct tm *tblock;
        tblock = localtime(&timer);    
        if (tblock->tm_hour >=8 && tblock->tm_hour <= 20) {
                bret = true;  
        }
        return bret;
}

int TimeOpr::htom(const string& time)
{
        size_t pos = time.find(":");
        const string h = time.substr(0, pos);
        const string m = time.substr(pos + 1);
        int min = atoi(h.c_str())*60 + atoi(m.c_str());
        return min;
}

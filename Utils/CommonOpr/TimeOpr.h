#ifndef _TIMEOPR_H_
#define _TIMEOPR_H_

#include <stdint.h>
#include <string>

using std::string;

class TimeOpr {
public:
        TimeOpr();
        ~TimeOpr();

public:

        /**
         * @note 判断归档运行时间是否结束
         */
        bool IsRunTimeOver(int64_t begTime, int runTime);

        /**
         * @note 获取数字形式当前日期
         */
        void    GetCurDay(int& year, int& mon, int& day);

        /**
         * @note 获取当前日期, 形如yyyy_mm_dd
         */
        void    GetCurDay(string& date);

        /**
         * @note 获取当前日期精确到秒，字符串形式如yyyy_mm_dd_hh_mm_ss
         */
        void    GetCurSec(string& sec);

        /**
         * @note 获取当前日期精确到微妙, 字符串形式如yyyy_mm_dd_hh_mm_ss_usec
         */
        void    GetCurMicroSec(string& mSec);

        /**
         * @note 从字符串时间中解析出小时和分钟
         */
        void    ParseTime(const string& settingTime, int& hour, int& min);

        /**
         * @note 是否白天
         */
        bool    IsDay();

        /**
         *      get time format by YYYY-MM-DD HH:MM:SS
         */
        void GetDetailTime(string& time);

        /**
         * @ transfer hh:mm to hh*60+mm 
         */
        int htom(const string& time);
private:
};

#endif

#include <set>
#include <string>
#include <time.h>
#include <errno.h>
#include <algorithm>
#include <sys/time.h>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/DirFileOpr.h"

using std::set;
using std::string;
using namespace boost::gregorian;


string GetCurTime();
string GetCurDate();
bool   IsFileTimeOk(const string& path, int delDay, const string& curDate);


class DelFile {
public:
        DelFile(int delDay, const string& curDate)
                :m_DelDay(delDay),m_CurDate(curDate){}
        ~DelFile(){}
        
        void operator()(const string& elem)
        {
                if (IsFileTimeOk(elem, m_DelDay, m_CurDate)) {
                        printf("[%s]  delete file [%s]\n", GetCurTime().c_str(), elem.c_str());
                        int ret = remove(elem.c_str());
                        if (ret < 0) {
                                LOG_ERROR("remove error! errInfo=" << strerror(errno) 
                                                        << " ,path=" << elem);
                        }
                }
        }
private:
        int    m_DelDay;
        string m_CurDate;
};

int main(int argc, char* argv[])
{
        if (argc != 3) {
                printf("argv[0]=./DeleteBackupData\n");
                printf("argv[1]=准备删除数据的路径\n");
                printf("argv[2]=准备删除多少天前的数据\n");
                return 0;
        }

        string curDate = GetCurDate();
        int    delDay  = atoi(argv[2]);

        DirFileOpr dfo;
        /* 获取要删除的文件 */
        set<string> delFileSet;
        int ret = dfo.GetAllFileSet(argv[1], delFileSet);
        if (ret < 0) {
                LOG_ERROR("GetAllFileSet() Error! path=" << argv[1]);
                return ret;
        }

        /* 遍历并删除文件 */
        for_each(delFileSet.begin(), delFileSet.end(), DelFile(delDay, curDate));

        /* 删除没有包含文件的目录 */
        ret = dfo.DeleteEmptyDir(argv[1]);
        if (ret < 0) {
                LOG_ERROR("DeleteEmptyDir Error!");
        }
        return ret;
}

bool IsFileTimeOk(const string& path, int delDay, const string& curDate)
{
        int ret = 0;
        date now = from_undelimited_string(curDate);

        DirFileOpr dfo;

        int64_t mtime;
        ret = dfo.GetFileMTime(path, mtime);
        if (ret < 0) {
                LOG_ERROR("GetFileMTime Error!");
                return false;
        }
        
        struct tm *stm = localtime(&mtime);
        date fileMtime = date_from_tm(*stm);
        int interTime  = (now-fileMtime).days();

        return (interTime >= delDay)?true:false;
}

string GetCurDate()
{
        int BufLen = 32;
        struct tm *tblock;
        char buffer[BufLen];

        time_t t = time(NULL);

        tblock = localtime(&t);
        strftime(buffer, BufLen, "%Y%m%d", tblock);
        return buffer;
}

string GetCurTime()
{
        int BufLen = 32;
        struct tm *tblock;
        char buffer[BufLen];

        time_t t = time(NULL);

        tblock = localtime(&t);
        strftime(buffer, BufLen, "%Y-%m-%d %H:%M:%S", tblock);
        return buffer;
}

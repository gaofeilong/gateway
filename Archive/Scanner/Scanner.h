#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <map>
#include <set>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#define BOOSTPTR        boost::shared_ptr

#include "FilterType.h"
#include "FileFilter.h"

using std::map;
using std::set;
using std::string;
using std::vector;

class Scanner {
public:
        Scanner(const string& configPath);
        ~Scanner();

        int GetAllArvFile();

//public://测试
private:
        int  Init();
        string GetCurDate();
        string GetArvTaskFileName();

        bool IsFileTypeOk(const string& path);
        bool IsFileTimeOk(const string& path);

        int  DoGetArvFile(const string& path);
        int  WriteArvTaskFile(const string& path, const map<string, int64_t>& fileMap);
        void Filter(const set<string>& fileSet, map<string, int64_t>& fileMap);
        void GetDirArg(const string &tmp, const string &fix, struct _DirFilterArg &arg);
        void GetFixArg(const string &tmp, const string &fix, struct _FixFilterArg &arg);
        void GetFix(const string &fix, set<string> &fixSet);
        void Show();

private:
        int    m_ModifyTime;
        int    m_IntervalTime;
        int    m_FileCnt;

        string m_CurDate;       //当前的日期 年月日
        string m_ConfigPath;    //归档配置文件路径
        string m_ArvTaskPath;
        string m_Mountpoint;    //分区挂载路径

        map<string, string>           m_ArvPairPathMap;
        map<string, vector<string> >  m_FilterTypeMap;

        BOOSTPTR<FileFilter> m_Filter;
        struct _FilterArg m_FilterArg;
};

#endif //_SCANNER_H_

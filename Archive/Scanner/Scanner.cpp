#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "Scanner.h"
#include "FilterFactory.h"
#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Config/FilterParser.h"
#include "Config/PathPairParser.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/BaseOpr.h"

#define BUFFER_SIZE 32
#define FILTER_TYPE "filterType"
#define DEMOND_TYPE "demandType"

using namespace boost::gregorian;

Scanner::Scanner(const string& configPath)
{
        m_ConfigPath = configPath;
        m_FileCnt    = 0;
}

Scanner::~Scanner()
{
}

int Scanner::Init()
{
        int ret = 0;

        m_CurDate = GetCurDate();

        IniParser iniParser(m_ConfigPath);
        ret = iniParser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }
        string tmp;
        iniParser.GetVal("GatewayArchive", "intervalTime", tmp);
        m_IntervalTime = atoi(tmp.c_str());

        iniParser.GetVal("GatewayArchive", "modifyTime", tmp);
        m_ModifyTime = atoi(tmp.c_str());

        iniParser.GetVal("GatewayArchive", "mountpoint", m_Mountpoint);

        /* 前缀过滤条件 */
        string fixSet;
        iniParser.GetVal("Filter", "prefixType", tmp);
        iniParser.GetVal("Filter", "prefix", fixSet);
        GetFixArg(tmp, fixSet, m_FilterArg._PrefixArg);

        /* 后缀过滤条件 */
        iniParser.GetVal("Filter", "postfixType", tmp);
        iniParser.GetVal("Filter", "postfix", fixSet);
        GetFixArg(tmp, fixSet, m_FilterArg._PostfixArg);

        /* 目录过滤条件 */
        string dirSet;
        iniParser.GetVal("Filter", "dirType", tmp);
        iniParser.GetVal("Filter", "dir", dirSet);
        GetDirArg(tmp, dirSet, m_FilterArg._DirArg);

        /* 文件大小过滤条件 */
        iniParser.GetVal("Filter", "sizeType", tmp);
        if (tmp != "0") {
                string min, max;
                iniParser.GetVal("Filter", "minSize", min);
                iniParser.GetVal("Filter", "maxSize", max);
                if (tmp == "1") {               /* 指定   */
                        m_FilterArg._SizeArg._Type = _DEMAND;
                        m_FilterArg._SizeArg._Min = atol(min.c_str());
                        m_FilterArg._SizeArg._Max = atol(max.c_str());
                } else if (tmp == "2") {        /* 过滤   */
                        m_FilterArg._SizeArg._Type = _FILTER;
                        m_FilterArg._SizeArg._Min = atol(min.c_str());
                        m_FilterArg._SizeArg._Max = atol(max.c_str());
                } else {                        /* 不识别 */
                        m_FilterArg._SizeArg._Type = _NOFILTER;
                }
        } else {                                /* 不指定也不过滤 */
                m_FilterArg._SizeArg._Type = _NOFILTER;
        }
        FilterFactory filterFac;
        m_Filter = filterFac.CreateFilter(m_FilterArg);
#ifdef  _DEBUG_
        Show();
#endif  //_DEBUG_

        //归档路径对配置文件路径
        string arvPairPath; 
        iniParser.GetVal("GatewayArchive", "arvDevPath", arvPairPath);

        //归档任务存放路径
        iniParser.GetVal("GatewayArchive", "arvTaskPath", m_ArvTaskPath);
        
        PathPairParser ppp(arvPairPath);
        ret = ppp.Read(m_ArvPairPathMap);
        if (ret < 0) {
                LOG_ERROR("Read Error! path=" << arvPairPath);
        }
        return ret;
}

int Scanner::GetAllArvFile()
{
        int ret = 0;
        //初始化信息
        ret = Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

#ifdef  _DEBUG_
        printf("AllTaskDir: \n");
#endif  //_DEBUG_
        map<string, string>::iterator mssIter = m_ArvPairPathMap.begin();
        for (; mssIter != m_ArvPairPathMap.end(); ++mssIter) {
#ifdef  _DEBUG_
                printf("%s--->%s\n", mssIter->first.c_str(), mssIter->second.c_str());
#endif  //_DEBUG_
                ret = DoGetArvFile(mssIter->first);
                if (ret < 0) {
                        LOG_ERROR("DoGetArvFile Error! path=" << mssIter->first);
                }
        }
        return 0;
}

bool Scanner::IsFileTimeOk(const string& path)
{
        int ret = 0;
        date now = from_undelimited_string(m_CurDate);

        DirFileOpr dfo;

        int64_t mtime;
        ret = dfo.GetFileMTime(path, mtime);
        if (ret < 0) {
                LOG_ERROR("GetFileMTime Error!");
                return false;
        }
        
        struct tm *stm = localtime(&mtime);
        date fileMtime = date_from_tm(*stm);

        int interTime = (now-fileMtime).days();

        bool r = false;
        if ( (interTime >= m_ModifyTime) &&
             (interTime <= (m_IntervalTime + m_ModifyTime -1)) ) {
                r = true;
        }
        return r;
}

#if 0
bool Scanner::IsFileTypeOk(const string& path)
{
        vector<string>::iterator vsIter;
        map<string, vector<string> >::iterator msvsIter;

        msvsIter = m_FilterTypeMap.begin();
        if ( m_FilterTypeMap.empty() ||  (msvsIter->first != FILTER_TYPE 
                                        && msvsIter->first != DEMOND_TYPE)) {
                return true;
        }

        //过滤
        if (msvsIter->first == FILTER_TYPE) {
                //包含过滤后缀 return false;
                vsIter = msvsIter->second.begin(); 
                for (; vsIter!=msvsIter->second.end(); ++vsIter) {
                        string sub = path.substr(path.size()-(*vsIter).size());
                        if (sub == *vsIter) {
                                return false;
                        }
                }
                return true;
        }

        //指定
        vsIter = msvsIter->second.begin(); 
        for (; vsIter!=msvsIter->second.end(); ++vsIter) {
                string sub = path.substr(path.size()-(*vsIter).size());
                if (sub == *vsIter) {
                        return true;
                }
        }
        return false;
}
#endif

string Scanner::GetCurDate()
{
        int BufLen = 32;
        struct tm *tblock;
        char buffer[BufLen];

        time_t t = time(NULL);

        tblock = localtime(&t);
        strftime(buffer, BufLen, "%Y%m%d", tblock);
        return buffer;
}

string Scanner::GetArvTaskFileName()
{
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "arv_path_%d", ++m_FileCnt);

        string tpath = m_ArvTaskPath + "/" + m_CurDate + "/" + buffer;
        return tpath;
}

void Scanner::Filter(const set<string>& fileSet, map<string, int64_t>& fileMap)
{
        int ret = 0;
        DirFileOpr dfo;
        int64_t fileSize = 0;

        set<string>::iterator ssIter = fileSet.begin();
        for (; ssIter != fileSet.end(); ++ssIter) {
                if(!m_Filter->Filter(*ssIter) || !IsFileTimeOk(*ssIter)) {
#ifdef  _DEBUG_
                        printf("Filter %s bad\n", ssIter->c_str());
#endif  //_DEBUG_
                        continue;
                }

                ret = dfo.GetFileSize(*ssIter, fileSize);
                if (ret < 0) {
                        LOG_ERROR("GetFileSize Error! path=" << *ssIter);
                }
                fileMap.insert(make_pair(*ssIter, fileSize));
        }
}

int Scanner::WriteArvTaskFile(const string& path, const map<string, int64_t>& fileMap)
{
        int ret = 0;
        DirFileOpr dfo;
        string arvTaskPath = GetArvTaskFileName();

        string parPath;
        dfo.GetParPath(arvTaskPath, parPath);

#ifdef  _DEBUG_
        printf("Task File Name: %s\n", arvTaskPath.c_str());
#endif  //_DEBUG_

        if (!dfo.HasPath(parPath)) {
                ret = dfo.MakeDir(parPath);
                if (ret < 0) {
                        LOG_ERROR("MakeDir Error! path=" << parPath);
                        return ret;
                }
        }

        FILE* file = fopen(arvTaskPath.c_str(), "w+");
        if (file == NULL) {
                LOG_ERROR("fopen error! errInfo=" << strerror(errno));
                return -1;
        }

        fprintf(file, "%s\n", path.c_str());
        map<string, int64_t>::const_iterator msiIter = fileMap.begin();
        for (; msiIter != fileMap.end(); ++msiIter) {
                fprintf(file, "%s,.?%ld\n", (msiIter->first).c_str(), msiIter->second);
        }

        fclose(file);
        return 0;
}

int Scanner::DoGetArvFile(const string& path)
{
        int ret = 0;
        DirFileOpr dfo;
        BaseOpr baseOpr;
        set<string> fileSet;

        string srcPath = path;
        //获取path下所有文件
        /* 1. 扫描前,如果path为设备文件, 挂载源分区到指定路径下 */
        if (dfo.IsBlockDevice(path)) {
                srcPath = m_Mountpoint;
                if (!dfo.IsMounted(path, m_Mountpoint) ) {
                        ret = baseOpr.Mount(path, m_Mountpoint);
                        if (ret < 0) {
                                LOG_ERROR("mount error! dev=" << path);
                                return ret;
                        }
                }
        }

        //获取分区内所有文件或者是指定路径下的所有文件
        ret = dfo.GetAllFileSet(srcPath, fileSet);
        if (ret < 0) {
                LOG_ERROR("GetAllFileSet Error! srcPath=" << srcPath);
                return ret;
        }

#ifdef  _DEBUG_
        printf("Before Filter\n");
        set<string>::iterator ssIt = fileSet.begin();
        for ( ; ssIt != fileSet.end(); ++ssIt) {
                printf("\t%s\n", ssIt->c_str()); 
        }
#endif  //_DEBUG_

        //过滤并获取文件大小
        map<string, int64_t> fileMap;
        Filter(fileSet, fileMap);
#ifdef  _DEBUG_
        printf("After Filter\n");
        map<string, int64_t>::iterator msiIt = fileMap.begin();
        for ( ; msiIt != fileMap.end(); ++msiIt) {
                printf("After Filter: %s\n", msiIt->first.c_str()); 
        }
#endif  //_DEBUG_

        
        //写入文件, 第一行写归档路径对中的源
        ret = WriteArvTaskFile(path, fileMap);
        if (ret < 0) {
                LOG_ERROR("WriteArvTaskFile Error! srcPath=" << srcPath);
        }
        
        /* 2. 扫描后卸载已经挂载的分区 */
        if (dfo.IsBlockDevice(path) && dfo.IsMounted(path, m_Mountpoint)) {
                ret = baseOpr.UnMount(path);
                if (ret < 0) {
                        LOG_ERROR("mount error! dev=" << path);
                }
        }
        return ret;
}

void Scanner::GetDirArg(const string &tmp, const string &fix, struct _DirFilterArg &arg)
{
        struct _FixFilterArg fixArg;
        GetFixArg(tmp, fix, fixArg);
        arg._Type = fixArg._Type;
        arg._DirSet = fixArg._FixSet;
}

void Scanner::GetFixArg(const string &tmp, const string &fix, struct _FixFilterArg &arg)
{
        if (tmp == "1") {
                arg._Type = _DEMAND;
                GetFix(fix, arg._FixSet);
        } else if (tmp == "2") {
                arg._Type = _FILTER;
                GetFix(fix, arg._FixSet);
        } else {
                arg._Type = _NOFILTER;
        }
}

void Scanner::GetFix(const string &fix, set<string> &fixSet)
{
        char *token, *savePtr;
        char *str = new char[fix.size() + 1];

        memset(str, 0, fix.size() + 1);
        memcpy(str, fix.c_str(), fix.size());
        while (1) {
                token = strtok_r(str, ";", &savePtr);
                if (token == NULL) {
                        break;
                }
                fixSet.insert(token);
                str = NULL;
        }
}

void Scanner::Show()
{
        /* prefix filter */
        enum _FilterType type = m_FilterArg._PrefixArg._Type;
        if (type == _NOFILTER) {
                printf("no prefix filter\n"); 
        } else {
                printf("%s prefix :\n", type == _FILTER? "filter": "demand");
                set<string>::iterator it = m_FilterArg._PrefixArg._FixSet.begin();
                for ( ;it != m_FilterArg._PrefixArg._FixSet.end(); ++it) {
                        printf("\t%s\n", it->c_str()); 
                }
        }

        /* postfix filter */
        type = m_FilterArg._PostfixArg._Type;
        if (type == _NOFILTER) {
                printf("no postfix filter\n"); 
        } else {
                printf("%s postfix :\n", type == _FILTER? "filter": "demand");
                set<string>::iterator it = m_FilterArg._PostfixArg._FixSet.begin();
                for ( ;it != m_FilterArg._PostfixArg._FixSet.end(); ++it) {
                        printf("\t%s\n", it->c_str()); 
                }
        }

        /* dir filter */
        type = m_FilterArg._DirArg._Type;
        if (type == _NOFILTER) {
                printf("no dir filter\n"); 
        } else {
                printf("%s dir :\n", type == _FILTER? "filter": "demand");
                set<string>::iterator it = m_FilterArg._DirArg._DirSet.begin();
                for ( ;it != m_FilterArg._DirArg._DirSet.end(); ++it) {
                        printf("\t%s\n", it->c_str()); 
                }
        }

        /* size filter */
        type = m_FilterArg._SizeArg._Type;
        if (type == _NOFILTER) {
                printf("no size filter\n"); 
        } else {
                printf("%s size:\n", type == _FILTER? "filter": "demand");
                printf("\tmin: %ld\n", m_FilterArg._SizeArg._Min);
                printf("\tmax: %ld\n", m_FilterArg._SizeArg._Max);
        }
}

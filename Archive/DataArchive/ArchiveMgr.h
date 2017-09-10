#ifndef _ARCHIVE_MGR_H_
#define _ARCHIVE_MGR_H_

#include <map>
#include <list>
#include <vector>
#include <string>

#include "ThreadParam.h"

using std::map;
using std::list;
using std::string;
using std::vector;

class Thread;
class DDFSArchive;

class ArchiveMgr {
public:
        ~ArchiveMgr();
        ArchiveMgr(const string& configPath);

public:
        int Start();

        static void* ThreadArchive(void* args);
        static void* DevThreadArchive(void* args);

private:
        int  Init();

        int  DispatchTask();
        int  DispatchDevTask();

        int  ReadArvConfig();

        int  DispatchDevThread(const string& dev);
        void DispatchThread(const string& taskPath);

        void GetThreadParam(const string& taskPath, ThreadParam* threadParam);
        int  GetDevThreadParam(const string& dev, DevThreadParam* devThreadParam);

        int  GetAllTask();
        int  DistinguishByDev(map<string, map<string,int64_t> >& devTask);

private:
        /* 存放所有任务，按分区存放 */
        map<string, map<string, int64_t> >   m_AllDevArvTask;

        /*  存放所有任务，不按分区区别 */
        map<string, int64_t>                 m_AllArvTask;

        /* 存放目前所有归档任务的分区或所有任务路径 */
        vector<string>                       m_AllDev;

        size_t                               m_CurThreadCnt;
        size_t                               m_MaxThreadCnt;
        string                               m_ConfigPath;

        /* 是否根据分区开启线程 */
        int                                  m_ClassifyDev;
        /* 归档运行时长 */
        float                                m_ArvRunTime;                                 
        /* 程序开始时间*/
        time_t                               m_StartTime;

        /* 存放目前运行的Dev线程 */
        list<std::pair<Thread*, DevThreadParam*> >   m_CurRunDevThread;
        /* 存放目前运行的线程 */
        list<std::pair<Thread*, ThreadParam*> >      m_CurRunThread;

};

#endif

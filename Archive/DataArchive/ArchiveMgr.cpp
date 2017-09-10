#include "Archive.h"
#include "TaskMgr.h"
#include "ArchiveMgr.h"
#include "Utils/Log/Log.h"
#include "Utils/Thread/Thread.h"
#include "Config/IniParser.h"

#define  NO_DEV_TASK 1

ArchiveMgr::ArchiveMgr(const string& configPath)
{
        m_CurThreadCnt = 0;
        m_MaxThreadCnt = 0;
        m_ArvRunTime   = 0;
        m_StartTime    = time(NULL);
        m_ConfigPath   = configPath;
}

ArchiveMgr::~ArchiveMgr()
{
}

void* ArchiveMgr::DevThreadArchive(void* args)
{
        DevThreadParam *pTask = (DevThreadParam*)args;
        (pTask->ddfsArchive)->ArchiveDevTask(pTask->devTask);
        return NULL;
}

void* ArchiveMgr::ThreadArchive(void* args)
{
        ThreadParam *pTask = (ThreadParam*)args;
        (pTask->ddfsArchive)->Archive(pTask->taskPath, pTask->lineNum);
        return NULL;
}

int ArchiveMgr::Init()
{
        int ret = 0;
        ret = ReadArvConfig();
        if (ret < 0) {
                LOG_ERROR("GetMaxThreadCnt Error!");
                return ret;
        }

        m_AllDev.clear();
        m_AllArvTask.clear();
        m_AllDevArvTask.clear();

        if ( m_ClassifyDev == 1 ) {
                /* 获取总任务信息，和总的分区信息 */
                ret = DistinguishByDev(m_AllDevArvTask);
                if (ret < 0) {
                        LOG_ERROR("DistinguishByDev Error!");
                        return ret;
                }
        } else {
                ret = GetAllTask();
                if (ret < 0) {
                        LOG_ERROR("GetAllTask Error!");
                }
        }
        return ret;
}

int ArchiveMgr::Start()
{
        int ret = Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        if (m_ClassifyDev == 1 ) {
                ret = DispatchDevTask();
                if (ret < 0) {
                        LOG_ERROR("DispatchDevTask Error!");
                        return ret;
                }
        } else if (m_ClassifyDev == 0) {
                ret = DispatchTask();
                if (ret < 0) {
                        LOG_ERROR("DispatchTask Error!");
                }
        }
        return ret;
}

int ArchiveMgr::DispatchTask()
{
        int ret  = 0;
        size_t i = 0;

        while (true) {
                if (m_CurThreadCnt < m_MaxThreadCnt
                                && time(NULL) < (m_StartTime + m_ArvRunTime * 3600)
                                && m_CurThreadCnt < m_AllDev.size()
                                && i <= m_AllDev.size()-1) {
                        DispatchThread(m_AllDev[i]);
                        ++m_CurThreadCnt;
                        ++i;
                }

                if (m_CurRunThread.empty()) {
                        break;
                }

                /* 所有任务都已经开启线程 */
                if (m_CurThreadCnt == m_MaxThreadCnt || i == m_AllDev.size()) {
                        list<pair<Thread*, ThreadParam*> >::iterator lpIter;
                        /* 查看现在运行的线程是否有结束的 */
                        for (lpIter = m_CurRunThread.begin(); lpIter != m_CurRunThread.end(); ) {
                                ret = (lpIter->first)->Join(10);
                                /*  线程正常结束 */
                                if (ret == 0) {
                                        --m_CurThreadCnt;
                                        delete lpIter->first;
                                        delete lpIter->second;
                                        lpIter = m_CurRunThread.erase(lpIter);
                                } else {
                                        ++lpIter;
                                }
                        }
                }
        }
        return 0;
}

int ArchiveMgr::DispatchDevTask()
{
        int ret  = 0;
        size_t i = 0;

        while (true) {
                if (m_CurThreadCnt < m_MaxThreadCnt
                                && time(NULL) < (m_StartTime + m_ArvRunTime * 3600)
                                && m_CurThreadCnt < m_AllDev.size()
                                && i <= m_AllDev.size()-1) {
                        ret = DispatchDevThread(m_AllDev[i]);
                        if (ret != NO_DEV_TASK) {
                                ++m_CurThreadCnt;
                        }
                        ++i;
                        //if (i < m_AllDev.size()) {
                        //}
                }

                if (m_CurRunDevThread.empty()) {
                        break;
                }

                /* 所有任务都已经开启线程 */
                if (m_CurThreadCnt == m_MaxThreadCnt || i == m_AllDev.size()) {
                        list<pair<Thread*, DevThreadParam*> >::iterator lpIter;
                        /* 查看现在运行的线程是否有结束的 */
                        for (lpIter = m_CurRunDevThread.begin(); lpIter != m_CurRunDevThread.end(); ) {
                                ret = (lpIter->first)->Join(10);
                                /*  线程正常结束 */
                                if (ret == 0) {
                                        --m_CurThreadCnt;
                                        delete lpIter->first;
                                        delete lpIter->second;
                                        lpIter = m_CurRunDevThread.erase(lpIter);
                                } else {
                                        ++lpIter;
                                }
                        }
                }
        }
        return 0;
}

int ArchiveMgr::DispatchDevThread(const string& dev)
{
        int ret = 0;

        DevThreadParam * devThreadParam = new DevThreadParam;
        ret = GetDevThreadParam(dev, devThreadParam);
        if (ret != NO_DEV_TASK) {
                Thread* thread = new Thread(DevThreadArchive, (void*)devThreadParam);
                thread->Start();
                m_CurRunDevThread.push_back(make_pair(thread, devThreadParam));
        }
        return ret;
}

void ArchiveMgr::DispatchThread(const string& taskPath)
{
        ThreadParam * threadParam = new ThreadParam;
        GetThreadParam(taskPath, threadParam);
        Thread* thread = new Thread(ThreadArchive, (void*)threadParam);
        thread->Start();
        m_CurRunThread.push_back(make_pair(thread, threadParam));
}

void ArchiveMgr::GetThreadParam(const string& taskPath, ThreadParam* threadParam)
{
        map<string, int64_t>::iterator msiIter;

        msiIter = m_AllArvTask.find(taskPath);
        if (msiIter != m_AllArvTask.end()) {
                DDFSArchive* ddfsArchive = new DDFSArchive(m_ConfigPath);

                threadParam->ddfsArchive = ddfsArchive;
                threadParam->taskPath    = msiIter->first;
                threadParam->lineNum     = msiIter->second;

                m_AllArvTask.erase(msiIter);
        }
}

int ArchiveMgr::GetDevThreadParam(const string& dev, DevThreadParam* devThreadParam)
{
        map<string, int64_t>::iterator msiIter;
        map<string, map<string, int64_t> >::iterator msmIter;

        msmIter = m_AllDevArvTask.find(dev);
        if (msmIter != m_AllDevArvTask.end()) {
                /* 分区任务是否为空 */
                msiIter = msmIter->second.begin();
                if (msiIter == msmIter->second.end()) {
                        m_AllDevArvTask.erase(msmIter);
                        return NO_DEV_TASK;
                }
                DDFSArchive* ddfsArchive = new DDFSArchive(m_ConfigPath);

                devThreadParam->ddfsArchive = ddfsArchive;
                devThreadParam->devTask     = msmIter->second;
        }
        return 0;
}

int ArchiveMgr::DistinguishByDev(map<string, map<string,int64_t> >& devTask)
{
        int ret = 0;
        TaskMgr taskMgr;
        map<string, int64_t> allTask;

        ret = taskMgr.GetAllTask(m_ConfigPath, allTask);
        if (ret != 0) {
                LOG_ERROR("get task error"); 
                return ret;
        } 

        map<string, int64_t>::iterator msiIter = allTask.begin();
        map<string, map<string, int64_t> >::iterator mspIter = devTask.begin();

        /* 按分区把现有的任务排列 */
        for (; msiIter != allTask.end(); ++msiIter) {
                string volume;
                taskMgr.GetTaskVolume(msiIter->first, volume);
                mspIter = devTask.find(volume); 

                if (mspIter != devTask.end()) {
                        (mspIter->second).insert(make_pair(msiIter->first, msiIter->second));
                } else {
                        map<string, int64_t> oneDevTask;
                        oneDevTask.insert(make_pair(msiIter->first, msiIter->second));
                        devTask.insert(make_pair(volume, oneDevTask));
                        m_AllDev.push_back(volume);
                }
        }
        return 0;
}

int ArchiveMgr::GetAllTask()
{
        int ret = 0;

        TaskMgr taskMgr;
        ret = taskMgr.GetAllTask(m_ConfigPath, m_AllArvTask);
        if (ret != 0) {
                LOG_ERROR("get task error"); 
                return ret;
        } 

        map<string, int64_t>::iterator msiIter = m_AllArvTask.begin();
        for (; msiIter != m_AllArvTask.end(); ++msiIter) {
                m_AllDev.push_back(msiIter->first);
        }
        return ret;
}

int ArchiveMgr::ReadArvConfig()
{
        /* 读取配置信息，确认归档的参数 */
        IniParser iniOpr(m_ConfigPath);

        int ret;
        ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        string tMax;
        ret = iniOpr.GetVal("GatewayArchive", "arvMaxThreadCnt", tMax);
        if (ret < 0) {
                LOG_ERROR("parse arvTaskPath error! ret=" << ret);
        } 
        m_MaxThreadCnt = atoi(tMax.c_str());

        /* 归档运行时长 */
        string tmpStr;
        ret = iniOpr.GetVal("GatewayArchive", "runTime", tmpStr);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        m_ArvRunTime = atof(tmpStr.c_str());

        /* 是否根据分区开启多线程 */
        ret = iniOpr.GetVal("GatewayArchive", "classifyDev", tmpStr);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        m_ClassifyDev = atoi(tmpStr.c_str());

        return ret;
}

#include <errno.h>

#ifdef _ST_
extern "C" {
#include "Utils/STXR/stxr.h"
} 
#endif //_ST_

#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Config/PathPairParser.h"
#include "Utils/CommonOpr/TimeOpr.h"
#include "Archive/Md5Copy/Md5Copy.h"
#include "Archive/Email/LogManager.h"
#include "Archive/DataArchive/DBInfo.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "Archive/DataArchive/Archive.h"
#include "GatewayBG/Operation/DDFSMgr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

#define G      (1024L*1024L*1024L);
#define IS_ST  1

#define CAN_NOT_SET_READ_ONLY 3

DDFSArchive::DDFSArchive(const string& configPath)
{
        m_SubStr         = ",.?";               
        m_SrcPath        = "/mnt/index";        /* 挂载点路径 */
        m_SubStrLen      = m_SubStr.size();    
        m_ArvTotalSize   = 0;                  
        m_ArvErrStatus   = false;
        m_ErrProcess     = false;
        m_DBOpr          = NULL;
        m_StartTime      = time(NULL);
        m_ErrStatus      = false;
        m_ConfigPath     = configPath;
}

DDFSArchive::~DDFSArchive()
{
        if (m_DBOpr != NULL) {
                m_DBOpr->Close();
                delete m_DBOpr;
        }
}

int DDFSArchive::Init()
{
        int ret = 0;
        /* 读取归档配置文件，初始化各个域 */
        ret = ReadConfig();
        if (ret < 0) {
                LOG_ERROR("ReadConfig Error! ret=" << ret);
                LogManager logMgr;
                logMgr.InitEmailTimeConfig();
                logMgr.WriteErrorLog("/etc/scigw/GWconfig can not find!");
                return ret;
        }

        /* 获取准备挂载的路径, 不存在创建 */
        if ( !m_SrcPath.empty() && !m_PathOpr.HasPath(m_SrcPath) ) {
                ret = m_PathOpr.MakeDir(m_SrcPath.c_str());
                if (ret < 0) {
                        LOG_ERROR("MakeDir Error! ret=" << ret);
                        return ret;
                }
        }

        /* 获取当前Ip */
        BaseOpr baseOpr;
        ret = baseOpr.GetIp(m_NetworkId, m_CurIp);
        if (ret < 0){
                LOG_ERROR("GetIp Error! ret=" << ret);
                m_CurIp = "unkonw ip";
        }
        
        /* 创建数据库操作对象 */
        m_DBOpr = new DBOpr;
        ret = m_DBOpr->Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        ret = m_DBOpr->Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error! ret=" << ret);
        }
        return ret;
}

int DDFSArchive::ArchiveDevTask(map<string, int64_t>& devTask)
{
        int ret = 0;
        map<string, int64_t>::iterator tMsiIter;
        map<string, int64_t>::iterator msiIter = devTask.begin();

        for (; msiIter != devTask.end(); ) {
                tMsiIter = msiIter++;
                ret = Archive(tMsiIter->first, tMsiIter->second);
                if (ret != 0) {
                        break;
                }
                devTask.erase(tMsiIter);
        }
        return ret;
}

int DDFSArchive::Archive(const string& taskPath, int64_t lineNum)
{
        int ret = 0;
        ArvLogInfo alInfo;
        list<pair<string, int64_t> > TaskMap;

        /* 读取配置文件,以方便通过源获取目标路径, 创建数据库对象 */
        ret = Init();
        if (ret < 0) {
                LOG_ERROR("Init Error! ret=" << ret);
                return -1;
        }

        /* 归档任务开始时间 */
        TimeOpr tOpr;
        tOpr.GetCurSec(m_BeginSec);
        alInfo.startTime = m_BeginSec;

        /*********************** 获取归档任务列表(由上层控制) *******************/
        ret = ParseTaskFile(taskPath, lineNum, 
                                m_DevName, TaskMap);
        if (ret < 0) {
                LOG_ERROR("ParseTaskFile Error!");
                return -2;
        }

        if (TaskMap.empty()) {
                LOG_INFO("taskMap empty! path=" << taskPath);
                ret = RemoveTaskFile();
                if (ret < 0) {
                        LOG_ERROR("RemoveTaskFile Error!");
                }
                return 0;
        }

        BaseOpr bOpr;
        /****************区分设备和目录，如果是设备的话就mount ******/
        if ( m_PathOpr.IsBlockDevice(m_DevName) && 
                         !m_PathOpr.IsMounted(m_DevName, m_SrcPath) ) {

                ret = bOpr.Mount(m_DevName, m_SrcPath);
                if (ret < 0) {
                        LOG_ERROR("Mount Error! dev=" << m_DevName);
                        return ret;
                }
        } else {
                m_SrcPath = m_DevName;
        }
        /**************************************************************/

        m_DestPath = GetDestPathBySrc(m_DevName);

        if ( m_DestPath.empty() ) {
                if ( m_PathOpr.IsBlockDevice(m_DevName) && 
                                 m_PathOpr.IsMounted(m_DevName, m_SrcPath) ) {
                        ret = bOpr.UnMount(m_DevName);
                        if (ret < 0) {
                                LOG_ERROR("UnMount Error! dev=" << m_DevName);
                        }
                }
                LOG_ERROR("Get destination path error! path=" << m_DevName);

                TimeOpr tOpr;
                string date;
                tOpr.GetCurDay(date);
                for (size_t i = 0; i < date.length(); ++i) {
                        if (date[i] == '_') {
                                date.erase(i, 1); 
                        } 
                }
                string curFile = m_CurTaskPath.substr(m_CurTaskPath.rfind('/') + 1);
                string errPath = m_ArvErrPath + date + "/" + curFile;
                string errPPath;
                m_PathOpr.GetParPath(errPath, errPPath);
                if (!m_PathOpr.HasPath((errPPath))) {
                        m_PathOpr.MakeDir(errPPath);
                }

                ret = rename(m_CurTaskPath.c_str(), errPath.c_str());
                if (ret != 0) {
                        LOG_ERROR("Move file fail! path=" << m_CurTaskPath);
                        return -4;
                }
                return -5;
        }
        /* 设置日志路径 */
        //SetLogPath(m_DestPath);

        /* 开始归档TaskMap中的数据 */
        int retVal = DoArchive(TaskMap);
        if (retVal < 0) {
                LOG_ERROR("DoArchive Fail m_DevName=" << m_DevName);
        } else if (retVal == 1) {
                LOG_INFO("Run Time Over!");
        } else if (retVal == 2) {
                LOG_INFO("磁盘剩余空间到达指定额度");
        }

        // 把内存中的数据刷得磁盘上
        sync();
        sleep(10);

        /* 卸载 */
        if ( m_PathOpr.IsBlockDevice(m_DevName) &&
                                m_PathOpr.IsMounted(m_DevName, m_SrcPath) ) {
                bOpr.UnMount(m_DevName);
        }

        /*
        if ( retVal == 0 && m_PathOpr.IsBlockDevice(m_DevName) ) {
                ret = bOpr.UnMount(m_DevName);
                if (ret < 0) {
                        LOG_ERROR("Umount Error! m_DevName=" << m_DevName);
                }
        }
        */

        /**
         * 一个分区任务完成,记录信息并插入数据库, 
         */
        alInfo.totalSize = m_ArvTotalSize;
        alInfo.srcPath   = m_DevName;
        alInfo.destPath  = m_DestPath;
        if ( m_ArvErrStatus ) {
                alInfo.arvStatus = 1;
        } else {
                alInfo.arvStatus = 0;
        }
        m_ArvTotalSize   = 0;
        /* 结束时间 */
        string endSec;
        tOpr.GetCurSec(endSec);
        alInfo.endTime = endSec;

        if ((alInfo.totalSize > 0 || !TaskMap.empty()) && !m_ErrStatus) {
                ret = InsertArvLog(alInfo);
                if (ret < 0) {
                        LOG_ERROR("InsertArvLog Error!");
                }
                m_ErrStatus = false;
        }
        return retVal;

        /* 正常和异常情况
        if (retVal == 1 || retVal < 0) {
        }
        */
}

int DDFSArchive::DoArchive(list<pair<string, int64_t> >& TaskMap)
{
        int ret = 0;

#ifdef _ST_
        if (atoi(m_NodeType.c_str()) == IS_ST) {
                /* 获取数据库链接信息 */
                char stConfPath[ST_CONN_SIZE];
                sprintf(stConfPath, "%s", m_STConfigPath.c_str());
                ret = STGetConnInfo(stConfPath, m_ConnInfo, ST_CONN_SIZE);
                if (ret != 0) {
                        LOG_ERROR("STGetConnInfo Error! configPath=" << m_STConfigPath);
                        return ret;
                }
        }
#endif //_ST_

        /* 归档任务列表中的任务 */
        ret = DoArchiveFile(TaskMap);
        if (ret < 0) {
                LOG_ERROR("DoArchiveFile Error! ret=" << ret);
                return ret;
        } else if (ret == 1) {
                return ret; 
        } else if (ret == 2) {
                return ret;
        }

        string     sendType;
        LogManager logMgr;
        logMgr.ReadEmailFile(sendType);

        /* 每归档完成一个源，检验一次是否存在cpu和磁盘空间不足的情况 */
        /*
        if ( sendType == "1" ) {
                ChildProcessOpr cmdOpr;
                ret = cmdOpr.ExecuteCmd("/usr/sbin/run_sci_check.sh check && /usr/sbin/run_sci_check.sh send");
                if (ret < 0) {
                        LOG_ERROR("ExecuteCmd Error!");
                }
        } else {
                ChildProcessOpr cmdOpr;
                ret = cmdOpr.ExecuteCmd("/usr/sbin/run_sci_check.sh check");
                if (ret < 0) {
                        LOG_ERROR("ExecuteCmd Error!");
                }
        }
        */

        /* 移动完成的任务文件, DoArchiveFile函数退出说明一个任务文件做完. */
        ret = RemoveTaskFile();
        if (ret < 0) {
                LOG_ERROR("RemoveTaskFile Error!");
        }
        return 0;
}

int DDFSArchive::ArchiveFile(const string& destFullPath, const string& srcPath, 
                        int64_t size, string& errInfo)
{
        int ret = 0;

        if (!m_PathOpr.HasPath(srcPath)) {
                LOG_INFO("src=" << srcPath << " don't exist!");
                return 1;
        }

        ret = CopyData(srcPath, m_DestPath);
        if (ret < 0) {
                LOG_ERROR("CopyData Error! ret=" << ret);
                /* 1. 如果是目标没有空间的话，结束归档 */
                string errInfo;
                ret = m_PathOpr.CheckPath(m_DestPath, "w", errInfo);
                if (ret == CAN_NOT_WRITE) {
                        LogManager logMgr;
                        string content = "[" + m_DestPath + "] " + errInfo;
                        logMgr.InitEmailTimeConfig();
                        logMgr.WriteErrorLog(content);
                        errInfo = content;
                        return -1;
                }
                /* 2. 如果是源不能读取的话，结束归档 */
                ret = m_PathOpr.CheckPath(srcPath, "r", errInfo);
                if (ret == CAN_NOT_READ) {
                        LogManager logMgr;
                        string content = "[" + srcPath + "] " + errInfo;
                        logMgr.InitEmailTimeConfig();
                        logMgr.WriteErrorLog(content);
                        errInfo = content;
                }
                return 1;
        } else if (ret == CAN_NOT_SET_READ_ONLY) {
                LOG_ERROR("ST Cann't set read only!!"); 
                return -4;
        }

        /* 记录移动任务 */
        ret = m_LogMgr.WriteMoveTask(m_ArvMovePath, srcPath, destFullPath);
        if (ret < 0) {
                LOG_ERROR("WriteMoveTask Error! ret=" << ret);
                return -2;
        }

        /* 获取YY_MM_DD HH_MM_SS */
        string strSec;
        TimeOpr tOpr;
        tOpr.GetDetailTime(strSec);

        ArvDetailInfo adi;
        adi.srcPath  = srcPath;
        adi.destPath = destFullPath;
        adi.arvTime  = strSec;
        adi.arvSize  = size;

        ret =InsertArvDetail(adi);
        if (ret < 0) {
                LOG_ERROR("InsertArvDetail Error! ret=" << ret);
                return -3;
        }
        return 0;
}

int DDFSArchive::CopyData(const string& srcPath, const string& destPath)
{
#ifdef _ST_
        int ret = 0;
        if (atoi(m_NodeType.c_str()) == IS_ST) {
                char   tbsName[ST_CONN_SIZE];
                char   tSrcPath[ST_CONN_SIZE];
                /* 获取源路径 */
                sprintf(tSrcPath, "%s", srcPath.c_str());
                /* 获取表空间名 */
                ret = STGetTablespaceName(m_ConnInfo, tSrcPath, tbsName, ST_CONN_SIZE);
                if (ret != 0) {
                        LOG_ERROR("STGetTablespaceName Error! srcPath=" << srcPath << " ,ret=" << ret);
                        return -2;
                }
                /* 设置表空间为只读,如果设置失败尝试60次，即一个小时 */
                int cnt = 0;
                while ( cnt++ < 60 ) {
                        ret = STSetTablespaceReadOnly(m_ConnInfo, tbsName);
                        LOG_INFO("STSetTablespaceReadOnly tbsName=" << tbsName << " ret=" << ret);
                        if (ret == 0) {
                                break;
                        } else {
                                sleep(60);
                                LOG_ERROR("STSetTablespaceReadOnly Error! ret=" << ret);
                        }
                }
                if (cnt == 60) {
                        return CAN_NOT_SET_READ_ONLY;
                }
        }
#endif //_ST_

        Md5Copy m5c;
        return m5c.CopyData(m_IsCheck, srcPath, destPath);
}

int DDFSArchive::ReadConfig()
{
        int ret = 0;

        /* 读取配置信息，确认归档的参数 */
        IniParser iniOpr(m_ConfigPath);
        ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        /* 配置文件路径 */
        string configPath;
        ret = iniOpr.GetVal("GatewayArchive", "arvDevPath", configPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! configPath=" << configPath);
                return ret;
        }

        /* 归档的源＆目的路径列表 */
        PathPairParser ppp(configPath);
        ret = ppp.Read(m_CanArchivePath);
        if (ret < 0) {
                LOG_ERROR("Read Error!");
                return ret;
        }

        /* 源数据路径(如果是设备的话会挂载到这个目录) */
        ret = iniOpr.GetVal("GatewayArchive", "mountpoint", m_SrcPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        if (m_SrcPath.at(m_SrcPath.length() - 1) != '/') {
                m_SrcPath+= '/'; 
        }

        /* 插入数据库间隔 */
        string tmpStr;
        ret = iniOpr.GetVal("GatewayArchive", "intervalTime", tmpStr);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        m_IntervalRecord = atoi(tmpStr.c_str());

        /* 归档任务文件路径 */
        ret = iniOpr.GetVal("GatewayArchive", "arvTaskPath", m_ArvTaskPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        if (m_ArvTaskPath.at(m_ArvTaskPath.length() - 1) != '/') {
                m_ArvTaskPath += '/'; 
        }

        /* 归档错误记录文件路径 */
        ret = iniOpr.GetVal("GatewayArchive", "arvErrPath", m_ArvErrPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        if (m_ArvErrPath.at(m_ArvErrPath.length() - 1) != '/') {
                m_ArvErrPath += '/'; 
        }

        /* 归档完成记录文件路径 */
        ret = iniOpr.GetVal("GatewayArchive", "arvFinishedPath", m_ArvFinishedPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        if (m_ArvFinishedPath.at(m_ArvFinishedPath.length() - 1) != '/') {
                m_ArvFinishedPath += '/'; 
        }

        /* 归档运行时长 */
        ret = iniOpr.GetVal("GatewayArchive", "runTime", tmpStr);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        m_ArvRunTime = atof(tmpStr.c_str());

        /* 是否校验 */
        string chkSig;
        ret = iniOpr.GetVal("GatewayArchive", "isCheck", chkSig);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        m_IsCheck = atoi(chkSig.c_str());

        /* 网络通路网卡编号 */
        ret = iniOpr.GetVal("GatewayArchive", "networkId", m_NetworkId);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
        }

        /* 移动日志路径 */
        ret = iniOpr.GetVal("GatewayArchive", "moveLogPath", m_ArvMovePath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
        }

        /* 磁盘剩余数据量，单位G */
        string tArvLeftSpace;
        ret = iniOpr.GetVal("GatewayArchive", "arvLeftSpace", tArvLeftSpace);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }
        m_ArvLeftSpace = atol(tArvLeftSpace.c_str()) * G;

        /* 读取节点类型是否属于神通 */
        ret = iniOpr.GetVal("GatewayArchive", "nodeType", m_NodeType);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
                return ret;
        }

        if (atoi(m_NodeType.c_str()) == IS_ST) {
                /* ST配置文件路径 */
                ret = iniOpr.GetVal("GatewayArchive", "stConfPath", m_STConfigPath);
                if (ret < 0) {
                        LOG_ERROR("GetVal Error! ret=" << ret);
                }
        }
        return ret;
}

int DDFSArchive::DoArchiveFile(list<pair<string, int64_t> >& TaskMap)
{
        /* 任务列表空 */
        if (TaskMap.empty()) {
                LOG_INFO("TaskMap Is empty");
                return 0;
        }

        int     ret       = 0;
        char    sqlLine[1024];
        string  destFullPath;
        TimeOpr tOpr;
        string  date;

        /* 记录错误日志的全路径 */
        tOpr.GetCurDay(date);
        for (size_t i = 0; i < date.length(); ++i) {
                if (date[i] == '_') {
                        date.erase(i, 1); 
                } 
        }

        size_t curLineNum = 0;
        string destPathTmp = m_DestPath;
        m_PathOpr.EraseLastBias(destPathTmp);
        list<pair<string, int64_t> >::iterator lsiIter = TaskMap.begin();
        for (; lsiIter != TaskMap.end(); ++lsiIter) {
                ++m_LineNum;
                ++curLineNum;

                /* 判断是否已经超时 */
                if (time(NULL) >= m_StartTime + m_ArvRunTime * 3600) {
                        return 1;
                }
                /* 判断磁盘空间是否已经到的指定容量 */
                if (CheckCapacity(lsiIter->second) < 0) {
                        return 2;
                }

                TimeOpr tOpr;
                /* 构造归档的目的路径，用于记录到日志，归档文件 */
                string tErrInfo;
                destFullPath = destPathTmp + lsiIter->first;
                ret = ArchiveFile(destFullPath, lsiIter->first, lsiIter->second, tErrInfo);
                if (ret == -1) {
                        --m_LineNum;
                        /* 归档文件失败，记录错误log并插入数据库 */
                        //m_ArvErrStatus = true;
                        ArvErrInfo aeInfo;
                        aeInfo.srcPath = lsiIter->first;
                        tOpr.GetCurSec(aeInfo.time);
                        aeInfo.errSubject = "Copy File Error!";
                        aeInfo.errInfo = "Copy " + lsiIter->first + " To " 
                                         + destFullPath + "(" + tErrInfo + ")";
                        ret = InsertArvErr(aeInfo);
                        if (ret < 0) {
                                LOG_ERROR("InsertArvErr Error! ret=" << ret);
                        }
                        /* 单个文件归档失败，结束归档，并记录当前归档日志*/
                        int ret = InsertArvLogAndUpdateRecord();
                        if (ret < 0) {
                                LOG_ERROR("DoErrorCopy Error!");
                        }
                        return -1;
                } else if (ret == -2) {
                        /* 记录归档日志失败，不影响 */
                        LOG_INFO("archive file successed, write move log error");
                } else if (ret == -3) {
                        /* 插入数据库失败，不影响 */
                        LOG_INFO("archive file successed, insert database error");
                } else if (ret == -4) {
                        /* st数据库设置只读失败 */ 
                        return -1;
                } else if (ret == 0) {
                        m_ArvTotalSize += lsiIter->second;
                }

                /* 删除错误表中对应记录 */
                if ( m_ErrProcess ) {
                        string tmpStr = "'" + lsiIter->first + "'";
                        string strSql = "delete from sci_arv_errlog where src_path=" + tmpStr;
                        ret = m_DBOpr->Query(strSql);
                        if (ret < 0) {
                                LOG_ERROR("exec sql=" << strSql << " Error!");
                        }
                }

                /* 更新数据库，目前归档到文件的位置，每隔n条记录更新一次 */
                if ( (m_LineNum%m_IntervalRecord)==0 || m_LineNum >= TaskMap.size()) {
                        string curTaskPath = "'" + m_CurTaskPath + "'";
                        string configPath  = "'" + m_ConfigPath  + "'";

                        /* 查找sci_arv_record表中有无m_CurTaskPath对应信息，有的话更新没有insert */
                        vector<vector<string> > arvInfo;
                        string sql = "select * from sci_arv_record where task_path=" + curTaskPath;
                        ret = m_DBOpr->Query(sql, arvInfo);
                        if (ret < 0) {
                                LOG_ERROR("Select Error! sql=" << sql);
                                return -3;
                        }

                        if ( !arvInfo.empty() ) {
                                sprintf(sqlLine, "update sci_arv_record set line_num=%ld where task_path=%s", 
                                                        m_LineNum, curTaskPath.c_str());
                        } else {
                                sprintf(sqlLine, "insert into sci_arv_record(task_path, line_num, config_path) values(%s, %ld, %s)", 
                                                        curTaskPath.c_str(), m_LineNum, configPath.c_str());
                        }

                        ret = m_DBOpr->Query(sqlLine);
                        if (ret < 0) {
                                /* 更新数据库出错 */
                                LOG_ERROR("Query Db Error! sql=" << sqlLine);
                                return -3;
                        }

                        /* 归档完成,删除表中对应信息 */
                        if (curLineNum >= TaskMap.size()) {
                                sprintf(sqlLine, "delete from sci_arv_record where task_path=%s", curTaskPath.c_str());
                                ret = m_DBOpr->Query(sqlLine);
                                if (ret < 0) {
                                        /* 更新数据库出错 */
                                        LOG_ERROR("Delete Db Error! sql=" << sqlLine);
                                        return -2;
                                }
                        }
                }
        }
        return 0;
}

/*
int DDFSArchive::Mount(const string& devName, const string& srcPath)
{
        int ret = 0;

        BaseOpr bOpr;
        ret = bOpr.Mount(devName, srcPath);
        if (ret == 0) {
                return ret;
        }

        ret = bOpr.UnMount(devName);
        if (ret < 0) {
                // 挂载失败，卸载失败，视为目前正处在挂载状态，并且有人在操作
                LOG_INFO("Umount Failed! srcPath=" << srcPath);
                ret = 0;
        } else {
                // 挂载失败，卸载成功，再次挂载失败，记录日志
                ret = bOpr.Mount(devName, srcPath);
                if (ret < 0) {
                        LOG_ERROR("Mount Error! dev=" << devName 
                                                << ", srcPath=" << srcPath);
                }
        }
        return ret;
}
*/

int DDFSArchive::ParseTaskFile(const string& path, int lineNum, string& srcPath, 
                        list<pair<string, int64_t> >& taskMap)
{
        m_CurTaskPath = path;
        //m_LineNum = lineNum != 0? lineNum: 1;
        m_LineNum = lineNum;
        int ret = 0;
        ret = m_LogMgr.ReadArvTask(path, lineNum, srcPath, taskMap);
        if (ret < 0) {
                LOG_ERROR("ReadArvTask Error! ret=" << ret);
        }
        return ret;
}

string DDFSArchive::GetDestPathBySrc(const string& path)
{
        if (m_CanArchivePath.empty()) {
                return  "";
        }
        map<string, string>::iterator mssIter;
        mssIter = m_CanArchivePath.find(path);
        if (mssIter != m_CanArchivePath.end()) {
                return mssIter->second;
        }
        return "";
}

void DDFSArchive::SetLogPath(const string& path)
{
        string tpath = path;
        m_PathOpr.AppendBias(tpath);
        m_BasePath = tpath + "Log/";

        /* m_LogPath  = m_BasePath + "/ArvLog/"; */
        //m_ArvMovePath = m_BasePath + "/MoveLog/";
        /* m_Md5Path  = m_BasePath + "/Md5Log/"; */
}

int DDFSArchive::InsertArvLog(ArvLogInfo& alInfo)
{
        int ret = 0;
        char   sqlLine[1024];
        string fields = "start_time, end_time, total_size, \
                         src_path, dest_path, arv_status";

        string startTime = "'" + alInfo.startTime + "'";
        string endTime   = "'" + alInfo.endTime + "'";
        string srcPath   = "'" + alInfo.srcPath + "'";
        string destPath  = "'" + alInfo.destPath + "'";

        sprintf(sqlLine, "insert into sci_arv_log(%s) \
                                values(%s, %s, %ld, %s, %s, %d)", fields.c_str(), 
                                startTime.c_str(), endTime.c_str(), alInfo.totalSize, 
                                srcPath.c_str(), destPath.c_str(), alInfo.arvStatus);

        ret = m_DBOpr->Query(sqlLine);
        if (ret < 0) {
                LOG_ERROR("Query Error! ret=" << ret << endl << sqlLine);
        }
        return ret;
}

int DDFSArchive::InsertArvErr(ArvErrInfo& aeInfo)
{
        int ret = 0;
        char   sqlLine[4096];
        string fields = "ip, src_path, time, err_subject, err_info";

        string btime       = "'" + aeInfo.time + "'";
        string errSubject  = "'" + aeInfo.errSubject + "'";
        string errInfo     = "'" + aeInfo.errInfo + "'";
        string srcPath     = "'" + aeInfo.srcPath + "'";
        string ip          = "'" + m_CurIp + "'";

        sprintf(sqlLine, "insert into sci_arv_errlog(%s) \
                                values(%s, %s, %s, %s, %s)", fields.c_str(), 
                                ip.c_str(), srcPath.c_str(), btime.c_str(), 
                                errSubject.c_str(), errInfo.c_str());
        ret = m_DBOpr->Query(sqlLine);
        if (ret < 0) {
                LOG_ERROR("Query Error! ret=" << ret);
        }
        return ret;
}

int DDFSArchive::InsertArvDetail(ArvDetailInfo& adInfo)
{
        int ret = 0;
        char   sqlLine[4096];
        string fields = "src_path, dest_path, arv_time, arv_size, ip_addr";

        string  srcPath  = "'" + adInfo.srcPath   + "'";
        string  destPath = "'" + adInfo.destPath  + "'";
        string  arvTime  = "'" + adInfo.arvTime   + "'";
        int64_t arvSize  = adInfo.arvSize;

        string tip = "'" + m_CurIp + "'";

        sprintf(sqlLine, "insert into sci_arv_detail(%s) \
                                values(%s, %s, %s, %ld, %s)", fields.c_str(), 
                                srcPath.c_str(), destPath.c_str(), 
                                arvTime.c_str(), arvSize, tip.c_str());
        ret = m_DBOpr->Query(sqlLine);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sqlLine);
        }
        return ret;
}

int DDFSArchive::InsertArvLogAndUpdateRecord()
{
        char       sqlLine[1024];
        ArvLogInfo alInfo;
        alInfo.startTime = m_BeginSec;
        alInfo.totalSize = m_ArvTotalSize;
        alInfo.srcPath   = m_DevName;
        alInfo.destPath  = m_DestPath;
        alInfo.arvStatus = 1;

        /* 结束时间 */
        TimeOpr tOpr;
        string endSec;
        tOpr.GetCurSec(endSec);
        alInfo.endTime = endSec;
        
        int ret = InsertArvLog(alInfo);
        if (ret < 0) {
                LOG_ERROR("InsertArvLog Error!");
        }

        string curTaskPath = "'" + m_CurTaskPath + "'";
        string configPath  = "'" + m_ConfigPath  + "'";
        /* 查找sci_arv_record表中有无m_CurTaskPath对应信息，有的话更新没有insert */
        vector<vector<string> > arvInfo;
        string sql = "select * from sci_arv_record where task_path=" + curTaskPath;
        ret = m_DBOpr->Query(sql, arvInfo);
        if (ret < 0) {
                LOG_ERROR("Select Error! sql=" << sql);
                return -3;
        }

        if ( !arvInfo.empty() ) {
                sprintf(sqlLine, "update sci_arv_record set line_num=%ld where task_path=%s", 
                                        m_LineNum, curTaskPath.c_str());
        } else {
                sprintf(sqlLine, "insert into sci_arv_record(task_path, line_num, config_path) values(%s, %ld, %s)", 
                                        curTaskPath.c_str(), m_LineNum, configPath.c_str());
        }

        ret = m_DBOpr->Query(sqlLine);
        if (ret < 0) {
                /* 更新数据库出错 */
                LOG_ERROR("Query Db Error! sql=" << sqlLine);
                return -3;
        }
        m_ErrStatus = true;
        return 0;
}

int DDFSArchive::RemoveTaskFile()
{
        int ret = 0;
        /* 移动已经完成的任务 */
        string curTaskFile = m_CurTaskPath.substr(m_CurTaskPath.rfind('/') + 1); 
        string curTaskDir  = m_CurTaskPath.substr(0, m_CurTaskPath.rfind('/'));
        string dirName     = m_ArvFinishedPath 
                + curTaskDir.substr(curTaskDir.rfind('/') + 1); 
        if (!m_PathOpr.HasPath(dirName)) {
                m_PathOpr.MakeDir(dirName);
        }   
        string dest = dirName + '/' + curTaskFile;
        if (rename(m_CurTaskPath.c_str(), dest.c_str()) != 0) {
                LOG_ERROR("move finished task file error"); 
                ret = -1; 
        }   
        return ret;
}

int DDFSArchive::CheckCapacity(int64_t fileSize)
{
        BaseOpr baseOpr;
        int64_t total, left;
        int ret = baseOpr.GetLeftSpaceByMp(m_DestPath, total, left);
        if (ret < 0) {
                LOG_ERROR("GetLeftSpaceByMp Error! dataPath=" << m_DestPath);
                return ret;
        }

        if ( (left - fileSize) < m_ArvLeftSpace ) {
                return -1;
        }
        return 0;
}

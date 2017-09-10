/*************************************************
 * Copyright (C), 2010-2011, Scidata Tech. Co., Ltd.
 *
 * Description: 完成多路径归档任务
 * 
 * Author: 王博
 **************************************************/

#ifndef _DDFS_ARCHIVE_H_
#define _DDFS_ARCHIVE_H_

#include <map>
#include <list>
#include <string>
#include <vector>
#include <stdint.h>

#include "Utils/DB/DBOpr.h"
#include "Archive/DataArchive/ArvLogMgr.h"
#include "Utils/CommonOpr/DirFileOpr.h"

#define ST_CONN_SIZE 4096

using std::map;
using std::list;
using std::string;
using std::vector;

class DDFSArchive
{
public:
        DDFSArchive(const string& configPath);
        ~DDFSArchive();
public:

	/** 
	 * @none: 归档一个分区的任务
	 * @return: 
	 */	
        int ArchiveDevTask(map<string, int64_t>& devTask);

	/** 
	 * @none: 正常归档
	 * @return: successed(0), failed(<0)
	 */	
        //int Archive();
        int Archive(const string& taskPath, int64_t lineNum);

private:
	/** 
	 * @none: 归档初始化，包括读入配置文件，创建源数据的挂在点，连接数据库
	 * @return: successed(0), failed(<0)
	 */
        int Init();

        /**
         * @note 通过源路径获取归档目标路径，如果源路径不存在返回空sring
	 * @return: 归档目标路径或""
         */
        string GetDestPathBySrc(const string& path);

	/** 
	 * @none: 设置归档完成后生成的移动日志的存放路径
	 * @return: successed(0), failed(<0)
	 */	
        void SetLogPath(const string& path);

        /** 
         * @note: 拷贝单个文件,实际调用md5copy的copydata方法
	 * @return: successed(0), failed(<0)
         */
        int CopyData(const string& srcPath, const string& destPath);

        /** 
         * @note: 归档单个文件
         * @param:
         *      destFullPath: 归档后文件的全路径
         *      srcPath: 归档前文件全路径
         *      size: 文件size
	 * @return: successed(0), failed(<0)
         */
        int ArchiveFile(const string& destFullPath, const string& srcPath, 
                        int64_t size, string& errInfo);
        /**
         * @note: 归档任务列表
	 * @return: successed(0), failed(<0)
         */
        int DoArchiveFile(list<pair<string, int64_t> >& TaskMap);

        /**
         * @note: 调用DoArchiveFile归档任务列表，完成后移动任务文件
	 * @return: successed(0), failed(<0)
         */
        int DoArchive(list<pair<string, int64_t> >& TaskMap);

        /**
         * @note 读取归档配置
	 * @return: successed(0), failed(<0)
         */
        int  ReadConfig();

        /**
         * @note 挂载
	 * @return: successed(0), failed(<0)
         */
        //int Mount(const string& devName, const string& srcPath);

        /**
         * @note 解析任务文件，获取归档任务列表
         * @param:
         *      path: 任务文件全路径
         *      lineNum: 上次归档结束时的位置
         *      srcPath: 源路径(目录或者分区)
	 * @return: successed(0), failed(<0)
         */
        int ParseTaskFile(const string& path, int lineNum, string& srcPath,
			  list<pair<string, int64_t> >& taskMap);
        /**
         * @note: 一个分区或源目录完成归档，记录到数据库
	 * @return: successed(0), failed(<0)
         */
        int InsertArvLog(ArvLogInfo& alInfo);

        /**
         * @note 记录归档出错信息，包括mount失败，copy文件失败和更新数据库失败
	 * @return: successed(0), failed(<0)
         */
        int InsertArvErr(ArvErrInfo& aeInfo);

        /**
         * @note 记录归档相信信息日志
	 * @return: successed(0), failed(<0)
         */
        int InsertArvDetail(ArvDetailInfo& adInfo);

        /**
         * @note copy数据出错后执行该函数
         */
        int InsertArvLogAndUpdateRecord();

        /**
         * @note 移动归档完的归档任务文件
         */
        int RemoveTaskFile();

        int InitDDFS();
        int CheckCapacity(int64_t fileSize);
private:
        int     m_IsCheck;                      /* 是否校验                              */
        int     m_SubStrLen;                    /* 分隔符长度                            */
        float   m_ArvRunTime;                   /* 程序运行时长，计算结束时间            */
        int     m_IntervalRecord;               /* 间隔多少条记录更新一次数据库          */
        int64_t m_ArvTotalSize;                 /* 每个归档的数据总量                    */
        int64_t m_ArvLeftSpace;                 /* 指定的磁盘剩余空间                    */
        bool    m_ArvErrStatus;                 /* 归档的错误状态                        */
        bool    m_ErrProcess;                   /* 归档错误的记录                        */
        time_t  m_StartTime;                    /* 开始时间                              */
        size_t  m_LineNum;                      /* 任务文件中归档到的位置                */
        string  m_SrcPath;                      /* 当前归档的源数据路径                  */
        string  m_DestPath;                     /* 当前归档的目标数据路径                */
        string  m_BasePath;                     /* 存放所有日志的开始路径                */
        string  m_SubStr;                       /* 日志中分隔符，防止目录中存在特殊字符  */
        string  m_DevName;                      /* 设备名                                */
        string  m_CurTaskPath;                  /* 当前归档任务的路径                    */
        string  m_ArvTaskPath;                  /* 归档任务记录路径                      */
        string  m_ArvErrPath;                   /* 归档过程出现错误记录路径              */
        string  m_ArvFinishedPath;              /* 归档完成任务路径                      */
        string  m_ArvMovePath;                  /* 移动任务记录路径                      */
        string  m_ConfigPath;                   /* 配置文件全路径                        */
        map<string, string> m_CanArchivePath;   /* 准备归档的路径集合                    */

private:
        DirFileOpr      m_PathOpr;
        ArvLogMgr       m_LogMgr;
        DBOpr*          m_DBOpr;
        string          m_BeginSec;
        string          m_CurIp;
        string          m_NetworkId;

        string          m_NodeType;
        string          m_STConfigPath;             
        char            m_ConnInfo[ST_CONN_SIZE];

        bool            m_ErrStatus;
};

#endif //_DDFS_ARCHIVE_H_

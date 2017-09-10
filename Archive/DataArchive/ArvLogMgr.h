#ifndef _LOGMGR_H_
#define _LOGMGR_H_

#include <list>
#include <string>

#include "Utils/CommonOpr/DirFileOpr.h"

using std::pair;
using std::list;
using std::string;

class ArvLogMgr
{
public:
        ArvLogMgr();
        ~ArvLogMgr();

public:
        /** 
         * @note: 解析归档配置文件
         *
         * @parameter:
         *      path: 归档任务文件全路径
         *      lineNum: 上次归档结束时归档到的文件所在行
         *      srcPath: 界面设置的归档源目录或者分区
         *      taskMap: 解析出的源&目标路径集合
         *
	 * @return: successed(0), failed(<0)
         */
        int ReadArvTask(const string& path, int lineNum, string& srcPath, 
                                list<pair<string, int64_t> >& taskMap);

        /** 
         * @note: 写移动任务文件, 该文件为归档正确结束后，需要移动的源数据
         *        文件列表，包括源数据文件全路径，移动目标位置全路径。
         *
         * @parameter:
         *      path: 移动任务文件全路径
         *      srcFile: 要移动的文件的路径
         *      destFile: 要移动到的位置
         *
	 * @return: successed(0), failed(<0)
         */
        int WriteMoveTask(const string& path, const string& srcFile, 
                                const string& destFile);

        /** 
         * @note: 归档出错时记录正在归档的文件信息到错误日志
         *
         * @parameter:
         *      path: 错误任务文件全路径
         *      srcPath: 正在归档的文件坐在的分区或者源目录
         *      taskPath: 正在归档的人物文件路径
         *      fileSize: 正在归档的文件大小
         *
	 * @return: successed(0), failed(<0)
         */
        int WriteErrorTask(const string& path, const string& srcPath, 
                                const string& taskPath, int64_t fileSize);
private:
        /* 任务文件中分隔源&目的路径的分隔符，默认用",.?" */
        string m_SubStr;
        /* 分隔符的长度 */
        size_t m_SubStrLen;
};

#endif

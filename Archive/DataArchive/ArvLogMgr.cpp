#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/TimeOpr.h"
#include "Archive/DataArchive/ArvLogMgr.h"

using std::ifstream;
using std::cout;
using std::endl;

ArvLogMgr::ArvLogMgr():m_SubStr(",.?"), m_SubStrLen(m_SubStr.size())
{
        
}

ArvLogMgr::~ArvLogMgr()
{

}

int ArvLogMgr::ReadArvTask(const string& path, int lineNum, string& srcPath, 
                                list<pair<string, int64_t> >& taskMap)
{
        DirFileOpr dfOpr;
        if (!dfOpr.HasPath(path)) {
                LOG_INFO("path not exist! path=" << path); 
                return 0;
        }
        
        string  line;
        string  parPath;
        string  strSize;
        int64_t size = 0;
        size_t idx = 0;

        ifstream fin(path.c_str());
        if (!fin) {
                LOG_ERROR("open error!" << path);
                return -1;
        }

        int cnt = 1;
        while (getline(fin, line)) {
                if (cnt <= lineNum) {
                        if (cnt == 1) {
                                srcPath = line;
                        }
                        ++cnt;
                        continue;
                }

                idx     = line.find(m_SubStr);
                if (idx == string::npos) {
                        LOG_ERROR("任务文件格式错误,lineNum=" << lineNum);
                }
                parPath = line.substr(0, idx);
                strSize = line.substr(idx+m_SubStrLen);
                size    = atoll(strSize.c_str());
                taskMap.push_back(std::make_pair(parPath, size));
        }
        return 0;
}

int ArvLogMgr::WriteErrorTask(const string& path, const string& srcPath, 
                                const string& taskPath, int64_t fileSize)
{
        FILE* file  = NULL;
        DirFileOpr dfOpr;

        /* 建立上层目录 */
        string  parPath;
        dfOpr.GetParPath(path, parPath);
        if ( !dfOpr.HasPath(parPath) ) {
                int ret = dfOpr.MakeDir(parPath);
                if (ret < 0) {
                        LOG_ERROR("MakeDir Error! parPath=" << parPath);
                        return ret;
                }
        }

        /* 存在的话追加写，不存在创建 */
        if ( !dfOpr.HasPath(path) ) {
                file = fopen(path.c_str(), "w+");
                if (file == NULL) {
                        LOG_ERROR("fopen path="<<path);
                        return -1;
                }
                fprintf(file, "%s\n", srcPath.c_str());
        } else {
                file = fopen(path.c_str(), "a+");
                if (file == NULL) {
                        LOG_ERROR("fopen path="<<path);
                        return -2;
                }
        }
        fprintf(file, "%s,.?%ld\n", taskPath.c_str(), fileSize);

        fclose(file);

        return 0;
}

int ArvLogMgr::WriteMoveTask(const string& path, const string& srcFile, 
                                        const string& destFile)
{
        string tmpPath = path;
        DirFileOpr dfOpr;
        dfOpr.AppendBias(tmpPath);
        string mvFileName = tmpPath + "mvLog";

        pthread_t pt = pthread_self();

        char buf[1024];
        sprintf(buf, "%s.%ld", mvFileName.c_str(), pt);

        int ret = 0;
        if (!dfOpr.HasPath(tmpPath)) {
                ret = dfOpr.MakeDir(tmpPath);
                if (ret != 0) {
                        LOG_ERROR("mkdir for mvLog error! tmpPath=" << tmpPath);
                        return ret;
                }
        } 

        int mvFd = open(buf, (O_RDWR | O_CREAT | O_APPEND), 0644);
        if (mvFd < 0) {
                LOG_ERROR("open move file error! errInfo: " << strerror(errno));
                return -1;
        }

        string content = srcFile + ",.?" + destFile + "\n";
        ret = write(mvFd, content.c_str(), content.size());
        if (ret < 0) {
                LOG_ERROR("write move info to file error! errInfo: " << strerror(errno));
                close(mvFd);
                return ret;
        }
        ret = close(mvFd);
        if (ret != 0) {
                LOG_ERROR("close move file error");
        }

        if ( dfOpr.IsMaxRecord(buf) ) {
                TimeOpr to;
                string timeName;
                string newFileName;
                to.GetCurMicroSec(timeName);
                newFileName = tmpPath + timeName;
                ret = rename(buf, newFileName.c_str());
                if (ret < 0) {
                        LOG_ERROR("rename error! fileName=" << buf << " errInfo:" << strerror(errno));
                }
        }
        return ret;
}

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

#include "Utils/Log/Log.h"
#include "Utils/DB/DBOpr.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Archive/DataArchive/TaskMgr.h" 
#include "GatewayBG/Operation/DiskMgr.h"

TaskMgr::TaskMgr()
{
}

TaskMgr::~TaskMgr()
{
}

int TaskMgr::GetDBTask(const string& configPath, map<string, int64_t> &task)
{
        int ret = 0;
        DBOpr db;
        vector<vector<string> > info;

        ret = db.Init();
        if (ret != 0) {
                LOG_ERROR("init database error");
                return ret;
        }

        ret = db.Connect();
        if (ret != 0) {
                LOG_ERROR("connect database error");
                return ret;
        }

        string tcp = "'" + configPath + "'";
        string sql = "select task_path,line_num from sci_arv_record where config_path=" + tcp;
        ret = db.Query(sql, info);
        if (ret != 0) {
                LOG_ERROR("query database error! sql=" << sql);
                return ret;
        }
        db.Close();

        vector<vector<string> >::iterator it = info.begin();
        for (; it != info.end(); ++it) {
                int64_t lineNum = atoi(it->back().c_str());
                ret = access(it->front().c_str(), F_OK);
                if (ret == 0) {
                        task.insert(make_pair(it->front(), lineNum));
                } else {
                        LOG_INFO("task file [" << it->front() << "] not exist!");
                }
        }
        return 0;
}

int TaskMgr::GetTaskPath(const string &configPath, string& path)
{
        /* 读取配置信息，确认归档的参数 */
        int ret = 0;

        IniParser iniOpr(configPath);
        ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        ret = iniOpr.GetVal("GatewayArchive", "arvTaskPath", path);
        if (ret < 0) {
                LOG_ERROR("parse arvTaskPath error! ret=" << ret);
        }
        return ret;
}

int TaskMgr::GetAllTask(const string& configPath, map<string, int64_t> &task)
{
        int ret = 0;
        string path;
        DirFileOpr dfOpr;
        set<string> files;

        ret = RemoveEmptyTaskDir(configPath);
        if (ret != 0) {
                LOG_ERROR("remove empty task dir error"); 
                return ret;
        }

        ret = GetDBTask(configPath, task);
        if (ret != 0) {
                LOG_ERROR("Get database task file error"); 
                return ret;
        }

        ret = GetTaskPath(configPath, path);
        if (ret != 0) {
                LOG_ERROR("get arv task path error"); 
                return ret;
        }

        ret = dfOpr.GetAllFileSet(path, files);
        if (ret != 0) {
                LOG_ERROR("Get all task file error"); 
                return ret;
        }

        set<string>::iterator it = files.begin();
        for (; it != files.end(); ++it) {
                task.insert(make_pair(*it, 1));
        }
        return 0;
}

int TaskMgr::RemoveEmptyTaskDir(const string& configPath)
{
        int ret = 0;
        string path;
        set<string> dirs;
        DirFileOpr dfOpr;

        ret = GetTaskPath(configPath, path);
        if (ret != 0) {
                LOG_ERROR("get arv task path error"); 
                return ret;
        }

        dfOpr.GetDirSet(path, dirs);
        set<string>::iterator it = dirs.begin();
        for (; it != dirs.end(); ++it) {
                if(!dfOpr.IsDirEmpty(*it)) {
                        continue;
                }
                ret = remove(it->c_str()); 
                if (ret != 0) {
                        LOG_ERROR("remove dir error, errorInfo: " << strerror(errno)); 
                        return ret;
                }
        }
        return 0;
}

int TaskMgr::GetTaskVolume(const string& path, string &vol)
{
        FILE *stream = fopen(path.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }
        
        DirFileOpr dfOpr;
        const int LEN = 1024;
        char buffer[LEN];

        if (fgets(buffer, LEN, stream) != NULL) {
                string tmp = buffer;
                tmp.erase(tmp.find('\n'));
                if (dfOpr.IsBlockDevice(tmp)) {
                        vol = tmp;
                } else {
                        DiskMgr diskMgr;
                        diskMgr.GetPartitionFromDir(tmp, vol);
                }
        }
        fclose(stream);

        return 0;
}

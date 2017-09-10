#include <map>
#include <fstream>
#include <iostream>
using namespace std;

#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/Lock.h"
#include "GatewayBG/Operation/DiskMgr.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

int BackupFile(const string& path);
int DelBak(const string& path);

int DoDeleteFile(const string& path);
int DeleteFile(const string& logPath);

int WriteFile(const string& path, const map<string, string>& fileMap);
int GetArchiveInfo(const string& logPath, map<string, string>& fileMap);

//----------
int main(int argc, char* argv[])
{
        if (argc != 2) {
                cout << "argv[0] = DeleteData"   << endl;
                cout << "argv[1] = 移动日志路径" << endl;
                return 0;
        }
        int ret = 0;
        string logPath  = argv[1];

        Lock lockFile;
        //程序已经运行
        if ( lockFile.LockFile("/var/run/ddfsArvDel.pid") == LOCK_FAIL ) {
                LOG_INFO("AlreadyRunning!!!");
                return 0;
        }

        ret = DeleteFile(logPath);
        if (ret < 0) {
                LOG_ERROR("DeleteFile Error!");
        }
        return 0;
}

int DeleteFile(const string& logPath)
{
        int ret = 0;
        DirFileOpr dfo;

        set<string> fileSet;
        ret = dfo.GetFileSet(logPath, fileSet);
        if (ret < 0) {
                LOG_ERROR("GetFileSet Error! ret=" << ret);
                return ret;
        }

        map<string, string>::iterator mssIter;
        set<string>::iterator ssIter = fileSet.begin();
        //第一个循环取到删除日志文件
        for (; ssIter != fileSet.end(); ++ssIter) {
                map<string, string> fileMap;
                ret = GetArchiveInfo(*ssIter, fileMap);
                if (ret < 0) {
                        LOG_ERROR("GetArchiveInfo Error! path=" << *ssIter);
                        continue;
                }
                //第二个循环去到文件中内容，并执行删除
                for (mssIter=fileMap.begin(); mssIter!=fileMap.end(); ++mssIter) {
                        if (!dfo.HasPath(mssIter->first)) {
                                //目标也不存在，直接进入下一个
                                if (!dfo.HasPath(mssIter->second)) {
                                        fileMap.erase(mssIter->first);
                                        continue;
                                }
                                ret = DoDeleteFile(mssIter->second);
                                if (ret < 0) {
                                        LOG_ERROR("DoDeleteFile Error! path=" << mssIter->second);
                                } else {
                                        fileMap.erase(mssIter->first);
                                }
                        }
                }
                //已经全部完成后删除日志文件
                if (fileMap.empty()) {
                        ret = DoDeleteFile(*ssIter);
                        if (ret < 0) {
                                LOG_ERROR("DoDeleteFile Error! path=" << *ssIter);
                        }
                        continue;
                }

                //重命名,失败继续下一个
                ret = BackupFile(*ssIter);
                if (ret < 0) {
                        LOG_ERROR("Rename Error! path=" << *ssIter);
                        continue;
                }

                //把余下的源还存在的文件重新写入文件
                ret = WriteFile(*ssIter, fileMap);
                if (ret < 0) {
                        LOG_ERROR("WriteFile Error! path=" << *ssIter);
                        continue;
                }

                //删除备份文件
                ret = DelBak(*ssIter);
                if (ret < 0) {
                        LOG_ERROR("DelBak Error! path=" << *ssIter);
                }
        }
        return ret;
}

int DoDeleteFile(const string& path)
{
        ChildProcessOpr cpo;
        string cmd = "rm -rf '" + path + "'";

        int ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
        }
        return ret;
}

//logPath:日志所在目录
int GetArchiveInfo(const string& logPath, map<string, string>& fileMap) 
{
        string srcPath;
        string destPath;
        ifstream fin(logPath.c_str(), ios::in);

        if (!fin.good()) {
                LOG_ERROR("fin error! path=" << logPath);
                return -2;
        }

        string line;
        size_t idx  = 0;
        while (getline(fin, line)) {
                idx = line.find(",.?");
                if (idx == string::npos) {
                        continue;	
                }
                srcPath  = line.substr(0, idx);
                destPath = line.substr(idx + 3);
                fileMap.insert(make_pair(srcPath, destPath));
        }
        fin.close();
        return 0;
}

int WriteFile(const string& path, const map<string, string>& fileMap)
{
        int ret = 0;
        DirFileOpr dfo;

        FILE* file = fopen(path.c_str(), "w+");
        if (file == NULL) {
                LOG_ERROR("fopen error!");
                return -1;
        }

        map<string, string>::const_iterator mssIter = fileMap.begin();
        for (; mssIter != fileMap.end(); ++mssIter) {
                fprintf(file, "%s,.?%s\n", (mssIter->first).c_str(), (mssIter->second).c_str());
        }

        ret = fclose(file);
        if (ret < 0) {
                LOG_ERROR("fclose error!");
        }
        return ret;
}

int BackupFile(const string& path)
{
        int ret = 0;
        ChildProcessOpr cpo;
        string cmd = "mv '" + path + "' '" + path + ".bak'";
        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
        }
        return ret;
}

int DelBak(const string& path)
{
        int ret = 0;
        ChildProcessOpr cpo;
        string cmd = "rm -rf '" + path + ".bak'";
        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! path=" << cmd);
        }
        return ret;
}

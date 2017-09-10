#include <map>
#include <fcntl.h>
#include <errno.h>
#include <fstream>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

#ifdef _ST_
extern "C" {
#include "Utils/STXR/stxr.h"
} 
#endif //_ST_

#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/Lock.h"
#include "GatewayBG/Operation/DiskMgr.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

using namespace std;

char        connInfo[4096];
string      nodeType;

string      errPath;
DirFileOpr  dfo;
int         firstRun = 1;    

#define DIFF_PARTITION  1  
#define IS_LINK         2

typedef map<string, string> MAP;
typedef map<string, string>::iterator ITER;

int  GetArchiveInfo(const string& logDir, MAP& Links);	

int  DoMoveFile(const string& srcPath, const string& destPath);
int  DoLinkFile(const string& srcPath, const string& destPath);
int  MoveFile(const string& logPath, const string& destPath);

int  WriteMvErrorFile(const string& srcPath, const string& destPath);
int  WriteLnErrorFile(const string& srcPath, const string& destPath);
int  WriteErrorFile(const string& path, const string& srcPath, const string& destPath);

//----------
int main(int argc, char* argv[])
{
        if (argc != 4) {
                cout << "DDFSArvMove Params:"                       << endl;
                cout << "\t[0]process DDFSArvMove"                  << endl;
                cout << "\t[1]移动日志路径(目标路径/Log/MoveLog)"   << endl;
                cout << "\t[2]移动数据路径(数据移动到哪)"           << endl;
                cout << "\t[3]错误日志路径(移动出错后日志存放路径)" << endl;
                cout << endl << "注：移动数据路径必须和源数据路径为同一分区" << endl;
                return 0;
        }

        int ret = 0;
        Lock lockFile;
        //程序已经运行
        if ( lockFile.LockFile("/var/run/ddfsArvDel.pid") == 1 ) {
                LOG_INFO("AlreadyRunning!!!");
                return 0;
        }
        /* 获取节点类型 */
        IniParser iniOpr("/etc/scigw/default/GWconfig");
        ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Ini Init error!");
                return 0;
        }
        iniOpr.GetVal("GatewayArchive", "nodeType", nodeType);

#ifdef _ST_
        if (atoi(nodeType.c_str()) == 1) {
                string stConfPath;
                iniOpr.GetVal("GatewayArchive", "stConfPath", stConfPath);

                char configPath[4096];
                sprintf(configPath, "%s", stConfPath.c_str());
                ret = STGetConnInfo(configPath, connInfo, 4096);
                if (ret != 0) {
                        LOG_ERROR("STGetConnInfo Error! configPath=" << configPath << " ,ret=" << ret);
                        return -2; 
                }
        }
#endif //_ST_

        /*--------------------*/
        errPath = argv[3];
        dfo.AppendBias(errPath);

        string logPath  = argv[1];
        string dataPath = argv[2];

        if (!dfo.HasPath(logPath)) {
                cout << endl << "\t输入的日志路径不存在，请确认!" << endl << endl;
                return 0;
        } else if(!dfo.IsDir(logPath)) {
                cout << endl << "\t输入的日志路径不是目录，请确认!" << endl << endl;
                return 0;
        }
        if (!dfo.IsDir(dataPath)) {
                cout << endl << "\t输入的移动数据路径不是目录，请确认!" << endl << endl;
                return 0;
        } else if (!dfo.HasPath(dataPath)) {
                cout << endl << "\t输入的移动数据路径不存在，请确认!" << endl << endl;
                return 0;
        }
        if (!dfo.IsDir(errPath)) {
                cout << endl << "\t输入的错误路径不是目录，请确认!" << endl << endl;
                return 0;
        }

        set<string> fileSet;
        ret = dfo.GetFileSet(logPath, fileSet);
        if (ret < 0) {
                LOG_ERROR("GetFileSet Error! ret=" << ret);
                return ret;
        }

        set<string>::iterator setIter = fileSet.begin();
        for (; setIter != fileSet.end(); ++setIter) {
                ret = MoveFile(*setIter, dataPath);
                if (ret < 0) {
                        LOG_ERROR("MoveFile Error!");
                        continue;
                } else if (ret == DIFF_PARTITION) {
                        cout << "info: "  << endl;
                        cout << "\t******************** 错误 ***********************" << endl;
                        cout << "\t*                                               *" << endl;
                        cout << "\t* 移动到的路径和源数据不在同一个分区，无法移动! *" << endl;
                        cout << "\t*                                               *" << endl;
                        cout << "\t*************************************************" << endl;
                        return 0;
                }
        }
        return 0;
}

int MoveFile(const string& logPath,  const string& destPath)
{
        MAP Links;
        int ret = 0;
        if ((ret = GetArchiveInfo(logPath,  Links)) != 0) {
                LOG_ERROR("GetArchiveInfo Error! logPath=" << logPath);
                return ret;
        }

        //重命名 链接
        string dest;
        string destPathTmp = destPath;

        //判断是否和源为同一个分区
        for (ITER it = Links.begin(); it != Links.end(); ++it) {
                //判断源文件是否存在
                if (!dfo.HasPath(it->first) || dfo.IsLink(it->first)) {
                        LOG_INFO("src=" << it->first << " don't exist or is a link!");
                        continue;
                }
                if (firstRun == 1) {
                        DiskMgr dm;
                        string srcPartition, destPartition;
                        int64_t t1, t2;

                        dm.GetDeviceUsageFromDir(it->first, srcPartition, t1, t2);
                        dm.GetDeviceUsageFromDir(destPath, destPartition, t1, t2);
                        if (srcPartition != destPartition) {
                                return DIFF_PARTITION;
                        }
                        firstRun = 0;
                }
                if (atoi(nodeType.c_str()) != 1) {
                        //判断源文件是否被使用
                        if (dfo.IsUsing(it->first)) {
                                LOG_INFO("path=" << it->first << " is using");
                                continue;
                        }
                }
                //判断文件修改时间是否有变化
                if (dfo.IsChanged(it->first, it->second)) {
                        LOG_INFO("path=" << it->first << " has changed");
                        continue;
                }

                //------------------------------------------------------------------
#ifdef _ST_
                char tbsName[4096];
                if (atoi(nodeType.c_str()) == 1) {
                        /* 获取StDB的链接信息 */
                        char tSrcPath[4096];
                        /* 获取配置文件路径和源路径*/
                        sprintf(tSrcPath, "%s", (it->first).c_str());
                        /* 获取表空间名 */
                        ret = STGetTablespaceName(connInfo, tSrcPath, tbsName, 4096);
                        LOG_INFO("STGetTablespaceName tSrcPath=" << tSrcPath << " ,tbsName=" << tbsName << " ,ret=" << ret);
                        if (ret != 0) {
                                LOG_ERROR("STGetTablespaceName tSrcPath=" << tSrcPath << " ,tbsName=" << tbsName << " ,ret=" << ret);
                                return -3;
                        }
                        /* 设置StDB为offline */
                        ret = STSetTablespaceOffline(connInfo, tbsName);
                        LOG_INFO("STSetTablespaceOffline tbsName=" << tbsName << " ,ret=" << ret);
                        if (ret != 0) {
                                LOG_ERROR("STSetTablespaceOffline tbsName=" << tbsName << " ,ret=" << ret);
                                return -4;
                        }
                }
#endif //_ST_
                //--------------------------------------------------------------------

                dfo.EraseLastBias(destPathTmp);
                dest = destPathTmp + it->first;

                ret = DoMoveFile(it->first, dest);
                if (ret < 0) {
                        WriteMvErrorFile(it->first, it->second);
                        LOG_ERROR("DoMoveFile Error! " << it->first << " To " << dest);
#ifdef _ST_
                        if (atoi(nodeType.c_str()) == 1) {
                                ret = STSetTablespaceOnline(connInfo, tbsName);
                                LOG_INFO("STSetTablespaceOnline return ret=" << ret << " ,tbsName=" << tbsName);
                        }
#endif //_ST_
                        continue;
                }

                //链接失败, 移动回去
                ret = DoLinkFile(it->first, it->second);
                if (ret < 0) {
                        ret = DoMoveFile(dest, it->first);
                        if (ret < 0) { 
                                WriteLnErrorFile(it->first, it->second);
                        }
                }

                //---------------------------------------------------------
                /* 恢复StDB为online */
#ifdef _ST_
                if (atoi(nodeType.c_str()) == 1) {
                        int cnt = 0;
                        int maxCnt = (2*24*60)/10;  //2天,每10分钟试一次
                        while (cnt++ < maxCnt) {
                                ret = STSetTablespaceOnline(connInfo, tbsName);
                                LOG_INFO("STSetTablespaceOnline return ret=" << ret << " ,tbsName=" << tbsName);
                                if (ret != 0) {
                                        LOG_ERROR("STSetTablespaceOnline Error!");
                                } else {
                                        break;
                                }
                                sleep(600);
                        }
                }
#endif //_ST_
                //--------------------------------------------------------
        }
        return 0;
}

int DoMoveFile(const string& srcPath, const string& destPath)
{
        int ret = 0; 

        string destPar;
        dfo.GetParPath(destPath, destPar);
        if (!dfo.HasPath(destPar.c_str())) {
                ret = dfo.MakeDir(destPar.c_str());
                if (ret < 0) {
                        LOG_ERROR("MkDir Error!");
                        return ret;
                }
        }

        ret = rename(srcPath.c_str(), destPath.c_str());
        if (ret < 0) {
                LOG_ERROR("rename error! errstr: " << strerror(errno) << "  "<< srcPath << " To " << destPath);
        }
        return ret;
}

int DoLinkFile(const string& srcPath, const string& destPath)
{
        int ret = 0;
        ret = symlink(destPath.c_str(), srcPath.c_str());
        if (ret < 0) {
                LOG_ERROR("symlink Error! errstr: " << strerror(errno) << "  " << destPath << " ," << srcPath);
                ret = -1;
        }

        /* TODO: 修改文件属主和用户组 svn:509，步骤：
         * 1 获取 destPath 文件属性（用户和组）
         * 2 修改连接的用户和组
         * DONE
         **/
        struct stat statbuf;  
        ret = lstat(destPath.c_str(), &statbuf);
        if (ret < 0) {
                LOG_ERROR("lstat error! path=" << destPath << " ,strerror=" << strerror(errno));
                return -2;
        }
        ret = lchown(srcPath.c_str(), statbuf.st_uid, statbuf.st_gid);
        if (ret < 0) {
                LOG_ERROR("chown error! path=" << srcPath << " ,strerror=" << strerror(errno));
                return -3;
        }
        return ret;
}

//logPath:日志所在目录
int GetArchiveInfo(const string& logDir, MAP& Links) 
{
        string srcPath;
        string destPath;
        ifstream fin(logDir.c_str(), ios::in);

        if (!fin.good()) {
                return -2;
        }

        string Line;
        string userTmp;
        size_t idx  = 0;
        while (getline(fin, Line)) {
                idx = Line.find(",.?");
                if (idx == string::npos) {
                        continue;	
                }
                srcPath  = Line.substr(0, idx);
                destPath = Line.substr(idx + 3);
                Links.insert(make_pair(srcPath, destPath));
        }
        fin.close();
        return 0;
}

int WriteMvErrorFile(const string& srcPath, const string& destPath)
{
        int ret = 0;

        string moveFilePath = errPath;
        moveFilePath += "MoveErrLog.dat";

        ret = WriteErrorFile(moveFilePath, srcPath, destPath);
        if (ret < 0) {
                LOG_ERROR("WriteErrorFile Error!");
        }
        return ret;
}

int WriteLnErrorFile(const string& srcPath, const string& destPath)
{
        int ret = 0;

        string linkFilePath = errPath;
        linkFilePath += "LinkErrLog.dat";

        ret = WriteErrorFile(linkFilePath, srcPath, destPath);
        if (ret < 0) {
                LOG_ERROR("WriteErrorFile Error!");
        }
        return ret;
}

int WriteErrorFile(const string& path, const string& srcPath, const string& destPath)
{
        int ret = 0;

        if (!dfo.HasPath(errPath.c_str())) {
                ret = dfo.MakeDir(errPath.c_str());
                if (ret < 0) {
                        LOG_ERROR("MkDir Error!");
                        return ret;
                }
        }

        FILE* file = fopen(path.c_str(), "a+");
        if (file == NULL) {
                LOG_ERROR("fopen error!");
                return -1;
        }

        fprintf(file, "%s,.?%s\n", srcPath.c_str(), destPath.c_str());

        ret = fclose(file);
        if (ret < 0) {
                LOG_ERROR("fclose error!");
        }
        return ret;
}

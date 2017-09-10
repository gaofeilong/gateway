#include <stdio.h>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Archive/Email/LogManager.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

using std::ifstream;

DirFileOpr::DirFileOpr()
{
}

DirFileOpr::~DirFileOpr()
{
}

int  DirFileOpr::GetDirSet(const string& path, set<string>& dirSet)
{
        int ret = 0;
        set<string> fileSet;
        ret = TravelDir(path, dirSet, fileSet);
        if (ret < 0) {
                LOG_ERROR("TravelDir Error!");
        }
        return ret;
}

int  DirFileOpr::GetFileSet(const string& path, set<string>& fileSet)
{
        int ret = 0;
        set<string> dirSet;
        ret = TravelDir(path, dirSet, fileSet);
        if (ret < 0) {
                LOG_ERROR("TravelDir Error!");
        }
        return ret;
}

string DirFileOpr::GetFileNameByFullPath(const string& path)
{
        string sub;
        size_t idx = path.find_last_of("/");

        if (idx != string::npos) {
                sub = path.substr(idx+1);
        } else {
                sub = path;
        }
        return sub;
}

void DirFileOpr::GetParPath(const string& path, string& parPath)
{
        int idx = 0;
        string tpath = path;
        EraseLastBias(tpath);
        
        idx     = tpath.find_last_of("/");
        parPath = tpath.substr(0, idx);
}

void DirFileOpr::GetDestPath(const string& refSrcPath, const string& refDestPath, 
                 const string& srcPath, string& destFullPath)
{

        destFullPath = refDestPath;

        if ( '/' != refDestPath[refDestPath.size()-1] ) { 
                destFullPath.append("/"); 
        }

        string strTmp;

        int idx = srcPath.find(refSrcPath) + refSrcPath.size();
        strTmp = srcPath.substr(idx);
        destFullPath += strTmp;
}

int  DirFileOpr::GetPathSize(string path, int64_t& size)
{
        int ret = 0;

        string cmdLine = "du -sb ";
        cmdLine.append("\'");
        cmdLine.append(path);
        cmdLine.append("\'");

        char buffer[255];
        FILE* file = popen(cmdLine.c_str(), "r");
        if (file == NULL) {
                LOG_ERROR("popen error! cmd=" << cmdLine << " strerror=" << strerror(errno));
                return -1;
        }

        char* pathSizeStr = fgets(buffer, 255, file);
        string line;
        if (pathSizeStr != NULL) {
                line = pathSizeStr;
                size_t idx = line.find_first_of(" ");
                if (idx != string::npos) {
                        size = atoll(line.substr(0, idx).c_str());
                } else {
                        size = atoll(line.c_str());
                }
        }
        ret = pclose(file);
        if (ret < 0) {
                LOG_ERROR("pclose error! path=" << path);
        }
        return ret;
}

bool DirFileOpr::HasPath(const string& path)
{
        int ret = 0;
        ret = access(path.c_str(), F_OK);
        if (ret != 0) {
                return false;
        }
        return true;
}

int  DirFileOpr::GetFileMTime(const string& path, int64_t& sec)
{
        int ret = 0;
        struct stat statbuf; 
        ret = lstat(path.c_str(), &statbuf);
        if (ret < 0) {
                LOG_ERROR("lstat error! path=" << path << " ,strerror=" << strerror(errno));
        }
        sec = statbuf.st_mtime;
        return ret;
}

int  DirFileOpr::MakeDir(const string& path)
{
        int ret = 0;
        string tPath = "'" + path + "'";
        string cmd   = "mkdir -p " + tPath;

        ChildProcessOpr cpo;
        ret = cpo.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
        }
        return ret;
}


bool DirFileOpr::IsDir(const string& path)
{
        bool ret = false;
        struct stat st;
        if (lstat(path.c_str(), &st) < 0) {
                LOG_ERROR("lstat error! path=" << path << " ,errstr=" << strerror(errno) );      
                ret = false;
        }
        if (S_ISDIR(st.st_mode)) {
                ret = true;
        }
        return ret;
}

bool DirFileOpr::IsLink(const string& path)
{
        bool ret = false;
        struct stat st;
        if (lstat(path.c_str(), &st) < 0) {
                LOG_ERROR("lstat error! path=" << path << " ,errstr=" << strerror(errno) );      
                ret = false;
        }

        if (S_ISLNK(st.st_mode)) {
                ret = true;
        }
        return ret;
}

bool DirFileOpr::IsBlockDevice(const string& path)
{
        bool ret = false;
        struct stat st;
        if (lstat(path.c_str(), &st) < 0) {
                LOG_ERROR("lstat error! path=" << path << " ,errstr=" << strerror(errno) );      
                ret = false;
        }

        if (S_ISBLK(st.st_mode)) {
                ret = true;
        }
        return ret;
}

bool DirFileOpr::IsDirEmpty(const string& path)
{
        int cnt = 0;
        DIR* dir = NULL;
        dir = opendir(path.c_str());
        if (dir == NULL) {
                LOG_ERROR("opendir error! path=" << path << " ,strerro=" << strerror(errno));
                return false;
        }

        while (readdir(dir) != NULL) {
                ++cnt;
                if (cnt > 2) {
                        closedir(dir);
                        return false;
                }
        }

        closedir(dir);
        return true;
}

void DirFileOpr::AppendBias(string& path)
{
        if ( '/' != path[path.size()-1] ) { 
                path.append("/");
        }
}

void DirFileOpr::EraseLastBias(string& path)
{
        int idx = path.size() - 1;
        if ( '/' == path[idx] ) { 
                path.erase(idx);
        }
}

//private
int DirFileOpr::TravelDir(const string& path, set<string>& dirSet, set<string>& fileSet)
{
        int ret = 0;
        string tpath = path;

        AppendBias(tpath);

        DIR* dp;
        struct dirent* dir;
        struct stat statbuf;  

        dp = opendir(path.c_str());
        if (dp == NULL) {
                LOG_ERROR("opendir error! path=" << path << " ,strerror=" << strerror(errno));
                return -1;
        }

        string pt;
        while ( (dir=readdir(dp)) != NULL) {
                pt = tpath; 
                pt.append(dir->d_name);
                ret = lstat(pt.c_str(), &statbuf);
                if (ret < 0) {
                        LOG_ERROR("lstat error! path=" << pt << " ,strerror=" << strerror(errno));
                        continue;
                }

                if (S_ISDIR(statbuf.st_mode)) {
                        if (strcmp(".", dir->d_name)==0 || strcmp("..", dir->d_name)==0) {
                                continue;
                        }
                        dirSet.insert(pt);
                } else if (S_ISREG(statbuf.st_mode)) {
                        fileSet.insert(pt);
                }
        }

        ret = closedir(dp);
        if (ret < 0) {
                LOG_ERROR("closedir error!");
        }
        return ret;
}

string DirFileOpr::RebulidPath(const string& srcFullPath, const string& srcPath, const string& destPath)
{
        int idx = srcFullPath.find(srcPath);
        string sub = srcFullPath.substr(idx+srcPath.size());
        string tmp = destPath;
        AppendBias(tmp);
        sub = tmp + sub;
        return sub;
}

bool DirFileOpr::IsMaxRecord(const string& path)
{
        //获取每个文件记录条数
        IniParser iniOpr("/etc/scigw/default/GWconfig");
        int ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return false;
        }

        string tmpRecord;
        ret = iniOpr.GetVal("GatewayArchive", "mvMaxRecord", tmpRecord);
        if (ret < 0) {
                LOG_ERROR("GetVal ERROR! ret=" << ret);
                return false;
        }
        int maxRecord = atoi(tmpRecord.c_str());

        string cmd = "wc ";
        cmd += path;
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("excute cmd:wc error! cmd=" << cmd << " ,strerror=" << strerror(errno));
                return false;
        }

        const int bufLen = 32;
        char buf[bufLen];
        memset(buf, 0, bufLen);
        fgets(buf, bufLen, stream);

        int rcdNum = 0;
        sscanf(buf, "%d", &rcdNum);

        pclose(stream);
        return (rcdNum >= maxRecord)? true: false;
}

int DirFileOpr::CheckPath(const string& path, const string& type, string& errInfo)
{
        if (type == "r") {
                return IsRead(path, errInfo);
        }

        if (type == "w") {
                return IsWrite(path, errInfo);
        }
        return 0;
}

int DirFileOpr::IsRead(const string& path, string& errInfo)
{
        FILE* file = fopen(path.c_str(), "r");
        if (file == NULL) {
                errInfo = strerror(errno);
                LOG_INFO("fopen error! path=" << path << " ,errInfo: " << errInfo); 
                return CAN_NOT_READ;
        }

        char buffer[128];
        int ret = fread(buffer, 128, 1, file);
        if (ret < 0) {
                errInfo = strerror(errno);
                LOG_INFO("can't read! path=" << path << " ,errInfo: " << errInfo);
                fclose(file);
                return CAN_NOT_READ;
        }
        fclose(file);
        return CAN_READ;
}

int DirFileOpr::IsWrite(const string& path, string& errInfo)
{
        char tmpName[1024];
        sprintf(tmpName, "%s/%s", path.c_str(), "XXXXXX");

        int fd = mkstemp(tmpName);
        if (fd < 0) {
                errInfo = strerror(errno);
                LOG_ERROR("mkstemp error! errInfo= "<< errInfo);
                return CAN_NOT_WRITE;
        }

        char buffer[10];
        memset(buffer, 0, 10);

        int retVal = CAN_WRITE;
        int ret = write(fd, buffer, 10);
        if (ret < 0) {
                errInfo = strerror(errno);
                LOG_INFO("can,t write! errInfo: " << errInfo); 
                retVal = CAN_NOT_WRITE;
        }
        close(fd);

        ret = remove(tmpName);
        if (ret < 0) {
                LOG_ERROR("remove error! errInfo:" << strerror(errno));
        }
        return retVal;
}

int  DirFileOpr::GetAllFileSet(const string& path, set<string>& fileSet)
{
        int ret = 0;
        set<string> pathList;
        ret = TravelDir(path, pathList, fileSet);
        if (ret < 0) {
                LOG_ERROR("TravelDir Error! ret=" << ret);
                return ret;
        }
        if (pathList.size() == 0) {
                return 0;
        }

        set<string>::iterator lsIter = pathList.begin();
        for (; lsIter != pathList.end(); ++lsIter) {
                ret = GetAllFileSet(*lsIter, fileSet);
                if (ret < 0) {
                        LOG_ERROR("FindAllFile Error! ret=" << ret);
                        return ret;
                }
        }
        return 0;
}

int DirFileOpr::DeleteEmptyDir(const string& path)
{
        int ret = 0;
        set<string> pathSet;
        set<string> fileSet;
        ret = TravelDir(path, pathSet, fileSet);
        if (ret < 0) {
                LOG_ERROR("TravelDir Error! ret=" << ret);
                return ret;
        }

        ChildProcessOpr cpo;
        set<string>::iterator lsIter = pathSet.begin();
        for (; lsIter != pathSet.end(); ++lsIter) {
                //判断目录是否不包含文件，没有文件删除
                if (!HaveFile(*lsIter)) {
                        string cmd = "rm -rf " + *lsIter;
                        ret = cpo.ExecuteCmd(cmd);
                        if (ret < 0) {
                                LOG_ERROR("ExecuteCmd Error! cmd=" << cmd);
                        }
                        continue;
                }
                ret = DeleteEmptyDir(*lsIter);
                if (ret < 0) {
                        LOG_ERROR("FindAllFile Error! ret=" << ret);
                        return ret;
                }
        }
        return 0;
}

bool DirFileOpr::HaveFile(const string& path)
{
        int ret = 0;
        ChildProcessOpr cpo;

        string cmd = "find " + path + " -type f | wc -l";

        vector<string> fileCntVec;
        ret = cpo.Popen(cmd, fileCntVec); 
        if (ret < 0) {
                LOG_ERROR("Popen error! cmd=" << cmd);
                return false;
        }
        
        int fileCnt = 0;
        if (!fileCntVec.empty()) {
                fileCnt = atoi(fileCntVec[0].c_str());
        }
        return (fileCnt==0)?false:true;
}

int DirFileOpr::GetFileSize(const string& path, int64_t& size)
{
        int ret = 0;
        struct stat statbuf; 
        ret = lstat(path.c_str(), &statbuf);
        if (ret < 0) {
                LOG_ERROR("lstat error! path=" << path << " ,strerror=" << strerror(errno));
        }
        size = statbuf.st_size;
        return ret;
}

bool DirFileOpr::IsUsing(const string& path)
{
        bool bret = false;
        ChildProcessOpr cpo;
        string cmd = "lsof '" + path + "' | wc -l";

        vector<string> retVec;
        int ret = cpo.Popen(cmd, retVec);
        if (ret < 0) {
                LOG_ERROR("Popen Error! cmd=" << cmd);
                return bret;
        }
        if (retVec[0] != "0") {
                bret = true;
        }
        return bret;
}

bool DirFileOpr::IsChanged(const string& src, const string& dest)
{
        int ret = 0;
        int bret = false;

        //获取源文件修改时间
        int64_t srcMTime, srcFileSize;
        ret = GetFileMTime(src, srcMTime);
        if (ret < 0) {
                LOG_ERROR("GetFileMTime Error! src=" << src);
                return bret;
        }
        ret = GetFileSize(src, srcFileSize);
        if (ret < 0) {
                LOG_ERROR("GetFileSize Error! src=" << src);
                return bret;
        }

        //获取目标文件修改时间
        int64_t destMTime, destFileSize;
        ret = GetFileMTime(dest, destMTime);
        if (ret < 0) {
                LOG_ERROR("GetFileMTime Error! dest=" << dest);
                return bret;
        }
        ret = GetFileSize(dest, destFileSize);
        if (ret < 0) {
                LOG_ERROR("GetFileSize Error! src=" << dest);
                return bret;
        }

        //查看是否修改
        if ( (srcMTime != destMTime) || (srcFileSize != destFileSize) ) {
                bret = true;
        }
        return bret;
}

bool DirFileOpr::IsMounted(const string& dev, const string& path)
{
        int  ret  = 0;
        bool bret = false;

        ChildProcessOpr cpo;
        string cmd = "mount | grep " + dev + " | grep " + path;
        vector<string> recVec;
        ret = cpo.Popen(cmd, recVec);
        if (ret < 0) {
                LOG_ERROR("popen error!");
                return false;
        }
        if (recVec.size() > 0) {
                bret = true;
        }
        return bret;
}

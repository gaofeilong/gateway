#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <attr/xattr.h>

#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/Lock.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/DDFSOpr.h"
#include "GatewayBG/Operation/DDFSInfo.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

DDFSOpr::DDFSOpr(const string& mp)
{
        m_MountPoint = mp;
}

DDFSOpr::DDFSOpr(const string& mp, const string& configPath)
{
        m_MountPoint = mp;
        m_ConfigPath = configPath;
}

DDFSOpr::~DDFSOpr()
{
}

int DDFSOpr::Mount()
{
        int ret = 0;

        string cmd = "ddfs_mount ";
        cmd.append(m_MountPoint);
        cmd.append(" ");
        cmd.append(m_ConfigPath);

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("exec ddfs_mount error!");
        }
        return ret;
}

int DDFSOpr::UnMount()
{
        int ret = 0;

        string cmd = "ddfs_umount ";
        cmd.append(m_MountPoint);
        cmd.append(" ");
        cmd.append(m_ConfigPath);

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("exec ddfs_umount error!");
        }
        return ret;
}

int DDFSOpr::Fsck(const string& level)
{
        int ret = 0;

        //string cmd = "fsck.ddfs -c " + m_ConfigPath + " -l " + level;
        string cmd = "fsck.ddfs -c " + m_ConfigPath;

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("exec fsck.ddfs error!");
        }
        return ret;
}

int DDFSOpr::SystemUnMount()
{
        int ret = 0;

        string cmd = "umount ";
        cmd.append(m_MountPoint);

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("exec ddfs_mount error!");
        }
        return ret;
}

int DDFSOpr::GetUsedCPURatio(float& ratio)
{
        /* 读取文件 */
        int ret = 0;

        /*
        string infoPath;
        ret = GetInfoPath(infoPath);
        if (ret < 0) {
                LOG_ERROR("GetInfoPath Error!");
                return ret;
        }

        IniParser ini(infoPath);
        ret = ini.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        string strCpu;
        ret = ini.GetVal("INFO", "cpu", strCpu);
        if (ret < 0) {
                LOG_ERROR("GetVal Error!");
                return ret;
        }
        if (!strCpu.empty()) {
                ratio = atof(strCpu.c_str());
        }
        */

        int pid = 0;
        ret = GetMPPID(pid);
        if (ret < 0) {
                LOG_ERROR("GetMPPID Error! ret=" << ret);
        }

        char strPid[10];
        sprintf(strPid, "%d", pid);
        string cmd = "top -d 0.1 -n 2 -b -p ";
        cmd += strPid;

        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open cmd error at function GetCpuRatio()");
                return -1; 
        } 

        int    LEN = 1024;
        char   lineBuf[LEN];
        string line;
        memset(lineBuf, 0, LEN);
        char cpuRatio[10];

        while (fgets(lineBuf, LEN, stream) != NULL) {  
                line = lineBuf;
                if (line.find(strPid) != string::npos) {
                        sscanf(lineBuf, "%*s %*s %*s %*s %*s %*s %*s %*s %s", cpuRatio);
                        ratio = atof(cpuRatio);
                        break;
                }
        }
        pclose(stream);
        return 0;
}

int DDFSOpr::GetUsedMem(int64_t& mem)
{
        return 0;
}

int DDFSOpr::GetUsedMemRatio(float& mem)
{
        /* 读取文件 */
        int ret = 0;
        /*
        string infoPath;
        ret = GetInfoPath(infoPath);
        if (ret < 0) {
                LOG_ERROR("GetInfoPath Error!");
                return ret;
        }

        IniParser ini(infoPath);
        ret = ini.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        string strMem;
        ret = ini.GetVal("INFO", "mem", strMem);
        if (ret < 0) {
                LOG_ERROR("GetVal Error!");
                return ret;
        }
        if (!strMem.empty()) {
                mem = atof(strMem.c_str());
        }
        */

        char cmd[255];
        int pid = 0;
        ret = GetMPPID(pid);
        if (ret < 0) {
                LOG_ERROR("GetMPPID Error! ret=" << ret);
        }

        sprintf(cmd, "ps -p %d -o pmem | awk '{print $1}' | sed -n -e '2p'", pid);

        FILE *pfp = popen(cmd, "r");
        if (pfp == NULL) {
                LOG_ERROR("popen error!");
                return -1;
        }

        char* memRatio = fgets(cmd, 255, pfp);
        if (memRatio != NULL) {
                mem = atof(memRatio);
        }
        ret = pclose(pfp);
        if (ret < 0) {
                LOG_ERROR("pclose error!");
                return -3;
        }
        return 0;
}

int DDFSOpr::GetDedupRatio(float& ratio)
{
        int64_t total = 0;
        int64_t real  = 0;

        int ret = GetDataSize(total, real);
        if (ret < 0) {
                LOG_ERROR("GetDataSize error! ret=" << ret);
                return ret;
        }

        if (real != 0) {
                ratio = 1- (float)real/total;
        }

        if (ratio < 0) {
                ratio = 0;
        }

        return 0;
}

int DDFSOpr::GetDataSize(int64_t& raw, int64_t& real)
{
        /* 读取文件 */
        /*
        string infoPath;
        int ret = GetInfoPath(infoPath);
        if (ret < 0) {
                LOG_ERROR("GetInfoPath Error!");
                return ret;
        }

        IniParser ini(infoPath);
        ret = ini.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        string strRaw;
        ret = ini.GetVal("INFO", "total", strRaw);
        if (ret < 0) {
                LOG_ERROR("GetVal Error!");
                return ret;
        }

        double totalSize;
        SizeToByte(strRaw, totalSize);
        raw = (int64_t)totalSize;

        string strReal;
        ret = ini.GetVal("INFO", "real", strReal);
        if (ret < 0) {
                LOG_ERROR("GetVal Error!");
                return ret;
        }
        double realSize;
        SizeToByte(strReal, realSize);
        real = (int64_t)realSize;
        */

        const char* totalStr = "user.io.total_size";
        const char* realStr = "user.io.real_size";

        int64_t totalBytes = 0;
        int ret = getxattr(m_MountPoint.c_str(), totalStr, &totalBytes, sizeof(totalBytes));
        if(ret < 0) {
                printf("getxattr:%s error\n", totalStr);
                return ret;
        }
        raw = totalBytes;

        int64_t realBytes = 0;
        ret = getxattr(m_MountPoint.c_str(), realStr, &realBytes, sizeof(realBytes));
        if(ret < 0) {
                printf("getxattr:%s error\n", realStr);
                return ret;
        }
        real = realBytes; 
        
        return 0;
}

int DDFSOpr::GetMPState()
{
        int ret = 0;
        int mpStatus = -1;

        IniParser parser(m_ConfigPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        string dataPath;
        parser.GetVal("Plds", "dataPath1", dataPath);
        if (dataPath[dataPath.size()-1] != '/') {
                dataPath.append("/");
        }
        dataPath.append("ddfs.pid");

        Lock       lockFile;
        DirFileOpr dfo;
        if (dfo.HasPath(dataPath)) {
                /* 挂载或崩溃*/
                ret = lockFile.TryLockFile(dataPath);
                if (ret < 0) {
                        LOG_ERROR("GetLockStatus Error!");
                        return ret;
                } else if (ret == LOCK_FAIL) {
                        mpStatus = MOUNTED;
                } else {
                        mpStatus = MP_IS_NOT_CONNECTED;
                }
        } else {
                mpStatus = UNMOUNTED;
        }
        return mpStatus;
}


int DDFSOpr::DeleteMountPoint()
{
        int ret = 0;

        IniParser parser(m_ConfigPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }
        string metaPath;
        string mapPath;
        string biPath;
        string siPath;
        const string PLDS     = "Plds";
        const string LDS      = "Lds";

        parser.GetVal(LDS, "biPath", biPath);
        parser.GetVal(LDS, "siPath", siPath);
        parser.GetVal(PLDS, "metaPath", metaPath);
        parser.GetVal(PLDS, "mapPath", mapPath);

        ChildProcessOpr cmdOpr;
        string cmd = "rm -rf " + metaPath + " " + biPath + " " + mapPath + " " + siPath + " " +  m_MountPoint;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("exec rm error!");
        }

        int            cnt = 1;
        char           buffer[15];
        string         dataPath;
        /*获取数据路径*/
        while (true) {
                sprintf(buffer, "dataPath%d", cnt);
                ret = parser.GetVal(PLDS, buffer, dataPath);
                if (ret == -1) {
                        ret = 0;
                        break;
                }
                string cmd = "rm -rf " + dataPath;
                ret = cmdOpr.ExecuteCmd(cmd);
                if (ret < 0) {
                        LOG_ERROR("exec rm error!");
                }
                ++cnt;
        }
        return ret;
}

int DDFSOpr::DDFSMkfs(const char* configPath)
{
        int ret = 0;

        m_ConfigPath = configPath;

        string cmd = "ddfs_mkfs ";
        cmd.append(m_ConfigPath);

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret != 0) {
                LOG_ERROR("exec ddfs_mkfs error!");
                return -1;
        }
        return 0;
}

string DDFSOpr::GetConfigPath()
{
        return m_ConfigPath;
}

void DDFSOpr::SetConfigPath(const char* configPath)
{
        m_ConfigPath = configPath;
}

int DDFSOpr::GetMPPID(int& pid)
{
        int ret = 0;

        IniParser parser(m_ConfigPath);
        ret = parser.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }

        string dataPath;
        parser.GetVal("Plds", "dataPath1", dataPath);
        if (dataPath[dataPath.size()-1] != '/') {
                dataPath.append("/");
        }
        dataPath.append("ddfs.pid");
        FILE* file = fopen(dataPath.c_str(), "r");
        if (file == NULL) {
                LOG_ERROR("not has pid file");
                return -1;
        }
        ret = fread(&pid, sizeof(pid), 1, file);
        if (ret < 0) {
                LOG_ERROR("fread error! ret=" << ret);
                return -2;
        }
        ret = fclose(file);
        if (ret < 0) {
                LOG_ERROR("fclose error! ret=" << ret);
        }
        return ret;
}

/*
int DDFSOpr::GetInfoPath(string& infoPath)
{
        IniParser ini;
        int ret = ini.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }
        
        string tInfoPath;
        ret = ini.GetVal("NFS", "infoPath", tInfoPath);
        if (ret < 0) {
                LOG_ERROR("GetVal Error!");
                return ret;
        }
        infoPath = tInfoPath + "/" + m_MountPoint + "/status.info";

        return 0;
}

void DDFSOpr::SizeToByte(const string& dataSize, double& byteSize)
{
        string util1 = dataSize.substr(dataSize.size()-2, dataSize.size());
        string util2 = dataSize.substr(dataSize.size()-1, dataSize.size());
        string size;

        if (util1 == "KB" || util2 == "K") {
                size     = dataSize.substr(0, dataSize.size()-2);
                byteSize = atof(size.c_str()) * 1024;

        } else if (util1 == "MB" || util2 == "M") {
                if (util1 == "MB") {
                        size = dataSize.substr(0, dataSize.size()-2);
                } else {
                        size = dataSize.substr(0, dataSize.size()-1);
                }
                byteSize = atof(size.c_str()) * 1024 * 1024;

        } else if (util1 == "GB" || util2 == "G") {
                if (util1 == "GB") {
                        size = dataSize.substr(0, dataSize.size()-2);
                } else {
                        size = dataSize.substr(0, dataSize.size()-1);
                }
                byteSize = atof(size.c_str()) * 1024 * 1024 * 1024;

        } else if (util1 == "TB" || util2 == "T") {
                if (util1 == "TB") {
                        size = dataSize.substr(0, dataSize.size()-2);
                } else {
                        size = dataSize.substr(0, dataSize.size()-1);
                }
                byteSize = atof(size.c_str()) * 1024 * 1024 * 1024 * 1024;

        } else if (util1 == "PB" || util2 == "P") {
                if (util1 == "PB") {
                        size = dataSize.substr(0, dataSize.size()-2);
                } else {
                        size = dataSize.substr(0, dataSize.size()-1);
                }
                byteSize = atof(size.c_str()) * 1024 * 1024 * 1024 * 1024 * 1024;
        } else if (util2 == "B") {
                size     = dataSize.substr(0, dataSize.size()-1);
                byteSize = atof(size.c_str());
        } else {
                size     = dataSize.substr(0);
                byteSize = atof(size.c_str());
        }
}
*/

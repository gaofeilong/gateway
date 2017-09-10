#ifndef _DDFS_OPR_H_
#define _DDFS_OPR_H_

#include <string>
#include <stdint.h>
#include <vector>

using std::string;
using std::vector;

class DDFSOpr
{
public:
        DDFSOpr(const string& mp);
        DDFSOpr(const string& mp, const string& configPath);
        ~DDFSOpr();

public:
        int Mount();
        int SystemUnMount();
        int UnMount();
        int Fsck(const string& level);

        int DeleteMountPoint();
        int GetUsedCPURatio(float& ratio);
        int GetUsedMemRatio(float& mem);
        int GetUsedMem(int64_t& mem);
        int GetDedupRatio(float& ratio);
        int GetDataSize(int64_t& raw, int64_t& real);
        int DDFSMkfs(const char* configPath);
        int GetMPState();

        //void SizeToByte(const string& dataSize, double& byteSize);

        string GetConfigPath();
        void SetConfigPath(const char* configPath);

private:
        int GetMPPID(int& pid);
        //int GetInfoPath(string& infoPath);

private:
        string          m_MountPoint;
        string          m_ConfigPath;
};

#endif


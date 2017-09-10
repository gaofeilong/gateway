#ifndef _DDFS_MGR_H_
#define _DDFS_MGR_H_

#include <map>
#include <string>
#include <vector>

#include "GatewayBG/Operation/DDFSOpr.h"

class DDFSMgr
{
public:
        DDFSMgr();
        ~DDFSMgr();

public:
        int Init();
        int DDFSMkfs(const char* mountpoint, const char* configPath);
        /**
         * 挂载文件系统到指定的目录
         * @return =0 挂载成功
         *         =2 挂载点不存在
         *         =3 挂载点不为空
         *         <0 挂载失败
         */
        int Mount(const char* mountpoint);

        /**
         * 卸载对应的文件系统
         * @return =0 卸载成功
         *         <0 卸载失败
         */
        int UnMount(const char* mountpoint);
        int SystemUnMount(const char* mountpoint);
        int Fsck(const char* mountpoint, const string& level);

        int DeleteMountPoint(const char* mountpoint);

        /**
         * 获取文件系统对应进程的CPU使用率
         * @return =0 成功
         *         <0 失败
         */
        int GetUsedCPURatio(const char* mountpoint, float& ratio);

        /**
         * 获取文件系统对应进程的内存使用率
         * @return =0 成功
         *         <0 失败
         */
        int GetUsedMemRatio(const char* mountpoint, float& mem);
        int GetUsedMem(const char* mountpoint, int64_t& mem);

        /**
         * 获取文件系统的消冗率
         * @return =0 成功
         *         <0 失败
         */
        int GetDedupRatio(const char* mountpoint, float& ratio);

        /**
         * 获取文件系统的数据总量和实际数据
         * @return =0 成功
         *         <0 失败
         */
        int GetDataSize(const char* mountpoint, int64_t& raw, int64_t& real);

        /**
         * 获取挂载点目前的状态
         */
        int  GetMPState(const char* mountpoint);
        /**
         *
         */
        int  GetConfigByMP(const char* mountpoint, string& configPath);
        int  GetMPList(std::vector<std::string>& mpList);
        int  GetOnlineMPMap(std::map<std::string, std::string>& mpMap);
        void SetMPConfigPath(const char* mountpoint, const char* configPath);

        std::map<string,string> GetAllMPMap();

        int  DeleteMysql(const string& mp);
        int  InsertMysql(const string& mp, const string& config);
private:
        DDFSOpr* GetDDFSOpr(const char* mountpoint, const string& configPath="");

private:
        typedef std::map<string, DDFSOpr*>::iterator DDFSMPMapIt;
        std::map<string, DDFSOpr*> m_DDFSMPMap;
        //std::map<string, pair<string, int> > m_MountPointStateMap;
        std::map<string, string>   m_MountPointStateMap;
        string m_CurIp;
};

#endif


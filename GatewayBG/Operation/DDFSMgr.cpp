#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <iostream>

#include "Utils/Log/Log.h"
#include "Utils/DB/DBOpr.h"
#include "Config/IniParser.h"
#include "Utils/CommonOpr/DirFileOpr.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "GatewayBG/Operation/DDFSMgr.h"
#include "GatewayBG/Operation/DDFSInfo.h"

DDFSMgr::DDFSMgr()
{
        m_DDFSMPMap.clear();
}

DDFSMgr::~DDFSMgr()
{
        DDFSMPMapIt it = m_DDFSMPMap.begin();
        for ( ; it != m_DDFSMPMap.end(); ++it ) {
                delete it->second;
        }
        m_DDFSMPMap.clear();
}

int DDFSMgr::Init()
{
        int ret = 0;

        DBOpr db;
        ret = db.Init();
        if (ret < 0) {
                LOG_ERROR("DB Init Error!");
                return ret;
        }

        do {
                ret = db.Connect();
                if (ret < 0) {
                        LOG_ERROR("Connect Error! ret=" << ret);
                        sleep(30);
                }
        } while (ret < 0);

        /* 获取本机Ip */
        BaseOpr baseOpr;
        string networkId;
        ret = baseOpr.GetNetworkId(networkId);
        if (ret < 0) {
                LOG_ERROR("GetNetworkId Error!");
                db.Close();
                return ret;
        }

        ret = baseOpr.GetIp(networkId, m_CurIp);
        if (ret < 0) {
                LOG_ERROR("GetIp Error!");
                db.Close();
                return ret;
        }

        string cmd = "select * from sci_mp_status where ip_addr='" + m_CurIp + "'";
        vector<vector<string> > cont;
        ret = db.Query(cmd, cont);
        if (ret < 0) {
                LOG_ERROR("Query Error!");
                db.Close();
                return ret;
        }

        for (unsigned int i=0; i<cont.size(); ++i) {
                m_MountPointStateMap.insert( make_pair(cont[i][0], cont[i][1]) );
        }

        map<string, string>::iterator mspIter;
        for(mspIter = m_MountPointStateMap.begin(); mspIter != m_MountPointStateMap.end(); ++mspIter) {
                SetMPConfigPath(mspIter->first.c_str(), mspIter->second.c_str());
        }
        db.Close();

        return ret;
}

int DDFSMgr::DDFSMkfs(const char* mountpoint, const char* configPath)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint, configPath);

        int mpState = ddfsOpr->GetMPState();
        if (mpState == MOUNTED) {
                LOG_INFO("mount point is unmounted!");
                return mpState;
        }
        
        map<string, string>::iterator iter = m_MountPointStateMap.find(mountpoint);
        if (iter != m_MountPointStateMap.end()) {
                return MP_IS_EXIST;
        }

        /*
         * 先插入数据库并放入m_MountPointStateMap中
         */
        int ret = InsertMysql(mountpoint, configPath);
        if (ret < 0) {
                LOG_ERROR("InsertMysql Error!");
                return ret;
        }
        m_MountPointStateMap.insert( make_pair(mountpoint, configPath) );

        ret = ddfsOpr->DDFSMkfs(configPath);
        if ( ret < 0 ) {
                LOG_ERROR("ddfsmkfs mountpoint="<< mountpoint << " configPath=" << configPath << " ,error!ret="<<ret);
                DeleteMysql(mountpoint);
                /* 删除map中对应的挂载点信息 */
                m_MountPointStateMap.erase(mountpoint);

                /* 删除ddfsmp中对应挂载点的对象并释放资源 */
                DDFSMPMapIt iter = m_DDFSMPMap.find(mountpoint);
                if (iter != m_DDFSMPMap.end()) {
                        delete iter->second;
                        m_DDFSMPMap.erase(iter);
                }
        }
        return ret;
}

DDFSOpr* DDFSMgr::GetDDFSOpr(const char* mountpoint, const string& configPath)
{
        string mp = mountpoint;
        DDFSOpr* ddfsOpr = NULL;
        DDFSMPMapIt it = m_DDFSMPMap.find(mp);
        if ( it == m_DDFSMPMap.end() ) {
                if (configPath.empty()) {
                        ddfsOpr = new DDFSOpr(mp);
                } else {
                        ddfsOpr = new DDFSOpr(mp, configPath);
                }
                m_DDFSMPMap.insert(std::make_pair(mp, ddfsOpr));
        } else {
                ddfsOpr = it->second;
        }
        return ddfsOpr;
}

int DDFSMgr::Mount(const char* mountpoint)
{
        //目录不存在
        DirFileOpr dfOpr;
        if ( !dfOpr.HasPath(mountpoint) ) {
                LOG_INFO("mountpoint not exist! mountpoint=" << mountpoint);
                return MP_NOT_EXIST;
        }

        //目录不为空
        if ( !dfOpr.IsDirEmpty(mountpoint) ) {
                LOG_INFO("mountpoint not empty! mountpoint=" << mountpoint);
                return MP_IS_NOT_EMPTY;
        }

        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        //没有配置文件
        string configPath = ddfsOpr->GetConfigPath();
        if (configPath.empty()) {
                LOG_INFO("not has configFile!");
                return MP_NOT_HAS_CONFIG_FILE;
        }

        int mpState = ddfsOpr->GetMPState();
        if (mpState == MOUNTED) {
                LOG_INFO("mount point is mounted!");
                return mpState;
        }

        int ret = ddfsOpr->Mount();
        if (ret < 0) {
                LOG_ERROR("mount point:"<<(string)mountpoint<<",error!ret="<<ret);
        }
        return ret;
}

int DDFSMgr::UnMount(const char* mountpoint)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState == UNMOUNTED) {
                LOG_INFO(mountpoint << " mount point is unmounted!");
                return mpState;
        }

        int ret = ddfsOpr->UnMount();
        //int ret = ddfsOpr->SystemUnMount();
        if (ret < 0) {
                LOG_ERROR("umount mount point:"<< mountpoint <<",error!ret="<< ret);
        }
        return ret;
}

int DDFSMgr::Fsck(const char* mountpoint, const string& level)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState == MOUNTED || mpState == UNMOUNTED) {
                return 0;
        }

        int ret = 0;
        ret = ddfsOpr->SystemUnMount();
        if (ret < 0) {
                LOG_ERROR("SystemUnMount Error!");
        }

        ret = ddfsOpr->Fsck(level);
        if ( ret < 0 ) {
                LOG_ERROR("fsck mount point:"<< mountpoint <<",error!ret="<<ret);
        }
        return ret;
}

int DDFSMgr::SystemUnMount(const char* mountpoint)
{
        int ret = 0;

        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState == UNMOUNTED) {
                LOG_INFO("mount point is unmounted!");
                return mpState;
        }

        ret = ddfsOpr->SystemUnMount();
        if ( ret < 0 ) {
                LOG_ERROR("mount point:"<<(string)mountpoint<<",error!ret="<<ret);
        }
        return 0;
}

int DDFSMgr::GetUsedCPURatio(const char* mountpoint, float& ratio)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState != MOUNTED) {
                return mpState;
        }

        int ret = ddfsOpr->GetUsedCPURatio(ratio);
        if (ret < 0) {
                LOG_ERROR("GetUsedCPURatio Error! ret=" << ret);
        }
        return ret;
}

int DDFSMgr::GetUsedMem(const char* mountpoint, int64_t& mem)
{
        return 0;
}

int DDFSMgr::GetUsedMemRatio(const char* mountpoint, float& mem)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState != MOUNTED) {
                return mpState;
        }

        int ret = ddfsOpr->GetUsedMemRatio(mem);

        if (ret < 0) {
                LOG_ERROR("GetUsedCPURatio Error! ret=" << ret);
        }
        return ret;
}

int DDFSMgr::GetDedupRatio(const char* mountpoint, float& ratio)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState != MOUNTED) {
                return mpState;
        }

        int ret = ddfsOpr->GetDedupRatio(ratio);
        if (ret < 0) {
                LOG_ERROR("GetDedupRatio Error! ret=" << ret);
        }
        return ret;
}

int DDFSMgr::GetDataSize(const char* mountpoint, int64_t& raw, int64_t& real)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState != MOUNTED) {
                return mpState;
        }

        int ret = ddfsOpr->GetDataSize(raw, real);
        if (ret < 0) {
                LOG_ERROR("GetDataSize Error! ret=" << ret);
        }
        return ret;
}

int DDFSMgr::GetMPState(const char* mountpoint)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        return ddfsOpr->GetMPState();
}

int DDFSMgr::GetMPList(std::vector<std::string>& mpList)
{
        map<string, string>::iterator it = m_MountPointStateMap.begin();
        for (; it != m_MountPointStateMap.end(); ++it) {
                mpList.push_back(it->first);
        }
        return 0;
}

int DDFSMgr::GetOnlineMPMap(std::map<std::string, std::string>& mpMap)
{
        map<string, string>::iterator it = m_MountPointStateMap.begin();
        int state = 0;
        for (; it != m_MountPointStateMap.end(); ++it) {
                state = GetMPState( (it->first).c_str() );
                if (state == MOUNTED) {
                        mpMap.insert( std::make_pair(it->first, it->second) );
                }
        }
        return 0;
}

map<string,string> DDFSMgr::GetAllMPMap()
{
        return m_MountPointStateMap;
}

int DDFSMgr::DeleteMountPoint(const char* mountpoint)
{
        int ret = 0;
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);

        int mpState = ddfsOpr->GetMPState();
        if (mpState == MOUNTED) {
                LOG_INFO("mountpoint is mounted " << mountpoint);
                return mpState; 
        }

        ret = ddfsOpr->DeleteMountPoint();
        if (ret < 0) {
                LOG_ERROR("DeleteMountPoint Error! ret=" << ret);
        }
        
        //删除数据库中信息和m_MountPointStateMap中的信息
        DDFSMPMapIt iter = m_DDFSMPMap.find(mountpoint);
        if (iter != m_DDFSMPMap.end()) {
                delete iter->second;
                m_DDFSMPMap.erase(iter);
                m_MountPointStateMap.erase(mountpoint);
        }

        ret = DeleteMysql(mountpoint);
        if (ret < 0) {
                LOG_ERROR("DeleteMysql Error!");
        }
        return ret;
}

void DDFSMgr::SetMPConfigPath(const char* mountpoint, const char* configPath)
{
        DDFSOpr* ddfsOpr = GetDDFSOpr(mountpoint);
        ddfsOpr->SetConfigPath(configPath);
}

int DDFSMgr::GetConfigByMP(const char* mountpoint, string& configPath)
{
        map<string, string>::iterator ssIter = m_MountPointStateMap.find(mountpoint);
        if (ssIter != m_MountPointStateMap.end()) {
                configPath = ssIter->second;
        } else {
                LOG_INFO("not find configPath! mountpoint=" << mountpoint);
                return MP_NOT_HAS_CONFIG_FILE;
        }

        return 0;
}

int DDFSMgr::InsertMysql(const string& mp, const string& config)
{
        DBOpr db;
        db.Init();
        int ret = db.Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error! ret=" << ret);
        }

        string mpTmp     = "'" + mp + "'";
        string configTmp = "'" + config + "'";

        string sql       = "insert into sci_mp_status values(" + mpTmp + "," + configTmp + ",'" + m_CurIp + "')";

        ret = db.Query(sql);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sql);
        }
        db.Close();

        return ret;
}

int DDFSMgr::DeleteMysql(const string& mp)
{
        DBOpr db;
        db.Init();
        int ret = db.Connect();
        if (ret < 0) {
                LOG_ERROR("Connect Error! ret=" << ret);
        }

        string mpTmp     = "'" + mp + "'";
        string sql       = "delete from sci_mp_status where mp_path=" + mpTmp + 
                           "and ip_addr='" + m_CurIp + "'";

        ret = db.Query(sql);
        if (ret < 0) {
                LOG_ERROR("Query Error! sql=" << sql);
        }
        db.Close();

        return ret;
}

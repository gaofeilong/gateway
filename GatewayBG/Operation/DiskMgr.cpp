#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sstream>
#include <algorithm>
#include "GatewayBG/Operation/DiskMgr.h"
#include "GatewayBG/Operation/MultiPathMgr.h"
#include "Utils/Log/Log.h"


using namespace std;
const int LEN = 128;

DiskMgr::DiskMgr()
{
}

DiskMgr::~DiskMgr()
{
}

int DiskMgr::GetAllDevName(set<string>& Devs)
{
        string line;
        size_t len;
        char   *buf = NULL;
        string cmd = "fdisk -l 2>/dev/null|awk -F [\": \"] '/^Disk \\/dev\\/sd/{print $2}'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("get disks info error");
                return -1;
        }
        //cout << "***************** all dev *****************" << endl;
        while (getline(&buf, &len, stream) != -1) {
                line = buf;
                line.erase(line.length() - 1);
                Devs.insert(line);
                //cout << line << endl;
        }
        free(buf);
        //cout << "***************** all dev end *************" << endl;

        if (0 != pclose(stream)) {
                LOG_ERROR("close stream error");
        }
        return 0;	
}

int DiskMgr::GetAllDiskName(string DevName, vector<string>& Disks)
{
        string cmd;
        string line;
        string diskName;
        size_t len;
        char   *buf = NULL;
        cmd = "fdisk -l 2>/dev/null|grep \"^" + DevName + "\"";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("get partition info error");
                return -1;
        }
        while (getline(&buf, &len, stream) != -1) {
                int isBoot = 0;
                line = buf;
                if (line.find("Extended") != string::npos ||    
                        line.find("Linux swap") != string::npos) {      //不显示
                        continue;
                } else if (line.find("*") != string::npos) {
                        isBoot = 1;
                }

                string mp, fs;
                diskName = line.substr(0, line.find_first_of(" \t\n") + 1);
                GetDiskMountPoint(diskName, mp, fs);
                if (isBoot == 1 && mp == "/boot") {
                        continue; 
                }
                if (!isdigit(diskName[DevName.length()])) {
                        continue; 
                }
                Disks.push_back(diskName);
        }
        free(buf);

        if (0 != pclose(stream)) {
                LOG_ERROR("close stream error");
        }
        return 0;
}

int DiskMgr::GetDevTotalSize(string DevName, int64_t& TotalSize)
{
        string cmd = "fdisk -l 2>/dev/null|grep \"^Disk " + DevName 
                        + "\"|awk '{print $5}'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("get partition info error");
                return -1;
        }

        char lineBuf[LEN] = {0};
        if (fgets(lineBuf, LEN, stream)) {
                TotalSize = atoll(lineBuf);
        } else {
                TotalSize = 0;
        }

        pclose(stream);
        return 0;	
}

int DiskMgr::GetDevVendor(string DevName, string& Vendor)
{
        if (DevName.empty()) {
                LOG_ERROR("Devname is empty!");
                return -1;
        }

        DevName = DevName.substr(5);                    // /dev/sda -> sda
        string cmd = "kudzu -p 2>/dev/null| grep " + DevName + " -A 1";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("ifconfig errro");
                return -1;
        }


        const int LEN = 128;
        char LineBuf[LEN] = "";
        string line;
        size_t pos = 0;

        while (fgets(LineBuf, LEN, stream)) {
                line = LineBuf;
                line.erase(line.length() - 1);
                if (line.find("desc:") == string::npos) {
                        continue; 
                } else {
                        pos = line.find("\"") + 1;
                        Vendor = line.substr(pos);
                        Vendor.erase(Vendor.length() - 1);
                        break;
                }
        }
        pclose(stream);

        return 0;
}

int DiskMgr::GetVolMap(map<string, string>& DevVolMap)
{
        FILE *stream = popen("lvm pvs 2>/dev/null", "r");
        if (NULL == stream) {
                LOG_ERROR("lvm pvs command error");
                return -1;
        }

        char lineBuf[LEN];
        memset(lineBuf, 0, LEN);
        string line;			//get line of /proc/partitions
        string disk;
        string vol;

        while (fgets(lineBuf, LEN, stream)) {
                line = lineBuf;
                line.erase(line.length() - 1);
                if (line.find("/dev/") != string::npos) {
                        istringstream si(line);
                        si >> disk >> vol;
                        DevVolMap.insert(make_pair(disk, vol));
                        break;
                }
        }	

        pclose(stream);
        if (DevVolMap.size() > 0) {
                return 1;
        }
        return 0;
}

int DiskMgr::GetMapperVolName(vector<MultiPathVol> &mVol, set<string> &AllDevName)
{
        FILE *stream = popen("multipath -l", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        int index = 0;
        char buffer[LEN] = {0};
        MultiPathVol vol;
        while (fgets(buffer, LEN, stream)) {
                char tmp1[LEN] = {0};
                char tmp2[LEN] = {0};

                sscanf(buffer, "%s %*s %s", tmp1, tmp2);
                if (strstr(tmp1, "mpath") == tmp1) {
                        vol.Disks.clear();
                        vol.Name  = "/dev/mapper/";
                        vol.Name += tmp1;
                        vol.Alias = "/dev/mapper/";
                        vol.Alias += tmp2;
                        //cout << endl << vol.Name << "\t" << vol.Alias << endl;

                        if (index == 1) {
                                //string dev = "/dev/" + mVol.back().Disks.front();
                                AllDevName.insert(mVol.back().Disks.front());
                                mVol.pop_back();
                        }
                        mVol.push_back(vol); 
                        index = 0;

                } else if (strstr(tmp2, "sd") == tmp2) {
                        cout << "\t" << tmp2 << endl;
                        string dev = "/dev/";
                        dev += tmp2;
                        mVol.back().Disks.push_back(dev);

                        //cout << "*********" << endl;
                        if (AllDevName.find(dev) != AllDevName.end()) {
                                //cout << "find: " << dev << endl;
                                AllDevName.erase(dev); 
                        }

                        ++index;
                } else {
                        //cout << "continue" << endl;
                        continue; 
                }
        }

        if (index == 1) {
                //string dev = "/dev/" + mVol.back().Disks.front();
                AllDevName.insert(mVol.back().Disks.front());
                mVol.pop_back();
                
                cout << "******************************" << endl;
                //cout << mVol.back().Disks.front() << endl;
                cout << "------------------------------" << endl;
        }

        /*
        cout << "***************** all left dev *************" << endl;
        for (set<string>::iterator it = AllDevName.begin(); 
                                it != AllDevName.end(); ++it) {
                cout << *it << endl; 
        }
        cout << "***************** all dev end *************" << endl;
        */

        pclose(stream);
        return 0;
}

int DiskMgr::GetStorageInfo(map<string, DevInfo>& StorageInfo) 
{	
        int ret;
        set<string> AllDevName;
        ret = GetAllDevName(AllDevName);
        if (ret != 0) {
                LOG_ERROR("get all dev name error"); 
                return -1;
        }

        MultiPathMgr mPath;
        ServiceStatus status;
        ret = mPath.GetServiceStatus(status);
        if (ret != 0) {
                LOG_ERROR("get multipathd status error"); 
                return -2;
        }

        /*
        if (status == RUNNING) {
                vector<MultiPathVol> mVol;
                ret = GetMapperVolName(mVol, AllDevName);
                if (ret != 0) {
                        LOG_ERROR("get mapper volume info error"); 
                        return -3;
                }

                for (vector<MultiPathVol>::iterator it = mVol.begin(); 
                                        it != mVol.end(); ++it) {
                        //cout << it->Name << "\t" << it->Alias << endl;
                        DevInfo DevInformation;
                        ret = GetMapperInfo(*it, DevInformation);
                        if (ret != 0) {
                                LOG_ERROR("get mapper dev info error"); 
                                return 0;
                        }
                        //cout << "get dev info ok " << endl;
                        StorageInfo.insert(make_pair(it->Name, DevInformation));	
                }
        }
        */

        for (set<string>::iterator it = AllDevName.begin(); 
                                it != AllDevName.end(); ++it) {
                //cout << *it << endl;
                DevInfo DevInformation;
                GetDevInfo(*it, DevInformation);
                StorageInfo.insert(make_pair(*it, DevInformation));	
        }
        return 0;
}

int DiskMgr::GetMapperInfo(const MultiPathVol &mVol, DevInfo& DevInformation)
{
        int ret;
        DevInformation.DevName = mVol.Name;
        DevInformation.Alias   = mVol.Alias;
        ret = GetDevVendor(mVol.Disks.front(), DevInformation.Vendor);	
        ret = GetDevTotalSize(mVol.Disks.front(), DevInformation.TotalSize);	

        vector<string> parts;
        ret = GetAllDiskName(mVol.Disks.front(), parts);
        vector<string>::iterator it = parts.begin();
        for (int i = 1; it != parts.end(); ++i, ++it) {
                char buffer[LEN] = {0};
                sprintf(buffer, "%sp%d", mVol.Name.c_str(), i);
                DiskInfo diskInfo;
                diskInfo.DiskName = buffer;
                ret = GetDiskMountPoint(buffer, diskInfo.MountPoint, diskInfo.FileSystem);
                if (1 == ret) {
                        GetDiskUsage(buffer, diskInfo.TotalSize, diskInfo.LeftSize);
                } else if (0 == ret) {
                        GetDiskUsage(*it, diskInfo.TotalSize, diskInfo.LeftSize);
                }
                DevInformation.Disks.push_back(diskInfo);
        }

        return 0;
}

int DiskMgr::GetDevInfo(string DevName, DevInfo& DevInformation)
{
        DevInformation.DevName = DevName;
        GetDevVendor(DevName, DevInformation.Vendor);	
        GetDevTotalSize(DevName, DevInformation.TotalSize);	
        GetDisksInfo(DevName, DevInformation.Disks);

        return 0;
}


int DiskMgr::GetDisksInfo(string DevName, list<DiskInfo>& DisksInfo)
{
        vector<string> AllDisksName; 	
        GetAllDiskName(DevName, AllDisksName);
        for (vector<string>::iterator it = AllDisksName.begin();
                                it != AllDisksName.end(); ++it) {
                DiskInfo DiskInformation;
                DiskInformation.DiskName = *it;
                GetDiskMountPoint(*it, DiskInformation.MountPoint, 
                                        DiskInformation.FileSystem);
                GetDiskUsage(*it, DiskInformation.TotalSize, 
                                        DiskInformation.LeftSize);
                DisksInfo.push_back(DiskInformation);
        }
        return 0;
} 

int DiskMgr::GetDiskMountPoint(string DiskName, string& MountPoint, 
                        string& FileSystem)
{
        //cout << "************************************" << endl;
        //cout << DiskName << endl;
        //cout << "------------------------------------" << endl;
        FILE *stream = popen("mount", "r");
        if (NULL == stream) {
                LOG_ERROR("mount command error");
                return -1;
        }

        map<string, string> DevVolMap;
        if (GetVolMap(DevVolMap) != 0) {                                     
                for (map<string, string>::iterator it = DevVolMap.begin();
                                        it != DevVolMap.end(); ++it) {
                        if (DiskName == it->first) {
                                DiskName = it->second; 
                                break;
                        } 
                }
        }
        char lineBuf[LEN];
        memset(lineBuf, 0, LEN);
        string line;			//get line of /proc/partitions
        string temp;
        int isMount = 0;

        while (fgets(lineBuf, LEN, stream)) {
                line = lineBuf;
                line.erase(line.length() - 1);
                if (line.find(DiskName) != string::npos) {
                        istringstream si(line);
                        si >> temp >> temp >> MountPoint >> temp >> FileSystem;
                        isMount = 1;
                }
        }	
        pclose(stream);
        return isMount;
}

int DiskMgr::GetDiskUsage(string DiskName, int64_t& TotalSize, int64_t& LeftSize)
{
        string Directory;
        string FileSystem;
        int ret = GetDiskMountPoint(DiskName, Directory, FileSystem);
        if (1 == ret) {			//已挂载文件系统
                string partition;
                GetDeviceUsageFromDir(Directory, partition, TotalSize, LeftSize);
        } else if (0 == ret) {          //未挂载文件系统
                GetDiskUsageFromDiskInfo(DiskName, TotalSize, LeftSize);
        } else {
                LOG_ERROR("Get disk mount point error!");	
                return -1;
        }

        return 0;
} 


int DiskMgr::GetPartitionFromDir(const string& path, string& partition)
{
        int64_t totalSize = 0;
        int64_t leftSize  = 0;
        return GetDeviceUsageFromDir(path, partition, totalSize, leftSize);
}

/**
 * 从挂载点获取磁盘使用量
 */
int DiskMgr::GetDeviceUsageFromDir(string Directory, string& Partition, 
                        int64_t& TotalSize, int64_t& LeftSize)
{
        string tDir = "'" + Directory + "'";
        string cmd = "df " + tDir + "|sed '1d;:a;N;s/\\n//;ba'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        int64_t total  = 0;
        int64_t left   = 0;
        char fs[4096]  = {0};

        char*  lineBuf = NULL;
        size_t lineLen;

        if (getline(&lineBuf, &lineLen, stream) != -1) {
                sscanf(lineBuf, "%s %ld %*d %ld %*s %*s", fs, &total, &left); 
        } else {
                LOG_ERROR("get device usage from dir error"); 
                pclose(stream);
                return -2;
        }
        free(lineBuf);

        Partition = fs;
        TotalSize = total * 1024;
        LeftSize  = left  * 1024;

        pclose(stream);
        return 0;
}

/**
 * 直接获取磁盘总空间
 */
int DiskMgr::GetDiskUsageFromDiskInfo(string DiskName, int64_t& TotalSize, 
                        int64_t& LeftSize)
{
        string cmd = "fdisk -l 2>/dev/null|grep " + DiskName + 
                "|awk '{if($2==\"*\"){print$5}else{print$4}}'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                return -1;
        }
        char lineBuf[LEN];
        memset(lineBuf, 0, LEN);
        string line;			//get line of /proc/partitions

        fgets(lineBuf, LEN, stream);
        line = lineBuf;
        if (line.find("+") != string::npos) {
                line.erase(line.find("+"), line.length());
        }
        TotalSize = atoi(line.c_str());
        TotalSize *= 1024;
        LeftSize = TotalSize;
        pclose(stream);

        return 0;	
}

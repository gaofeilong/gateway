#ifndef _DISKMGR_H_ 
#define _DISKMGR_H_

#include <list>
#include <map>
#include <set>
#include <vector>
#include "GatewayBG/Operation/BaseInfo.h"
using std::map;
using std::set;
using std::list;
using std::vector;
using std::string;
class DiskMgr{
public:
	DiskMgr();
	~DiskMgr();
public:
	int GetStorageInfo(map<string, DevInfo>& StorageInfo);
	int GetAllDevName(set<string>& Devs);
	int GetDevInfo(string DevName, DevInfo& DevInformation);
	int GetDevVendor(string DevName, string& Vendor);
	int GetDevTotalSize(string DevName, int64_t& TotalSize);
        int GetVolMap(map<string, string>& DevVolMap);

	int GetDisksInfo(string DevName, list<DiskInfo>& DisksInfo);
	int GetAllDiskName(string DevName, vector<string>& Disks);
	int GetDiskMountPoint(string DiskName, string& MountPoint, string& FileSystem);
	int GetDiskUsage(string DiskName, int64_t& TotalSize, int64_t&LeftSize);
	int GetDeviceUsageFromDir(string MountPoint, string& partition, 
                                int64_t& TotalSize, int64_t& LeftSize);		//从挂载点获取磁盘使用量
        int GetPartitionFromDir(const string& path, string& partition);
	int GetDiskUsageFromDiskInfo(string DiskName, int64_t& TotalSize, int64_t& LeftSize);
        int GetMapperVolName(vector<MultiPathVol> &mVol, set<string> &AllDevName);
        int GetMapperInfo(const MultiPathVol &DevName, DevInfo& DevInformation);
private:
        map<string, DevInfo> m_StorageInfo;
        size_t m_size;
};



#endif //_DISKMGR_H_

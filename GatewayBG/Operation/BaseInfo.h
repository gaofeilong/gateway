#ifndef _BASE_INFO_H_
#define _BASE_INFO_H_
#include <list>
#include <vector>
#include <stdlib.h>

#pragma pack(1)
typedef struct _NetInfo
{
        char IP[16];
        char Mask[16];
}NetInfo;
#pragma pack()

enum Type 
{
        Ext2 = 2,
        Ext3 = 3
};

typedef struct _PortInfo{
        std::string Ip;
        std::string Mask;
        std::string GateWay;
        std::string Vendor;
        std::string Chip;
        std::string Rate;
        int Status;
        char Name[8];
        char Mac[18];
        char Alias[128];

} PortInfo;

typedef struct _BondInfo{
        std::string Name;
        std::string Alias;
        std::string Ip;
        std::string Mask;
        std::string Mac;
        int Status;
        std::string GateWay;
        std::list<PortInfo> PortInfoList;
} BondInfo;

typedef struct _DiskInfo{
	std::string DiskName;
	std::string MountPoint;
        std::string FileSystem;
	int64_t TotalSize;
	int64_t LeftSize;
} DiskInfo;

typedef struct _DevInfo{
        std::string DevName;
	std::string Vendor;
        std::string Alias;
	int64_t TotalSize;
	std::list<DiskInfo> Disks;
} DevInfo;

typedef struct _MultiPathVol {
        std::string Name;
        std::string Alias; 
        std::vector<std::string> Disks;
} MultiPathVol;

typedef struct _NFSCfgInfo{
        std::string SharedDir; 
        std::string SharedIp; 
        std::string Permission; 
} NfsCfgInfo;

typedef struct _VersionInfo{
        std::string KVersion;
        std::string Deadline;
        std::string TotalSize;
        std::string UsedSize;
} VersionInfo;

typedef enum _ServiceStatus{
        STOPPED,
        RUNNING
} ServiceStatus;

#endif

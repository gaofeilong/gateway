/*
#include <stdio.h>
#include <list>
#include <iostream>
#include <string>
#include <limits.h>
#include <unistd.h>

#include "Archive/DataArchive/DDFSArchive.h"
#include "Archive//Md5Copy/Md5Copy.h"
#include "GatewayBG/Operation/BaseOpr.h"
#include "Utils/Log/Log.h"

using namespace std;

int main(int argc, char* argv[])
{
	BaseOpr bo;
//get cpu info
	string cpuInfo;
	bo.GetCPUInfo(cpuInfo);
	cout << cpuInfo << endl;
//cpu usage	
	float f;
	cout << "before: " << f << endl;
	bo.GetCpuRatio(f);
	cout << "after: " << f << endl;
//make dir
	bo.MkDir("/tmp/gaofeig/");
//set host name
        string name = "localhost";
	bo.SetMachineName(name);
//set net
	NetInfo ni = {"192.168.0.228", "255.255.254.0"};
	bo.SetNet("eth0", &ni);
//set ip
        cout << "set ip 0" << endl;
	bo.SetIp("eth0", "192.168.0.229");			
        cout << "set ip 1" << endl;
//set mask
        cout << "set mask 0" << endl;
	bo.SetMask("eth0", "255.255.255.0");
        cout << "set mask 1" << endl;
//delete a partition
	bo.DeletePartition("/dev/sdc1");
//find no format disk
	list<string> listStr; 
	bo.GetDiskDriver(listStr);
	for(list<string>::iterator it = listStr.begin(); it != listStr.end(); ++it) {
		cout << *it << "未分区" << endl;	
	}
//mkfs
	string dev = "/dev/sdc1";
	string fstype = "ext3";
	if (0 != bo.Mkfs(dev, fstype)) {
		cout << "make file system error" << endl;
	}
//mount
	string mp = "/TestMP";
	string dev = "/dev/sdc1";
	if (0 != bo.Mount(dev, mp)) {
		cout << "mount point error" << endl;
	}
//noformat dist
        list<string> slist;
        bo.GetDiskDriver(slist);
        cout << slist.size() << endl;
        return 0;
//get san info
	map<string, DevInfo> info;
	bo.GetStorageInfo(info);	
	for (map<string, DevInfo>::iterator it = info.begin(); it != info.end(); ++it) {
		cout << it->second.DevName << '\t' << it->second.Vendor << '\t' 
			<< fixed << it->second.TotalSize / 1073741824 << endl;
		for (list<DiskInfo>::iterator iter = it->second.Disks.begin();
					iter != it->second.Disks.end(); ++iter) {
			cout << "\tname: " << iter->DiskName << endl;;	
			cout << "\ttotal on disk: " << fixed << iter->TotalSize << endl;	
			cout << "\tleft on disk: " << fixed << iter->LeftSize << endl;	
			cout << "\tmount point: " << iter->MountPoint << endl;
			cout << "\tfile system: " << iter->FileSystem << endl;
		}
	}
	sleep(1);
	cout << endl;
//get mem usage
	int64_t a, b;
	bo.GetMemUsage(a, b);
	cout << a << endl << b << endl;
//bond & unbond
	if (argc == 2) {
		if (*argv[1] == 'i') {
			vector<string> PortNames;
			PortNames.push_back("eth3");
			PortNames.push_back("eth4");
			if (0 != bo.Bond("bond0", PortNames)) {
				cout << "bond failed" << endl;
				return -2;
			}
                        cout << "bonding success" << endl;
		} else if (*argv[1] == 'u') {
			if (0 != bo.UnBond("bond0")) {
				cout << "unbond failed" << endl;
				return -2;
			}
                        cout << "unbonding success" << endl;
		} else {
			cout << "unknown parament: " << *argv[1] << endl;	
			return -3;
		}
	} else {
                cout << "invalid parament" << endl; 
        }
//get bond info	
	while(1) {
		time_t t = time(0);
		struct tm *now = localtime(&t);
		cout << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec 
			<< "---------------------------------------------------" << endl;
		 
		BondInfo bi;
		bo.GetBondInfo("bond0", bi);
		cout << "name: " << bi.Name << endl;
		cout << "alias: " << bi.Alias << endl;
		cout << "mac: " << bi.Mac << endl;
		cout << "ip: " << bi.Ip << endl;
		cout << "mask: " << bi.Mask << endl;
		cout << "gateway: " << bi.GateWay << endl;
		cout << "status: " << bi.Status << endl;
		cout << "********************************" << endl;
		for (list<PortInfo>::iterator it = bi.PortInfoList.begin();
			it != bi.PortInfoList.end(); ++it) {
			cout << "name: " << it->Name << endl;
			cout << "alias: " << it->Alias << endl;
			cout << "vendor: " << it->Vendor << endl;
			cout << "chip: " << it->Chip << endl;
			cout << "rate: " << it->Rate << endl;
			cout << "ip: " << it->Ip << endl;
			cout << "mac: " << it->Mac << endl;
			cout << "mask: " << it->Mask << endl;
			cout << "gateway: " << it->GateWay << endl;
			cout << "status: " << it->Status << endl;
		}

//get all port info
		list<PortInfo>  pi;
		bo.GetAllPortsInfo(pi);
		for (list<PortInfo>::iterator it = pi.begin(); it != pi.end(); ++it) {
			cout << "name: " << it->Name << endl;
			cout << "alias: " << it->Alias << endl;
			cout << "vendor: " << it->Vendor << endl;
			cout << "chip: " << it->Chip << endl;
			cout << "rate: " << it->Rate << endl;
			cout << "ip: " << it->Ip << endl;
			cout << "mask: " << it->Mask << endl;
			cout << "mac: " << it->Mac << endl;
			cout << "gateway: " << it->GateWay << endl;
			cout << "status: " << it->Status << endl;
			cout << endl;
		}
		sleep(1);
	}
//get port rate
        NetMgr nm;           
        string Rate;
        nm.GetPortRate("eth3", Rate);
        cout << Rate << endl;
//get port info
	list<PortInfo>  pi;
	bo.GetFreePortInfo(pi);
	for (list<PortInfo>::iterator it = pi.begin(); it != pi.end(); ++it) {
		cout << "name: " << it->Name << endl;
		cout << "alias: " << it->Alias << endl;
		cout << "vendor: " << it->Vendor << endl;
		cout << "chip: " << it->Chip << endl;
		cout << "rate: " << it->Rate << endl;
		cout << "ip: " << it->Ip << endl;
		cout << "mask: " << it->Mask << endl;
		cout << "mac: " << it->Mac << endl;
		cout << "gateway: " << it->GateWay << endl;
		cout << endl;
	}
        NetMgr nm;           
	if (0 != nm.DelBondNetFile("bond0")){
		cout << "bad" << endl;	
	} else {
		cout << "good" << endl;	
	}
        */
//校验
        /*
	if (argc != 3) {
		cout << "invalid paraments" << endl
                     << "argv[1]=md5LogPath" << endl
                     << "argv[2]=moveLogPath" << endl;
		return -1;
	}
        Md5Copy m5c;
        m5c.SetMaxRecord(30);
        m5c.SetMd5Path(argv[1]);
        m5c.SetMvPath(argv[2]);
        
        m5c.Check();
        return 0;
}
*/

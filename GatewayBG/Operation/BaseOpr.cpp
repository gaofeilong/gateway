#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <fstream>
#include <map>

#include "GatewayBG/Operation/BaseOpr.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"
#include "Utils/Log/Log.h"

using namespace std;

const int LEN = 128;

BaseOpr::BaseOpr()
{
}

BaseOpr::~BaseOpr()
{
}

int BaseOpr::ReBoot()
{
        if(0 != system("reboot")) {
                LOG_ERROR("reboot error"); 
                return -1;
        }
        return 0;
}

int BaseOpr::ShutDown()
{
        if(0 != system("init 0")) {
                LOG_ERROR("shutdown error"); 
                return -1;
        }
        return 0;
}

int BaseOpr::GetCpuCoreNum(int &num)
{
        FILE *m_stream = fopen("/proc/stat", "r");
        if (NULL == m_stream) {
                return -1;
        }

        char buf[LEN] = "";
        string line;	

        while(fgets(buf, LEN, m_stream)) {
                line = buf;
                string cpuinfo;
                if (line.find("cpu", 0) == 0) {
                        cpuinfo	= line.erase(line.find(' ', 0), line.length()); //删除除CPU后数字以外的部分,只保留数字
                        cpuinfo = cpuinfo.erase(0, 3);
                        num = atoi(cpuinfo.c_str());
                }
        }
        ++num;
        fclose(m_stream);

        return 0;
}

int BaseOpr::EraseSpace(string &str)
{
        string temp;
        for (size_t i = 0; i < str.length(); ++i) {
                if (0 == i) {
                        temp +=str.at(i);
                } else {
                        if (isspace(str.at(i)) && isspace(str.at(i - 1))) {
                                continue;
                        } else {
                                temp +=str.at(i);
                        }
                }
        }
        str = temp;
        return 0;
}

int BaseOpr::GetCPUInfo(string& cpuInfo)
{
        FILE *m_stream = fopen("/proc/cpuinfo", "r");
        if (NULL == m_stream) {
                return -1;
        }

        char buf[LEN] = "";
        string line;
        string vendor;
        string model;
        string freq;

        while (fgets(buf, LEN, m_stream)) {
                line = buf;
                if (line.find("vendor_id", 0) == 0) {				//制造商
                        int pos = line.find(":", 0);
                        line.erase(0, pos + 2);
                        line .erase(line.length()-1, line.length());
                        vendor = line;
                }
                if (line.find("model name", 0) == 0) {				//型号
                        int pos = line.find(":", 0);
                        line.erase(0, pos + 2);
                        line .erase(line.length()-1, line.length());
                        model = line;
                        EraseSpace(model);
                }
                if (line.find("cpu MHz", 0) == 0) {				//主频
                        int pos = line.find(":", 0);
                        line.erase(0, pos + 2);
                        line .erase(line.length()-1, line.length());
                        freq = line;
                }
        }
        fclose(m_stream);
        //序列化
        //vendor	
        cpuInfo += "Vender: ";
        cpuInfo += vendor;

        //model:
        cpuInfo += "\tModel: ";
        cpuInfo += model;

        //cpu core number
        int cpuNum;
        if (0 != GetCpuCoreNum(cpuNum)) {
                return -1;
        }
        cpuInfo += "\tCoreNum: ";
        char str[LEN];
        memset(str, 0, LEN);
        sprintf(str, "%d", cpuNum);
        cpuInfo += str;

        //processor frequency
        cpuInfo += "\tFrequency: ";
        cpuInfo += freq;

        return 0;
}

int BaseOpr::GetCpuRatio(float& ratio)
{
	FILE *stream = popen("top -d 0.1 -n 2 -b", "r");
	if (NULL == stream) {
		LOG_ERROR("open cmd error at function GetCpuRatio()");
		return -1;
	}

	char lineBuf[LEN];
	memset(lineBuf, 0, LEN);
	string strRatio;
	string line;			//get line of /proc/partitions

	while (fgets(lineBuf, LEN, stream)) {
		line = lineBuf;
		if (line.find("Cpu") != string::npos) {
			istringstream si(line);
			for (int i = 0; i < 2; ++i) {
				si >> strRatio;	
			}
			string str;
			str = strRatio.substr(0, strRatio.find("%"));
			ratio = atof(str.c_str());
		}	
        }	
	if ( pclose(stream) < 0 ) {
                LOG_ERROR("pclose error! errInfo: " << strerror(errno));
        }
	return 0;	
}

int BaseOpr::GetMemUsage(int64_t& total, int64_t& used)
{
        FILE* file = popen("free -b | sed -n 3p | awk {'print $3 \" \" $4'}", "r");
        if (file == NULL) {
                LOG_ERROR("popen error! errinfo=" << strerror(errno));
                return -1;
        } 

        char *buffer = NULL;
        size_t len = 0;
        if (getline(&buffer, &len, file) < -1) {
                LOG_ERROR("getline error! errinfo: " << strerror(errno));
                return -2;
        }

        int64_t freeSize = 0;
        sscanf(buffer, "%ld %ld", &used, &freeSize);
        total = freeSize + used;
        
        if (buffer != NULL) {
                free(buffer);
        }

        if (pclose(file) < 0) {
                LOG_ERROR("pclose error! errInfo: " << strerror(errno));
        }
	return 0;
}

int BaseOpr::Kill(int pid)
{
        if (pid > 0) {
                kill(pid, SIGKILL);
                return 0;
        } else {
                return -1;
        }
}

int BaseOpr::GetDiskDriver(std::list<std::string>& devs)
{
	GetHDiskDriver(devs);
	GetSDiskDriver(devs);

        return 0;
}

int BaseOpr::Mkfs(const string &dev, const string &type)
{
        int ret = 0;
        string cmd = "mkfs.";
        cmd += type;
        cmd += " ";
        cmd += dev;

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
        }
        return ret;
}

int BaseOpr::Mount(const std::string &dev, const std::string &mp)
{
        int ret = 0;

        string cmd = "mount ";
        cmd += dev;
        cmd += " ";
        cmd += mp;

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd Error!");
        }
        return ret;
}

int BaseOpr::UnMount(const std::string &mp)
{
        int ret = 0;
        string cmd = "umount ";
        cmd += mp;

        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret < 0) {
                LOG_ERROR("ExecuteCmd " << cmd << " Error!");
        }
        return ret;
}


int BaseOpr::DeletePartition(const string &partition)
{
        return 0;
}

int BaseOpr::GetHDiskDriver(std::list<std::string>& devs)
{
        return GetDriver(devs, 'h');
}

int BaseOpr::GetSDiskDriver(std::list<std::string>& devs)
{
        return GetDriver(devs, 's');
}

int BaseOpr::GetDriver(std::list<std::string>& devs, char type)
{
        char path[] = "/proc/partitions";
        ifstream fin(path);
        if (!fin) {
                LOG_ERROR("open " << path << "error");
                return -1;
        }

        string driver;
        string substr;
        map<string, char> mp; //查找到的字符串存放到mp中

        while (getline(fin, driver)) {
                int strSize = driver.size();
                if (strSize < 4) {
                        continue;
                }
                substr = driver.substr(strSize-4, strSize-1);   
                int subSize = substr.size();
                if (substr[0] == ' ') {
                        substr = substr.substr(subSize-3, subSize-1);
                }
                mp.insert(make_pair(substr, 'a'));
        }

        char buffer[10];
        char partBuf[10];
        char a = 'a';
        sprintf(buffer, "%cd%c", type, a);

        for (map<string, char>::iterator beg=mp.begin(); beg!=mp.end(); ++beg) {
                map<string, char>::iterator strIter = mp.find(buffer);
                if (strIter == mp.end()) {
                        //cout << "没有发现硬盘" << endl;
                        break;
                }

                for (int j=1; j<=4; ++j) { 
                        sprintf(partBuf, "%s%d", buffer, j);
                        map<string, char>::iterator fii = mp.find(partBuf);
                        if (fii != mp.end()) {
                                //cout << buffer << " 已经分区" << endl;
                                break;
                        } 
                        if (j == 4) {
				//cout << buffer << " 没有分区" << endl;
                                devs.push_back(buffer);
                        }
                }
                sprintf(buffer, "%cd%c", type, ++a);
        }

        fin.close();

        return 0;
}

int BaseOpr::FindDev(const string &dev)
{
	FILE *stream = fopen("/proc/partitions", "r");
	if (NULL == stream) {
		cout << "open file error" << endl;
		return -1;
	}
	
	char lineBuf[LEN];
	memset(lineBuf, 0, LEN);
	string d = dev.substr(5);
	string line;

	while (fgets(lineBuf, LEN, stream)) {
		line = lineBuf;
		if (line.find(d) != string::npos) {
			return 1;	
		}	
	}
	fclose (stream);
	return 0;
}

int BaseOpr::GetPParCnt(const string &dev)
{
	if (FindDev(dev) != 1) {
		cout << "cannot find device" << endl;
		return -2;
	}	
	FILE *stream = fopen("/proc/partitions", "r");
	if (NULL == stream) {
		cout << "open file error" << endl;
		return -1;
	}
	
	char lineBuf[LEN];
	memset(lineBuf, 0, LEN);
	string d = dev.substr(5);
	int pCnt = 0;                   //p disk count
	int baseMinor = 0;              //device minor num
	int major = 0;                  //major
	int minor = 0;                  //disk minor number
	int64_t blocks = 0;             //block count
	string diskName;                //name
	string line;                    //get line of /proc/partitions

	while (fgets(lineBuf, LEN, stream)) {
		line = lineBuf;
		line.erase(line.length() - 1);
		size_t pos = line.find(d);
		if (pos != string::npos) {
			istringstream si(line);
			si >> major >> minor >> blocks >> diskName;
			if (line.substr(pos) == d) {
				baseMinor = minor;
			} else {
				if (blocks != 1 && minor <= baseMinor + 4) {
					pCnt++;	
				}	
			}
		}		
	}	
	fclose(stream);
	return pCnt;
}

int BaseOpr::GetParNum(const string &dev)
{
	if (FindDev(dev) != 1) {
		cout << "cannot find device" << endl;
		return -2;
	}	
	int cnt = GetPParCnt(dev);
	if (cnt < 0 || cnt > 3) {
		cout << "GetPParCnt() <0 or >3" << endl;
		return -3;
	}
	if (GetPParCnt(dev) == 0) {
		return 1;	
	}
	
	FILE *stream = fopen("/proc/partitions", "r");
	if (NULL == stream) {
		cout << "open file error" << endl;
		return -1;
	}
	
	char lineBuf[LEN] = "";
	string d = dev.substr(5);
	int major;			//major
	int minor;			//disk minor number
	int64_t blocks;			//block count
	string diskName;		//name
	string line;			//get line of /proc/partitions
	int index = 0;


	while (fgets(lineBuf, LEN, stream)) {
		line = lineBuf;
		line.erase(line.length() - 1);
		size_t pos;
		if (((pos = line.find(d)) != string::npos) && (line.substr(pos) != d)) {
			++index;
			istringstream si(line);
			si >> major >> minor >> blocks >> diskName;
			string diskIndex = diskName.substr(3);
			if (atoi(diskIndex.c_str()) != index) {			//有断层的情况
				return index;
			}	
		}		
	}	
	if (index > 0) {
		return ++index;	
	} 
	fclose(stream);
	return -4;
}

int BaseOpr::Partition(const string &dev, int64_t size)
{
	if (FindDev(dev) != 1) {
		cout << "cannot find device" << endl;
		return -1;
	}	
	if (GetPParCnt(dev) >= 4) {
		cout << "4 primary disk" << endl;
		return -2;
	}
	char strSz[8] = "";
	sprintf(strSz, "%c%ld%c", '+', size, 'M');
	int n = GetParNum(dev);
	char cmd[LEN] = "";
	sprintf(cmd, "%s %s %d %s", "sh Partition.sh", dev.c_str(), n, strSz);
	if (0 != system(cmd)) {
		cout << "system(cmd) error" << endl;
		return -3;
	}


	return 0;
}

int BaseOpr::DefPartition(const std::string &dev)
{
	int n = GetParNum(dev);
	if (GetPParCnt(dev) >= 4) {
		cout << "4 primary disks, cannot make one any more" << endl;
		return -1;
	}
	char cmd[LEN] = "";
	sprintf(cmd, "%s %s %d", "sh defPartition.sh ", dev.c_str(), n);
	if (0 != system(cmd)) {
		cout << "system(cmd) error" << endl;
		return -2;
	}
	
	return 0;
}

int BaseOpr::GetDiskFormatInfo(const std::string &dev, double &total)
{
	FILE *stream = fopen("/proc/partitions", "r");
	if (NULL == stream) {
		cout << "open file error" << endl;
		return -1;
	}
	
	char lineBuf[LEN];
	memset(lineBuf, 0, LEN);
	string d = dev.substr(5);
	int major;			//major
	int minor;			//disk minor number
	int64_t blocks;			//block count
	string diskName;		//name
	string line;			//get line of /proc/partitions
	const int BlockPerG = 1024*1024;

	while (fgets(lineBuf, LEN, stream)) {
		line = lineBuf;
		line.erase(line.length() - 1);
		size_t pos;
		if ((pos = line.find(d)) != string::npos) {
			istringstream si(line);
			si >> major >> minor >> blocks >> diskName;
			if (line.substr(pos) == d) {
				total = blocks / BlockPerG;
			} 
		//	else {
		//		totalBlocks += blocks;
		//	}
		}		
	}	
	//left = totalBlocks / BlockPerG;
	fclose(stream);
	return 0;
}

int BaseOpr::GetMachineName(string& MachineName)
{
	FILE *stream = popen("cat /etc/sysconfig/network|grep HOSTNAME|awk -F \"[.=]\" '{print$2}'", "r");
	if (NULL == stream) {
		LOG_ERROR("get hostname error");
		return -1;
	}
        char *name = NULL;
        size_t len = 0;
        if (getline(&name, &len, stream) != -1) {
                MachineName = name;
                MachineName.erase(MachineName.find('\n'));
        }
        
        if (name != NULL) {
                free(name);
        }
        
        if ( pclose(stream) < 0 ) {
                LOG_ERROR("pclose error! errInfo: " << strerror(errno));
        }
        return 0;
}

int BaseOpr::GetGateWay(string PortName, string& GateWay)
{
        return m_NetMgr.GetGateWay(PortName, GateWay);
}

int BaseOpr::GetIp(string PortName, string& Ip )
{
        return m_NetMgr.GetIp(PortName, Ip);
}

int BaseOpr::GetDns(string& dns)
{
        return m_NetMgr.GetDns(dns);
}

bool BaseOpr::IsBonded()
{
        return m_NetMgr.IsBonded(); 
}

int BaseOpr::SetDns(const string& dns)
{
        return m_NetMgr.SetDns(dns);
}

int BaseOpr::ClearDns()
{
        return m_NetMgr.ClearDns(); 
}

int BaseOpr::ClearGateway() 
{
        return m_NetMgr.ClearGateway(); 
}

int BaseOpr::GetMask(string PortName, string& Mask)
{
        return m_NetMgr.GetMask(PortName, Mask);
}

int BaseOpr::SetMachineName(string MachineName)
{
        return m_NetMgr.SetMachineName(MachineName);
}

int BaseOpr::SetIp(string PortName, string Ip)
{
        return m_NetMgr.SetIp(PortName, Ip);
}

int BaseOpr::SetMask(string PortName, string Mask)
{
        return m_NetMgr.SetMask(PortName, Mask);
}

int BaseOpr::SetNet(string PortName, NetInfo* NetInformation)
{
        return m_NetMgr.SetNet(PortName, NetInformation);
}

int BaseOpr::GetLeftSpaceByMp(string mp, int64_t& space, int64_t& left)
{
        string partition;
        m_DiskMgr.GetDeviceUsageFromDir(mp, partition, space, left);
        return 0;
}
int BaseOpr::GetStorageInfo(map<string, DevInfo>& StorageInfo)
{
	return m_DiskMgr.GetStorageInfo(StorageInfo);
}

int BaseOpr::GetPortInfo(string PortName, PortInfo& PortInformation)   //ip, mask, gateway, mac
{
        return  m_NetMgr.GetPortInfo(PortName, PortInformation);
}

int BaseOpr::Bond(vector<string> Eths)
{
        return m_NetMgr.Bond(Eths);
}

int BaseOpr::UnBond(string BondName)
{
        return m_NetMgr.UnBond(BondName);
}

int BaseOpr::GetAllPortsInfo(list<PortInfo>& PortsInfo)
{
	return m_NetMgr.GetAllPortsInfo(PortsInfo);
}

int BaseOpr::GetFreePortInfo(list<PortInfo>& PortsInfo)
{
        return m_NetMgr.GetFreePortInfo(PortsInfo);
}

int BaseOpr::GetBondInfo(string BondName, BondInfo& BondInformation)
{
        return m_NetMgr.GetBondInfo(BondName, BondInformation);
}

int BaseOpr::GetAllBondInfo(list<BondInfo>& BondInfoList)
{
        return m_NetMgr.GetAllBondInfo(BondInfoList);
}

int BaseOpr::SetGateWay(string PortName, string Ip)
{
        return m_NetMgr.SetGateWay(PortName, Ip);
}

int BaseOpr::GetNetworkId(string& networkId)
{
        return m_NetMgr.GetNetworkId(networkId);
}

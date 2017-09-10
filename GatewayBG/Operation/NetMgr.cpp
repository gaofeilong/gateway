#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#include "Config/IniParser.h"
#include "GatewayBG/Operation/NetMgr.h"
#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/ChildProcessOpr.h"

using namespace std;
const string DNS_CONFIG_FILE = "/var/named/chroot/var/named/scidata.com.zone";
const string BOND_CONFIG_DIR = "/etc/sysconfig/network-scripts/";

NetMgr::NetMgr()
{
}

NetMgr::~NetMgr()
{
}

int NetMgr::GetAvailBond(string &bond)
{
        FILE *stream = fopen("/sys/class/net/bonding_masters", "r");
        if (NULL == stream) {
                if (errno == 2) {               //no such file
                        bond = "bond0"; 
                        return 0;
                } 
                LOG_ERROR("open stream error");
                return -1;
        }

        char buf[1024] = {0};
        if (fgets(buf, 1024, stream)) {
                for (int i = 0; ; ++i) {
                        char bondName[8] = {0};
                        sprintf(bondName, "%s%d", "bond", i);
                        if (strstr(buf, bondName) == NULL) {
                                bond = bondName;
                                break;
                        }
                }
        }

        if (fclose(stream) != 0) {
                LOG_INFO("close stream error");
        }
        return 0;
}

int NetMgr::GetPortName(vector<string>& ports)
{
        FILE *stream = popen("ifconfig|grep \"^eth\"|awk '{print$1}'", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                ports.clear();
                return -1;
        }

        string port;
        size_t bufLen  = 0;
        char*  lineBuf = NULL;

        while (getline(&lineBuf, &bufLen, stream) != -1) {
                port = lineBuf;
                port = port.substr(0, port.find('\n'));
                ports.push_back(port);
        }
        free(lineBuf);

	if (0 != pclose(stream)) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int NetMgr::GetBondName(vector<string>& bonds)
{
        FILE *stream = popen("ifconfig|grep \"^bond\"|awk '{print$1}'", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                bonds.clear();
                return -1;
        }

        string line;
        size_t lineLen  = 0;
        char*  lineBuf = NULL;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;
                line.erase(line.find('\n')); 
                bonds.push_back(line);
                //LOG_INFO(line << "-------------in GetBondName");
        }
        free(lineBuf);

	if (0 != pclose(stream)) {
                LOG_ERROR("close stream error"); 
        }
        return 0;

}

int NetMgr::GetPortAlias(const string& port, char* alias)
{
        strcpy(alias, "*****");
        return 0;
}

int NetMgr::GetPortRate(const string& port, string& rate)
{
        //string cmd = "dmesg|grep \"" + port + "\"|grep \"Mbps\"|awk '{print $7}'|sort -r|sed -n \"1p\"";
        string cmd = "ethtool " + port + "|awk -F \"[a-z]\" '{print$1}'|sort -r|awk '{print$1}'|sed -n '1p'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        char*  rateBuf = NULL;
        size_t bufLen  = 0;
        if (getline(&rateBuf, &bufLen, stream) != -1) {
                rate = rateBuf;
                rate = rate.substr(0, rate.find('\n'));
                rate += "Mbps";
        } else {
                rate = "unknown rate"; 
        }
        free(rateBuf);

        if (pclose(stream) != 0) {
                LOG_ERROR("close stream error");
                return -2;
        }                                                                           
        return 0;
}

int NetMgr::GetPortVendorAndChip(const string& port, string& vendor, string& chip)                
{
        string cmd = "kudzu -p|grep " + port + " -A2|grep \"^desc\"|awk -F \"\\\"\" '{print $2}'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream errro");
                return -1;
        }

        size_t bufLen  = 0;
        char*  vendorBuf = NULL;
        string vc;

        if (getline(&vendorBuf, &bufLen, stream) != -1) {
                vc = vendorBuf;
        }
        free(vendorBuf);

        if (!vc.empty()) {
                size_t pos = vc.find("Corporation") + strlen("Corporation");
                chip   = vc.substr(pos + 1);
                vendor = vc.substr(0, pos);
        }

	if (pclose(stream) != 0) {
                LOG_ERROR("close stream error");
        }
        return 0;
}

int NetMgr::GetPortInfo(string PortName, PortInfo& PortInformation)                                     
{
	string cmd = "ifconfig " + PortName;
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR( "ifconfig " << PortName << " not found.");
                return -1;
        }

        const int LEN = 128;
        char LineBuf[LEN];
        string line;
        int bond_sig = 0;

	strcpy(PortInformation.Name, PortName.c_str());

        GetGateWay(PortName, PortInformation.GateWay);
        GetIp(PortName, PortInformation.Ip);
        GetMask(PortName, PortInformation.Mask);
        GetPortRate(PortName, PortInformation.Rate);
        /* 2011-12-22, by gfl, 不要获取厂商和芯片信息 */
        /*GetPortVendorAndChip(PortName, PortInformation.Vendor, PortInformation.Chip);
        */
        GetPortStatus(PortName, PortInformation.Status);

        while (fgets(LineBuf, LEN, stream)) {
                line = LineBuf;
                line.erase(line.length() - 1);
		size_t pos = 0;
                if ((pos = line.find("HWaddr")) != string::npos) {
			string mac = line.substr(line.find("HWaddr") + 7, 18);
			strcpy(PortInformation.Mac, mac.c_str());
			PortInformation.Mac[17] = '\0';
		} else if (line.find("SLAVE") != string::npos) {
                        bond_sig = 1;
                }
        }

        GetPortAlias(PortName, PortInformation.Alias);

	int ret;

        if (bond_sig == 1) {
                PortInformation.Chip.clear();
                PortInformation.GateWay.clear();
                PortInformation.Ip.clear();
                memset(PortInformation.Mac, 0, 18);
                PortInformation.Mask.clear();
                PortInformation.Rate.clear();
                PortInformation.Vendor.clear();
        }
        
	if (0 != (ret = pclose(stream))) {
                LOG_ERROR("close stream error");
		return ret;
        }
        return 0;
}

int NetMgr::GetPortStatus(const string& port, int& status)
{
	string cmd = "ethtool " + port + "|sed -n '$p'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                status = 0;
                return -1;
        }

        char*  lineBuf = NULL;
        size_t lineLen = 0;
        string line;

        if (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;
                status = (line.find("yes") != string::npos)? 1: 0;
        }
        free(lineBuf);

        if (pclose(stream) != 0) {
                LOG_ERROR("close stream error");
                return -2;
        }

        return 0;
}

int NetMgr::GetFreePortInfo(list<PortInfo>& portsInfo)
{
        portsInfo.clear();
        list<PortInfo> infoList;
        GetAllPortsInfo(infoList);
        list<PortInfo>::iterator it = infoList.begin();
        for ( ; it != infoList.end(); ++it) {
                if (it->Status == 0) {
                        portsInfo.push_back(*it); 
                }
        }
        return 0;
}

int NetMgr::GetAllPortsInfo(list<PortInfo>& portsInfo)
{
        portsInfo.clear();
	vector<string> ports;
	PortInfo portInfo;
	if (GetPortName(ports) != 0) {
                LOG_ERROR("Get port name error");
                return -1;
	}

	vector<string>::iterator it = ports.begin();
	for ( ;	it != ports.end(); ++it) {
		if (GetPortInfo(*it, portInfo) != 0) {
			LOG_ERROR("Get port info error");
                        return -2;
		}
		portsInfo.push_back(portInfo);
	}

	return 0;
}

int NetMgr::Bond(vector<string> Eths)
{
	int ret;
	string bond;
        ret = GetAvailBond(bond);
        if (ret != 0) {
                LOG_ERROR("get avail bond name error"); 
                return -1;
        }
        if (0 != (ret = Config(bond))) {			// /etc/modprobe.conf
                LOG_ERROR("config error");
                return ret;
        }

        for (vector<string>::iterator it = Eths.begin(); it != Eths.end(); ++it) {
                if (0 != (ret = UpdateEthNetFile(bond, *it, 1))) {		//ifcfg-ehtN
                        LOG_ERROR("eth config error");
                        return ret;
                }
        }
        string NetFileName = "/etc/sysconfig/network-scripts/ifcfg-" + bond;
        if (access(NetFileName.c_str(), F_OK) != 0) {
                string ip;
                string gateway;
                string netmask;
                GetIp(Eths.front(), ip);
                GetMask(Eths.front(), netmask);
                GetGateWay(Eths.front(), gateway);
                if(0 != (ret = AddBondNetFile(bond, ip, gateway, netmask))) { //ifcfg-bond0
                        LOG_ERROR("Add bond net file error");
			return ret;
                }
        }

        ChildProcessOpr cmdOpr;
        string cmd = "modprobe " + bond + ";service network restart";
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR("add bonding kernel module error");
		return ret;
        }

        /*
        for (vector<string>::iterator it = Eths.begin(); it != Eths.end(); ++it) {
                cmd = "ifdown " + *it + "; ifup " + *it;
		if (cmdOpr.ExecuteCmd(cmd) != 0) {
			LOG_ERROR("bonding: restart " << *it << " error");	
			return ret;
		}
        }

	cmd = "ifup " + bond + " > /dev/null";
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
		LOG_ERROR("restart " << bond << " error");	
		return ret;
	}
        */

	LOG_INFO("bond success");
        return 0;
}

int NetMgr::UnBond(string bond)
{
	int ret = 0;
        string cmd;
        vector<string> Eths;
        vector<string> bonds;
        
        cout << "unbonding..." << endl; 
        if (GetBondName(bonds) != 0) {
                LOG_ERROR("get bond name error"); 
                return ret;
        }

        if (bonds.size() == 1) {
                cmd = "rmmod bonding;service network restart";  
        } else {
                cmd = "modprobe -r " + bond + ";service network restart";
        }

        if (0 != (ret = GetPortOfBond(bond, Eths))) {
                LOG_ERROR("Get eth name of " << bond << " error!"); 
                return ret;
        }

        if (0 != (ret = DelBondNetFile(bond))) {
                LOG_ERROR("delete bond config file error"); 
                return ret;
        }

        for (vector<string>::iterator it = Eths.begin(); it != Eths.end(); ++it) {
                if (0 != (ret = UpdateEthNetFile(bond, *it, 0))) {
                        LOG_ERROR("delete bond information from ehts config file error!");
                        return ret;
                }
        }

        //cout << cmd << endl;
        ChildProcessOpr cmdOpr;
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR(cmd << " error!"); 
                return ret;
        }

        LOG_INFO("unbond success");
        return 0;
}

int NetMgr::Config(string BondName)
{
        string Alias = "alias " + BondName + " bonding";  
        FILE *stream = fopen("/etc/modprobe.conf", "r");
        if (NULL == stream) {
                LOG_ERROR("open eth config file error");
                return -1;
        }

        const int LEN = 128;
        char LineBuf[LEN];
        string line;
        vector<string> File;
        int pos;
        while(fgets(LineBuf, LEN, stream)) {
                line = LineBuf;
                if (0 == (pos = line.find(Alias))) {
                        return 0;	
                } 
        }

        int ret;
        ChildProcessOpr cmdOpr;
        string cmd = "echo " + Alias + " >> /etc/modprobe.conf";
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR("add bond alias to /etc/modprobe.conf error"); 
                return -2;
        }
        if (0 != (ret = fclose(stream))) {
                LOG_ERROR("close stream error");
                return ret;
        }
        return 0;
}

int NetMgr::AddBondNetFile(const string& bond, const string& ip, 
                        const string& gateway, const string& netmask)
{
        const string fileName = "/etc/sysconfig/network-scripts/ifcfg-" + bond; 
        FILE* stream = fopen(fileName.c_str(), "w");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }
        fprintf(stream, "DEVICE=%s\n", bond.c_str());
        fprintf(stream, "IPADDR=%s\n", ip.c_str());
        fprintf(stream, "GATEWAY=%s\n", gateway.c_str());
        fprintf(stream, "NETMASK=%s\n", netmask.c_str());
        fprintf(stream, "ONBOOT=yes\n");
        fprintf(stream, "BOOTPROTO=none\n");
        fprintf(stream, "USERCTL=no\n");
        fprintf(stream, "TYPE=BOND\n");
        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
                return -2;
        }
        return 0;
}

int NetMgr::DelBondNetFile(const string& bond)
{
        int ret;
        string configFile = BOND_CONFIG_DIR + "ifcfg-" + bond;
        ret = remove(configFile.c_str());
        if (ret != 0) {
                LOG_ERROR("delete bond config file error"); 
                return -1;
        }
        return 0;
}

int NetMgr::UpdateEthNetFile(string BondName, string Eth, int Sig)
{
        ChildProcessOpr cmdOpr;

        string EthFileName = "/etc/sysconfig/network-scripts/ifcfg-" + Eth;
        FILE *stream = fopen(EthFileName.c_str(), "r+");
        if (NULL == stream) {
                LOG_ERROR("open eth config file error");
                return -1;
        }

        const int LEN = 128;
        char LineBuf[LEN];
        string line;
        vector<string> File;
        string Master = "MASTER=" + BondName + "\n";
        string Slave = "SLAVE=yes\n";
        int ret;

        while(fgets(LineBuf, LEN, stream)) {
                line.clear();
                line = LineBuf;
                if (line.find(Master) != string::npos		//MASTRE=bondN
                                        || line.find(Slave) != string::npos) {	//SLAVE=yes
                        continue;
                }
                File.push_back(line);
                memset(LineBuf, 0, LEN);
        }

        rewind(stream);
        if (0 != (ret = ftruncate(fileno(stream), 0))) {
                LOG_ERROR("truncate file /etc/sysconfig/network error");	
                return ret;
        }

        if (Sig == 0) {					//delete from file
                for(vector<string>::iterator it = File.begin(); it != File.end(); ++it) {
                        fwrite(it->c_str(), sizeof(char), it->length(), stream);
                }
        } else if (Sig == 1) {				//add to file 
                File.push_back(Master);
                File.push_back(Slave);
                for(vector<string>::iterator it = File.begin();it != File.end(); ++it) {
                        fwrite(it->c_str(), sizeof(char), it->length(), stream);
                }
        }
        if (0 != (ret = fclose(stream))) {
                LOG_ERROR("close stream error");
                return ret;
        }
        return 0;
}

int NetMgr::GetPortOfBond(const string& bond, vector<string>& ports)
{

        string path = " /etc/sysconfig/network-scripts/ifcfg-eth*";
        string cmd = "grep " + bond + " -r" + path;
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open eth config file error");
                return -1;
        }

        const int LEN = 128;
        char LineBuf[LEN];
        string line = "";
        string temp = "";

        while(fgets(LineBuf, LEN, stream)) {
                line = LineBuf;
                line.erase(line.length() - 1);
                if (line.find("MASTER") != string::npos && line.find("#MASTER") == string::npos) {		
                        int pos = line.find("ifcfg-eth") + 6;
                        temp = line.substr(line.find("ifcfg-eth") + 6, line.rfind(":") - pos);
                        ports.push_back(temp);
                        //LOG_INFO(temp << "-----------GetPortOfBond " << bond);
                }
                line.clear();
                temp.clear();
                memset(LineBuf, 0, LEN);
        }

        int ret;
        if (0 != (ret = pclose(stream))) {
                LOG_ERROR("close stream error");
                return ret;
        }

        return 0;
}

int NetMgr::GetBondInfo(string BondName, BondInfo& BondInformation)
{ 
        string cmd = "ifconfig " + BondName;
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open ifcfg-ehtN file error");
                return -1;
        }
        BondInformation.Name = BondName;                                        //name

        const int LEN = 128;
        char LineBuf[LEN];
        string line;

        GetGateWay(BondName, BondInformation.GateWay);
        GetIp(BondName, BondInformation.Ip);
        GetMask(BondName, BondInformation.Mask);

        while (fgets(LineBuf, LEN, stream)) {
                line = LineBuf;
                line.erase(line.length() - 1);
                size_t pos = 0;
                if ((pos = line.find("HWaddr")) != string::npos) {
                        BondInformation.Mac = line.substr(line.find("HWaddr") + 7);
                }
        }

        int ret;                
        if (0 != (ret = GetBondAlias(BondName, BondInformation.Alias))) {       //alias
                LOG_ERROR("Get bond alias error");
                pclose(stream);
                return ret;
        }
        if (0 != (ret = GetBondStatus(BondName, BondInformation.Status))) {     //status
                LOG_ERROR("Get Bond Status error");					
                pclose(stream);
                return ret;
        }

        vector<string> Eths;
        PortInfo PortInformation;
        if (0 != (ret = GetPortOfBond(BondName, Eths))) {
                LOG_ERROR("get eths from bond name error"); 
                pclose(stream);
                return ret;
        }
        for (vector<string>::iterator it = Eths.begin(); it != Eths.end(); ++it) {
                if (0 != (ret = GetPortInfo(*it, PortInformation))) {
                        LOG_ERROR("Get port info error");
                        pclose(stream);
                        return ret;
                }
                //LOG_INFO(BondName << ": " << *it << "---------GetBondInfo");
                BondInformation.PortInfoList.push_back(PortInformation);
        }

        if (0 != (ret = pclose(stream))) {
                LOG_ERROR("close stream error");
                return ret;
        }
        return 0;
}

int NetMgr::GetBondStatus(const string& bond , int& status)
{
        return GetPortStatus(bond, status);
}

bool NetMgr::IsBonded()
{
        vector<string> bonds;
        GetBondName(bonds);
        return !bonds.empty();
}

int NetMgr::GetBondAlias(const string& bond, string& alias)
{
        alias = "******";
        return 0;
}

int NetMgr::GetAllBondInfo(list<BondInfo>& BondInfoList)
{
        vector<string> BondNames;
        int ret;
        if (0 != (ret = GetBondName(BondNames))) {
                LOG_ERROR("Get bond name error");
                return ret;
        }
        for (vector<string>::iterator it = BondNames.begin();
                                it != BondNames.end(); ++it) {
                //LOG_INFO(*it << "--------------GetAllPortsInfo");
                BondInfo bi;
                if (0 != (ret = GetBondInfo(*it, bi))) {
                        LOG_ERROR("Get port info error");
                        return ret;
                }
                BondInfoList.push_back(bi);
                /*
                   cout << bi.Name << " begin----------" << endl;
                   for (list<PortInfo>::iterator it = bi.PortInfoList.begin();
                   it != bi.PortInfoList.end(); ++it) {
                   cout << it->Name << endl;                
                   }
                   cout << bi.Name << " end----------" << endl;
                   */
        }

        return 0;
}

int NetMgr::RestartNetWork(const string& port)
{
        string cmd;
        if (port.find("bond")) {
                cmd = "service network restart";
        } else {
                cmd = "ifdown " + port + ";ifup " + port;
        }

        ChildProcessOpr cmdOpr;
        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR("restart network for " << port << " error");
                return -1;
        }
        return 0;
}

int NetMgr::ChangeIpConfigFile(const string& port, const string& ip)
{
        string fileName = "/etc/sysconfig/network-scripts/ifcfg-" + port;
        FILE *stream = fopen(fileName.c_str(), "r+");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        vector<string> Contents;
        char*  lineBuf = NULL;
        size_t lineLen = 0;
        string line;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;
                if (line.find("IPADDR") != string::npos) {
                        line = "IPADDR=" + ip + "\n"; 
                } 
                Contents.push_back(line);
        }
        free(lineBuf);

        rewind(stream);       
        ftruncate(fileno(stream), 0);

        for (vector<string>::iterator it = Contents.begin(); it != Contents.end(); ++it) {
                fprintf(stream, "%s", it->c_str());
        }

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error");
                return -2;
        }

        return 0;
}

int NetMgr::SetIp(const string& port, const string& ip)
{
        if (ChangeIpConfigFile(port, ip) != 0 ||
                                RestartNetWork(port) != 0) {
                LOG_ERROR("set ip error");	
                return -1;
        }
        return 0;
}

int NetMgr::ChangeMaskConfigFile(const string& port, const string& mask)
{
        const string fileName = "/etc/sysconfig/network-scripts/ifcfg-" + port;
        FILE *stream = fopen(fileName.c_str(), "r+");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        vector<string> Contents;
        bool   hasMask = false;
        char*  lineBuf = NULL;
        size_t lineLen = 0;
        string line;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf; 
                if (line.find("NETMASK") != string::npos) {
                        line = "NETMASK=" + mask + "\n"; 
                        hasMask = true;
                }
                Contents.push_back(line);
        }
        if (!hasMask) {
                line = "NETMASK=" + mask + "\n";
                Contents.push_back(line);
        }

        rewind(stream);       
        ftruncate(fileno(stream), 0);

        vector<string>::iterator it = Contents.begin();
        for ( ; it != Contents.end(); ++it) {
                fprintf(stream, "%s", it->c_str());
        }

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error");
                return -2;
        }

        return 0;
}

int NetMgr::SetMask(const string& port, const string& mask)
{
        if (ChangeMaskConfigFile(port, mask) != 0 ||
                                RestartNetWork(port) != 0) {
                LOG_ERROR("set mask error");
                return -1;
        } 
        return 0;
}

//读取ip配置文件，修改子网掩码和IP地址，写入文件，重启网卡
int NetMgr::SetNet(const string& port, NetInfo* netInfo)
{
        if (ChangeMaskConfigFile(port, netInfo->Mask) != 0 ||
                                ChangeIpConfigFile(port, netInfo->IP) != 0 || 
                                RestartNetWork(port)) {
                LOG_ERROR("set net error");	
                return -1;
        } 

        return 0;
}

int NetMgr::ChangeNetworkFile(const string& name)
{
        FILE *stream = fopen("/etc/sysconfig/network", "r+");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;	
        }

        vector<string> Contents;
        char*  lineBuf = NULL;
        size_t lineLen = 0;
        string line;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf; 
                if (line.find("HOSTNAME") != string::npos) {
                        line = "HOSTNAME=" + name + ".localdumain\n";
                } 
                Contents.push_back(line);        
        }
        free(lineBuf);

        rewind(stream);
        ftruncate(fileno(stream), 0); 

        vector<string>::iterator it = Contents.begin();                         //rewrite hostname config file
        for ( ; it != Contents.end(); ++it) {
                fprintf(stream, "%s", it->c_str());

        }

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error");
                return -2;
        }

        return 0;
}

int NetMgr::SetMachineName(const string& name)
{
        if (ChangeNetworkFile(name) != 0) {
                LOG_ERROR("change file /etc/sysconfig/network error"); 
                return -1;
        }

        ChildProcessOpr cmdOpr;
        string cmd = "hostname " + name;

        if (cmdOpr.ExecuteCmd(cmd) != 0) {
                LOG_ERROR("update hostname error"); 
                return -2;
        }
        return 0;
}

int NetMgr::SetGateWay(string PortName, string gateWayIp)
{
        int  ret = 0;
        bool hasGateWay = false;
        string FileName = "/etc/sysconfig/network-scripts/ifcfg-" + PortName;
        FILE *file = fopen(FileName.c_str(), "r+");
        if (NULL == file) {
                LOG_ERROR("open stream error");
                return -1;
        }

        const int LEN = 256;
        char LineBuf[LEN];
        string line;
        vector<string> Contents;

        while (fgets(LineBuf, LEN, file)) {
                line  = LineBuf;
                if (line.find("GATEWAY") == string::npos) {
                        Contents.push_back(line);	
                } else {
                        line.clear();
                        line = "GATEWAY=" + gateWayIp + "\n";
                        Contents.push_back(line);
                        hasGateWay = true;
                }
        }

        if (!hasGateWay) {
                line = "GATEWAY=" + gateWayIp + "\n";
                Contents.push_back(line);
        }

        rewind(file); 
        if (0 != (ret = ftruncate(fileno(file), 0))) {
                fclose(file);
                LOG_ERROR("truncate net config file error");	
                return ret;
        }

        for (vector<string>::iterator it = Contents.begin(); 
                                it != Contents.end(); ++it) {
                ret = fwrite(it->c_str(), sizeof(char), it->length(), file);
                if (ret < 0) {
                        LOG_ERROR("fwrite error! ret=" << ret);
                        return -1;
                }
        }
        ret = fclose(file);
        if (ret < 0) {
                LOG_ERROR("fclose error!");
                return -2;
        }

        ChildProcessOpr cmdOpr; 
        ret = cmdOpr.ExecuteCmd("service network restart");
        if (ret < 0) {
                LOG_INFO("restart network error"); 
        }

        return 0;
}

int NetMgr::GetGateWay(const string& port, string& gateway)
{
        FILE *stream = popen("route|grep \"default\"|awk '{print $2}'", "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        char*  lineBuf = NULL;
        size_t bufLen  = 0;

        if (getline(&lineBuf, &bufLen, stream) != -1) {
                gateway = lineBuf;
                gateway.erase(gateway.find('\n'));
        } else {
                gateway = "";        
        }
        free(lineBuf);

        if (0 != pclose(stream)) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int NetMgr::GetIp(const string& port, string& ip)
{
        string path = "/etc/sysconfig/network-scripts/ifcfg-" + port;
        FILE *stream = fopen(path.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        string line;
        char*  lineBuf = NULL;
        size_t lineLen = 0;

        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf;
                if (line.find("IPADDR") != string::npos) {
                        ip = line.substr(line.find("=") + 1);
                        ip.erase(ip.find('\n'));
                        if (ip[0] == '\"') {
                                ip.erase(0, 1);
                        }
                        if (ip[ip.size()-1] == '\"') {
                                ip.erase(ip.size()-1);
                        }
                }
        }
        free(lineBuf);

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int NetMgr::GetMask(const string& port, string& mask)
{
        /* 配置文件中可能没有掩码，但是ifconfig能查出掩码
        const string cmd = "ifconfig " + port + "|grep \"Mask\"|awk -F \"Mask:\" '{print$2}'";
           */
        const string cmd = "awk -F \"=\" '/NETMASK/{print $2}' /etc/sysconfig/network-scripts/ifcfg-"
                                + port;
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error"); 
                return -1;
        }

        char*  lineBuf = NULL;
        size_t bufLen  = 0;

        if (getline(&lineBuf, &bufLen, stream) != -1) {
                mask = lineBuf;
                mask.erase(mask.find('\n'));
        } else {
                mask = "";
        }
        free(lineBuf);

        if (0 != pclose(stream)) {
                LOG_ERROR("close stream error"); 
        }
        return 0;
}

int NetMgr::GetDns(string& dns)
{
        //string cmd = "cat " + DNS_CONFIG_FILE + "|grep \"^dns\"|awk '{print $4}'";
        string cmd = "grep \"nameserver\" /etc/resolv.conf | awk '{print$2}'";
        FILE *stream = popen(cmd.c_str(), "r");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        char*  lineBuf = NULL;
        size_t lineLen = 0;
        if (getline(&lineBuf, &lineLen, stream) != -1) {
                dns = lineBuf;
                dns.erase(dns.find('\n'));
        } else {
                dns = "";
        }
        free(lineBuf);

        if (fclose(stream) != 0) {
                LOG_ERROR("close stream error");
                return -2;
        }

        return 0;
}

int NetMgr::SetDns(const string& dns)
{
        const string cfgFile = "/etc/resolv.conf";
        vector<string> commands;

        FILE* stream = fopen(cfgFile.c_str(), "r+");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }

        char   *lineBuf = NULL;
        size_t lineLen = 0;
        string line;
        while (getline(&lineBuf, &lineLen, stream) != -1) {
                line = lineBuf; 
                line.erase(line.find('\n'));
                if (line.find("nameserver") == string::npos) {
                        commands.push_back(line);
                }
        }
        free(lineBuf);
        commands.push_back("nameserver " + dns);
        if (fclose(stream) < 0) {
                LOG_ERROR("close stream error");
                return -2;
        }

        stream = NULL;
        stream = fopen(cfgFile.c_str(), "w");
        if (NULL == stream) {
                LOG_ERROR("open stream error");
                return -1;
        }
        vector<string>::iterator iter = commands.begin();
        for ( ; iter != commands.end(); ++iter) {
                fprintf(stream, "%s\n", iter->c_str());
        }
        if (fclose(stream) != 0) {
                LOG_ERROR("close fout error"); 
        }

        //restart server: hpptd, network and sendmail
        ChildProcessOpr cmd;
        if (cmd.ExecuteCmd("service network restart") != 0) {
                LOG_ERROR("restart network error");
                return -3;
        }
        sleep(5);
        if (cmd.ExecuteCmd("service httpd restart") != 0) {
                LOG_ERROR("restart httpd error");
                return -3;
        }
        if (cmd.ExecuteCmd("service sendmail restart") != 0) {
                LOG_ERROR("restart sendmail error");
                return -3;
        }
        return 0;
}

int NetMgr::ClearDns()
{
        int ret;
        const string cmd = "sed -i '/nameserver/d' /etc/resolv.conf";
        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret != 0) {
                LOG_ERROR("clear dns error");
                return ret;
        }

        if (cmdOpr.ExecuteCmd("service network restart") != 0) {
                LOG_ERROR("restart network error");
                return -3;
        }
        sleep(5);
        if (cmdOpr.ExecuteCmd("service httpd restart") != 0) {
                LOG_ERROR("restart httpd error");
                return -3;
        }
        if (cmdOpr.ExecuteCmd("service sendmail restart") != 0) {
                LOG_ERROR("restart sendmail error");
                return -3;
        }
        return 0;
}

int NetMgr::ClearGateway() 
{
        int ret;
        const string cmd = "sed -i '/GATEWAY/d' /etc/sysconfig/network-scripts/ifcfg-eth0";
        ChildProcessOpr cmdOpr;
        ret = cmdOpr.ExecuteCmd(cmd);
        if (ret != 0) {
                LOG_ERROR("clear gateway error");
                return ret;
        }

        if (cmdOpr.ExecuteCmd("service network restart") != 0) {
                LOG_ERROR("restart network error");
                return -1;
        }
        return 0;
}

int NetMgr::GetNetworkId(string& networkId)
{
        int ret = 0;
        IniParser iniParse("/etc/scigw/default/GWconfig");
        ret = iniParse.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                return ret;
        }
        ret = iniParse.GetVal("GatewayArchive", "networkId", networkId);
        if (ret < 0) {
                LOG_ERROR("GetVal Error! ret=" << ret);
        }
        return ret;
}

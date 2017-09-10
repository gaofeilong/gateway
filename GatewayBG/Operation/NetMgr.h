#ifndef _NETMGR_H_
#define _NETMGR_H_

#include <vector>
#include "GatewayBG/Operation/BaseInfo.h"
using namespace std;

class NetMgr
{
public:
        NetMgr();
        ~NetMgr();
public:
        int GetNetworkId(string& networkId);
        int GetAvailBond(string &bond);
        int GetPortName(vector<string>& ports);                                                 //name
        int GetPortAlias(const string& port, char* alias);                                      //alias
        int GetPortRate(const string& port, string& rate);                                      //rate
        int GetPortVendorAndChip(const string& port, string& vendor, string& chip);             //vendor and chip
        int GetPortStatus(const string& port, int& status);
	int RestartNetWork(const string& port);
        int GetMask(const string& port, string& mask);
        int GetIp(const string& port, string& ip);
        int GetDns(string& dns);
        int SetDns(const string& dns);
        int GetGateWay(const string& port, string& gateway);
        int SetMachineName(const string& name);
        int ChangeNetworkFile(const string& name);
        int SetNet(const string& PortName, NetInfo* netInfo);
        int SetMask(const string& port, const string& mask);
	int ChangeMaskConfigFile(const string& port, const string& mask);
        int SetIp(const string& port, const string& ip);
	int ChangeIpConfigFile(const string& port, const string& ip);
	int GetBondAlias(const string& bond, string& alias);
        int GetBondStatus(const string& bond, int& status);
        bool IsBonded();

        int ClearDns();
        int ClearGateway();

	int DelBondNetFile(const string& bond);
	int AddBondNetFile(const string& bond, const string& ip, const string& gateway, const string& netmask);

	int GetAllPortsInfo(list<PortInfo>& portsInfo);
        int GetFreePortInfo(list<PortInfo>& portsInfo);
        int GetBondName(vector<string>& bonds);

        int GetPortOfBond(const string& bond, vector<string>& ports);
        int GetPortInfo(string PortName, PortInfo& PortInformation);   //ip, mask, gateway, mac
        int GetBondInfo(string BondName, BondInfo& BondInformation);
        int GetAllBondInfo(list<BondInfo>& BondInfoList);
        int SetGateWay(string PortName, string gateWayIp);
        int Bond(vector<string> Eths);                 //bond multiple net card
        int UnBond(string BondName);
	int Config(string BondName);
	int UpdateEthNetFile(string BondName, string Eth, int Sig);
};

#endif //_NETMGR_H_

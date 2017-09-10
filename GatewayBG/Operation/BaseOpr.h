#ifndef _BASE_OPR_H_
#define _BASE_OPR_H_

#include <list>
#include <vector>
#include <stdint.h>

#include "GatewayBG/Operation/DiskMgr.h"
#include "GatewayBG/Operation/NetMgr.h"
#include "GatewayBG/Operation/BaseInfo.h"

typedef std::vector<std::string>::iterator ITER;
typedef std::list<std::string>::iterator ListStrIter;

class BaseOpr
{
public:
        BaseOpr();
        ~BaseOpr();

public:
        int ReBoot();
        int ShutDown();
        /**
         * @note 获取CPU相关信息：cpu个数，主频，厂商，系列及型号
	 * @return =0 成功
         *         =-1 失败
         */
        int GetCPUInfo(std::string& cpuInfo);
	int GetCpuRatio(float& ratio);
	int GetMemUsage(int64_t& total, int64_t& used);
	int GetCpuCoreNum(int &num);
        /**
         * @param str 要删除多余空格的string对象 
         * @note 删除str中连续多个空格，只留下一个 
	 * @return =0 成功
         * @       =-1 失败 
         */
	int EraseSpace(std::string &str);


        int GetLeftSpaceByMp(string mp, int64_t& space, int64_t& left);

        /**
         * @param pid 进程ID
         * @note 强制杀死指定进程
	 * @return: on success, 0 is returned, on kill() error, -1 is returned
	 * @return =0 成功
         *         =-1 失败
         */
        int Kill(int pid);
        /**
         * @
         */
        int GetDiskDriver(std::list<std::string>& devs);
        /**
         * @param dev 设备名
         * @type 文件系统类型
         * @return =0 成功
         *         =-1 失败
         */
        int Mkfs(const std::string &dev, const std::string &type);
        /**
         * @param dev 设备名
         * @mp 挂载点
         * @return =0 成功
         *         =-1 失败
         */
        int Mount(const std::string &dev, const std::string &mp);
        /**
         * @param mp 挂载点
         * @return =0 成功
         *         =-1 失败
         */
	int UnMount(const std::string &mp);
        /**
         * @param dev 设备号
	 * @mp 分区大小，单位：MB
	 * @return =0 成功
	 *	  =-1 无设备
	 *	  =-2 获取分区信息失败
	 *	  =-3 执行分区操作失败
         */
        int Partition(const std::string &dev, int64_t size);
        /**
         * @desc 将剩余空间全部分成一个区
	 * @param dev 设备号
	 * @return =0 成功
	 *	   =-1 主分区达到4个
	 *	   =-2 执行分区失败
         */
        int DefPartition(const std::string &dev);
        /**
         * @param dev 要删除的设备名
         * @return =0 成功
         *         =-1 结束子进程失败
	 *         =-2 子进程非正常结束
         */
        int DeletePartition(const std::string &partition);
        /**
         * @desc :获取盘阵总大小和未分区的大小
	 * @param dev 盘阵 
	 * @	  total 盘阵总大小
	 * @	  left 未分区的大小
	 * @ return 0 成功
	 *	    -1 失败
         */
	int GetDiskFormatInfo(const std::string &dev, double &total);
        /**
         * @desc 获取网卡生产厂商信息
	 * @param com 存储返回的厂商信息字符串
	 * @return =0 成功
	 *	   =-1 失败
         */
        /**
         * @desc 获取网卡带宽信息
	 * @param com 存储返回的带宽信息字符串
	 * @return =0 成功
	 *	   =-1 失败
         */
	int GetStorageInfo(map<string, DevInfo>& StorageInfo);
        /**
         * @param name 系统文件/etc/hosts
         * @note 修改hosts文件中主机名行
	 * @return =0 成功
         * @       =-1 打开文件失败
         * @       =-2 创建子进程失败
         */
	int FindDev(const std::string &dev);	
        /**
         * @获取设备上主分区的数目
	 * @param dev 设备名
	 * @return =-1 设备不存在
	 *	   =-2 获取磁盘信息失败
	 *	   成功返回主分区数目
         */
	int GetPParCnt(const std::string &dev);	
        /**
         * @取得一个可用的主分区号，用于磁盘分区
	 * @param dev 设备名
	 * @return 成功返回一个可用的分区号
	 *	=-1 获取分区信息失败
	 *	=-2 无设备
	 *	=-3 主分区数超过3个
	 *	=-4 函数结束失败
         */
	int GetParNum(const std::string &dev);	
        /**
         * 获取IDE硬盘信息, 该函数直接调用GetDriver() 
         */
        int GetHDiskDriver(std::list<std::string>& devs);
        /**
         * 获取SCSI硬盘信息, 该函数直接调用GetDriver() 
         */
        int GetSDiskDriver(std::list<std::string>& devs);
        /**
         * 获取硬盘信息
	 * param devs 磁盘分区信息映射
	 *	 type 磁盘类型 (ide or scsi)
	 * return =0 成功
	 *	  =-1 打开磁盘配置文件失败
         */
        int GetDriver(std::list<std::string>& devs, char type);

        int GetNetworkId(string& networkId);
        int GetMachineName(string& MachineName);
        int GetGateWay(string PortName, string& GateWay);
        int GetIp(string PortName, string& Ip );
        int GetMask(string PortName, string& Mask);
        int GetDns(string& dns);
        int GetPortName(vector<string>& PortNames);                     //name
        int GetPortInfo(string PortName, PortInfo& PortInformation);   //ip, mask, gateway, mac
	int GetAllPortsInfo(list<PortInfo>& PortsInfo);
        int GetFreePortInfo(list<PortInfo>& PortsInfo);
        int GetBondInfo(string BondName, BondInfo& BondInformation);
        int GetAllBondInfo(list<BondInfo>& BondInfoList);
        bool IsBonded();

        int SetDns(const string& dns);
        int SetGateWay(string PortName, string Ip);
        int SetIp(string PortName, string Ip);
        int SetMask(string PortName, string Mask);
        int SetNet(string PortName, NetInfo* NetInformation);
        int SetMachineName(string MachineName);

        int ClearDns();
        int ClearGateway();

        int Bond(vector<string> Eths);
        int UnBond(string BondName);
private:
        DiskMgr         m_DiskMgr;
        NetMgr          m_NetMgr;
};

#endif //_BASEOPR_H_

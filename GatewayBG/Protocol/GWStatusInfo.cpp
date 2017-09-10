#include <stdio.h>
#include <vector>

#include "Utils/Log/Log.h"
#include "Config/IniParser.h"
#include "GatewayBG/Protocol/GWStatusInfo.h"
#include "GatewayBG/Operation/VersionMgr.h"

using std::vector;

GWStatusInfo::GWStatusInfo(PackMgr* packMgr, DDFSMgr* ddfsMgr, BaseOpr* baseOpr)
        :Protocol(ddfsMgr, baseOpr)
{
        m_PackMgr = packMgr;
}

int GWStatusInfo::Execute(const Pack& pack)
{
        //网关状态查看 0x3000000
        boost::mutex::scoped_lock lock(m_Mutex);
        int ret = 0;

        switch(pack.PkHead.Type & ASSISTANT_SECOND) {
        case GATEWAY_SECOND:
                ret = GateWayParse(pack);
                if (ret < 0) {
                        LOG_ERROR("GateWayParse Error! ret=" << ret);
                        return ret;
                }
                break;
        case AUTORIZATION_SECOND:
                ret = AutorizationParse(pack);
                if (ret < 0) {
                        LOG_ERROR("AutorizationParse Error! ret=" << ret);
                        return ret;
                }
                break;
        }

        return 0;
}

int GWStatusInfo::GateWayParse(const Pack& pack)
{
        int ret = 0;
        //0x310000x
        switch(pack.PkHead.Type) {
        case CMD_GATEWAY_RESOURCE:
                ret = ExecuteGateWayInfo(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteGateWayInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_MP_RESOURCE:
                ret = ExecuteMountPointInfo(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteMountPointInfo Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_ADD_NODE:
                ret = ExecuteAddNode(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteAddNode Error! ret=" << ret);
                        return ret;
                } 
                break;
        }
        return 0;
}

int GWStatusInfo::AutorizationParse(const Pack& pack)
{
        int ret = 0;
        //0x320000x
        switch(pack.PkHead.Type) {
        case CMD_AUTHORIZATION:
                ret = ExecuteAutorization(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteAutorization Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LICENSE_IMPORT:
                ret = ExecuteLicenseImport(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLicenseImport Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LICENSE_LOOKUP:
                ret = ExecuteLicenseLookup(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLicenseLookup Error! ret=" << ret);
                        return ret;
                }
                break;
        case CMD_LICENSE_EXPORT:
                ret = ExecuteLicenseExport(pack);
                if (ret < 0) {
                        LOG_ERROR("ExecuteLicenseExport Error! ret=" << ret);
                        return ret;
                }
                break;
        }
        return 0;
}

int GWStatusInfo::ExecuteAutorization(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteAutorization!---", pack);
        return 0;
}

int GWStatusInfo::ExecuteLicenseImport(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteLicenseImport!---", pack);
        int ret = 0;

        /* 判断是否有挂载状态的挂载点 */
        map<string, string> onLineMpMap;
        m_DDFSMgr->GetOnlineMPMap(onLineMpMap);
        if (!onLineMpMap.empty()) {
                if (!SendErr("有挂载点处于挂载状态，请卸载后再导入License!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        }

        string licSrcPath  = pack.JsonValue["licSrcPath"].asString();

        VersionMgr vMgr;
        ret = vMgr.ImportLicense(licSrcPath);
        if (ret < 0) {
                LOG_ERROR("ImportLicense Error! licSrcPath=" << licSrcPath);
                if (!SendErr("导入license文件失败!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        } else if (ret == LIC_NOT_EXIST) {
                if (!SendErr("license文件不存在!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        } else if (ret == LIC_INVALID) {
                if (!SendErr("license文件不合法!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        } else if (ret == LIC_MD5_SAME) {
                if (!SendErr("license文件同现有文件相同!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        }

        if ( !SendOk() ) {
                LOG_ERROR("SendOk Error!");
        }
        return ret;
}

int GWStatusInfo::ExecuteLicenseLookup(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteLicenseLookup!---", pack);
        int ret = 0;

        VersionMgr vMgr;
        vector<string> licInfoList;
        string useInfo;
        ret = vMgr.GetVerInfo(licInfoList, useInfo);
        if (ret < 0) {
                LOG_ERROR("GetVerInfo Error!");
                if (!SendErr("获取license信息失败!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        }

        Pack licInfo;
        licInfo.PkHead.Type = CMD_OK;

        for (size_t i=0; i<licInfoList.size(); ++i) {
                licInfo.JsonValue["licInfo"].append(licInfoList[i]);
        }

        licInfo.JsonValue["useInfo"] = useInfo;

        if (!m_PackMgr->Send(licInfo)) {
                LOG_ERROR("Send Ok Error");
                return ret;
        }

        cout << licInfo.JsonValue.toStyledString() << endl;
        return ret;
}

int GWStatusInfo::ExecuteLicenseExport(const Pack& pack)
{
        DisplayExecuteInfo("---ExecuteLicenseExport!---", pack);
        int ret = 0;

        string licSrcPath  = pack.JsonValue["licSrcPath"].asString();

        VersionMgr vMgr;
        ret = vMgr.ExportLicense(licSrcPath);
        if (ret < 0) {
                LOG_ERROR("ExportLicense Error! licSrcPath=" << licSrcPath);
                if (!SendErr("导入license文件失败!")) {
                        LOG_ERROR("Send Error!");
                }
                return ret;
        }

        if ( !SendOk() ) {
                LOG_ERROR("SendOk Error!");
        }
        return ret;
}

int GWStatusInfo::ExecuteGateWayInfo(const Pack& pack)
{
        int ret = 0;
        DisplayExecuteInfo("---ExecuteGateWayInfo!---", pack);

        Pack info;
        char buffer[100];

        //-----cpu个数
        int num = 0;
        m_BaseOpr->GetCpuCoreNum(num);
        info.JsonValue["cpunum"] = num;

        //----内存总量和使用量
        int64_t total = 0;
        int64_t used  = 0;

        ret = m_BaseOpr->GetMemUsage(total, used);
        if (ret < 0) {
                if (!SendErr("获取内存容量失败!")) {
                        LOG_ERROR("GetMemUsage Error!");
                }
                return ret;
        }

        sprintf(buffer, "%ld", total);
        info.JsonValue["totalmem"] = buffer;
        
        sprintf(buffer, "%ld", used);
        info.JsonValue["usemem"] = buffer;

        //--cpu使用量
        float cpu = 0;
        ret = m_BaseOpr->GetCpuRatio(cpu);
        if (ret < 0) {
                if (!SendErr("获取CPU使用量失败!")) {
                        LOG_ERROR("GetCpuRatio Error!");
                }
                return ret;
        }
        sprintf(buffer, "%.2f", cpu);
        info.JsonValue["cpu"] = buffer;

        info.PkHead.Type = CMD_OK;
        if (!m_PackMgr->Send(info)) {
                LOG_ERROR("Send MountPointInfo Ok Error");
                return ret;
        }

        cout << info.JsonValue.toStyledString() << endl;
        return 0;
}

int GWStatusInfo::ExecuteMountPointInfo(const Pack& pack)
{
        int ret = 0;
        DisplayExecuteInfo("---ExecuteMountPointInfo!---", pack);

        std::string mp = pack.JsonValue["mp"].asString();
        if (mp.empty()) {
                if (!SendErr("客户端传来的路径为空!")) {
                        LOG_ERROR("GetCpuRatio Error!");
                }
                return ret;
        }

        float mem = 0;
        ret = m_DDFSMgr->GetUsedMemRatio(mp.c_str(), mem);
        if (ret < 0) {
                if (!SendErr("获取内存使用率失败!")) {
                        LOG_ERROR("GetCpuRatio Error!");
                }
                return ret;
        }

        float cpu = 0;
        ret = m_DDFSMgr->GetUsedCPURatio(mp.c_str(), cpu);
        if (ret < 0) {
                if (!SendErr("获取CPU使用率失败!")) {
                        LOG_ERROR("GetCpuRatio Error!");
                }
                return ret;
        }

        Pack data;
        char buffer[100];

        //-----cpu个数
        int num = 0;
        m_BaseOpr->GetCpuCoreNum(num);
        data.JsonValue["cpunum"] = num;

        //sprintf(buffer, "%ld", total);
        data.JsonValue["totalmem"] = 0;
        
        //int64_t usedmem = total * (mem/100);
        sprintf(buffer, "%.2f", mem);
        data.JsonValue["usemem"] = buffer;

        //--cpu使用量
        sprintf(buffer, "%.2f", cpu);
        data.JsonValue["cpu"] = buffer;

        cout << data.JsonValue.toStyledString() << endl;

        data.PkHead.Type = CMD_OK;
        if (!m_PackMgr->Send(data)) {
                LOG_ERROR("Send Info Error");
                return ret;
        }
        cout << data.JsonValue.toStyledString() << endl;
        return 0;
}

int GWStatusInfo::ExecuteAddNode(const Pack& pack)
{
        IniParser iniOpr("/etc/scigw/default/Mysql.cnf");
        int ret = iniOpr.Init();
        if (ret < 0) {
                LOG_ERROR("Init Error!");
                SendErr("获取服务端Ip失败!");
                return ret;
        }
        string serverIp;
        iniOpr.GetVal("mysql", "server", serverIp);

        Pack revPack;
        revPack.PkHead.Type           = CMD_OK;
        revPack.JsonValue["serverIp"] = serverIp;

        m_PackMgr->Send(revPack);

        return 0;
}

#ifndef _CMD_TYPE_H_
#define _CMD_TYPE_H_

const int ASSISTANT_FIRST   = 0xF000000;
const int ASSISTANT_SECOND  = 0xFF00000;

const int CMD_OK   = 0x01;
const int CMD_ERR  = 0x02;
const int CMD_END  = 0x03;

//网关基础信息配置
const int NETINFO_MAIN         = 0x2000000;
const int NETINFO_SECOND       = 0x2200000;
const int LOG_SECOND           = 0x2500000;
const int EMAIL_SECOND         = 0x2600000;
const int NFS_SECOND           = 0x2700000;
const int MULTIPATH_SECOND     = 0x2800000;
const int SSDCACHE_SECOND      = 0x2900000;

const int CMD_NETINFO_CONFIG   = 0x2200001;
const int CMD_NETINFO_LOOKUP   = 0x2200002;

const int CMD_LOG_INFO         = 0x2500001;
const int CMD_LOG_WAR          = 0x2500002;
const int CMD_LOG_ERR          = 0x2500003;
const int CMD_LOGERR_LOOKUP    = 0x2500004;
const int CMD_LOGWAR_LOOKUP    = 0x2500005;

const int CMD_EMAIL_SRV_CONFIG = 0x2600002;
const int CMD_EMAIL_SRV_LOOKUP = 0x2600003;
const int CMD_EMAIL_CONFIG     = 0x2600004;
const int CMD_EMAIL_LOOKUP     = 0x2600005;

const int CMD_NFS_SAVE         = 0x2700001;
const int CMD_NFS_START        = 0x2700002;
const int CMD_NFS_STOP         = 0x2700003;
const int CMD_NFS_LOOKUP       = 0x2700004;

const int CMD_MULTIPATH_START  = 0x2800001;
const int CMD_MULTIPATH_STOP   = 0x2800002;
const int CMD_MULTIPATH_LOOKUP = 0x2800003;

const int CMD_SSDCACHE_CREATE  = 0x2900001;
const int CMD_SSDCACHE_LOAD    = 0x2900002;
const int CMD_SSDCACHE_UNLOAD  = 0x2900003;
const int CMD_SSDCACHE_LOOKUP  = 0x2900004;
const int CMD_SSDCACHE_DESTROY = 0x2900005;
//网关状态查看
const int GATEWAY_MAIN         = 0x3000000;
const int GATEWAY_SECOND       = 0x3100000;
const int AUTORIZATION_SECOND  = 0x3200000;

const int CMD_GATEWAY_RESOURCE = 0x3100001;
const int CMD_MP_RESOURCE      = 0x3100002;
const int CMD_ADD_NODE         = 0x3100003;

const int CMD_AUTHORIZATION    = 0x3200001;

const int CMD_LICENSE_IMPORT   = 0x3200002;
const int CMD_LICENSE_LOOKUP   = 0x3200003;
const int CMD_LICENSE_EXPORT   = 0x3200004;

//日志管理
const int SYSTEM_MAIN          = 0x4000000;
const int SYSTEM_LOG_SECOND    = 0x4100000;
const int SYSTEM_WAR_SECOND    = 0x4300000;

const int CMD_SYSTEM_LOG       = 0x4100001;
const int CMD_SYSTEM_WAR       = 0x4300002;

//存储网关管理
const int DEV_MAIN                  = 0x5000000;
const int DEV_SECOND                = 0x5100000;
const int NETCARD_SECOND            = 0x5200000;
const int DDFS_SECOND               = 0x5300000;
const int ARCHIVE_SECOND            = 0x5400000;

const int CMD_DISK_DRIVER           = 0x5100001;
const int CMD_DEV_MOUNT             = 0x5100002;
const int CMD_DEV_UMOUNT            = 0x5100003;
const int CMD_DEV_DELETE            = 0x5100004;

const int CMD_FROMAT_DEV            = 0x5100005;
const int CMD_CREATE_PARTITION_DEV  = 0x5100006;

const int CMD_NETCARD_INFO          = 0x5200001;
const int CMD_NETCARD_TRUNK         = 0x5200002;
const int CMD_NETCARD_UNTRUNK       = 0x5200003;
const int CMD_NETPORT_CONFIG        = 0x5200004;

const int CMD_DDFS_START            = 0x5300001;
const int CMD_DDFS_STOP             = 0x5300002;
const int CMD_DDFS_RESTART          = 0x5300003;
const int CMD_DDFS_FORCE_STOP       = 0x5300004;
const int CMD_DDFS_FORCE_RESTART    = 0x5300005;
const int CMD_DDFS_LOOKUP           = 0x5300006;

const int CMD_GWENGINE_STOP         = 0x5300007;
const int CMD_GWENGINE_RESTART      = 0x5300008;

const int CMD_DATA_ARCHIVE          = 0x5400001;
const int CMD_DATA_LOOKUP           = 0x5400002;
const int CMD_DATA_STOP             = 0x5400003;
const int CMD_DATA_START            = 0x5400004;
const int CMD_DATA_DELETE           = 0x5400005;
const int CMD_DATA_CONFIGLIST       = 0x5400006;
const int CMD_DATA_MODIFY           = 0x5400007;

//文件系统管理
const int MP_MAIN                   = 0x6000000;
const int MP_SECOND                 = 0x6100000;
const int CONFIG_SECOND             = 0x6200000;
const int MP_OPR_SECOND             = 0x6300000;

const int CMD_MP_ALL_LIST           = 0x6100001;
const int CMD_MP_ONLINE_LIST        = 0x6100002;
const int CMD_MP_INFO               = 0x6100003;

const int CMD_MP_ADD                = 0x6200001;
const int CMD_MP_MODIFY             = 0x6200002;
const int CMD_MP_DELETE             = 0x6200003;
const int CMD_MP_LOOKUP             = 0x6200004;

const int CMD_MP_MOUNT              = 0x6300001;
const int CMD_MP_UMOUNT             = 0x6300002;
const int CMD_MP_FSCK               = 0x6300003;
const int CMD_CLASSIFY_MP           = 0x6300004;

//集群管理
const int CLUSTER_MAIN              = 0x7000000;
const int CLUSTER_SECOND            = 0x7100000;

const int CMD_ADD_GW_NODE           = 0x7100001;
const int CMD_DEL_GW_NODE           = 0x7100002;
const int CMD_TRANSFER_NODE         = 0x7100003;
const int CMD_TRANSFER_MP           = 0x7100004;
const int CMD_SRC_MP_STATUS         = 0x7100005;
const int CMD_DEST_MP_STATUS        = 0x7100006;
const int CMD_SRC_NODE_STATUS       = 0x7100007;
const int CMD_DEST_NODE_STATUS      = 0x7100008;
const int CMD_KEEPALIVED_START      = 0x7100009;
const int CMD_KEEPALIVED_STOP       = 0x710000A;

//系统工具
const int GW_MAIN                   = 0xB000000;
const int GW_SECOND                 = 0xB200000;
const int CMD_GW_RESTART            = 0xB200001;
const int CMD_GW_SHUTDOWN           = 0xB200002;

#endif


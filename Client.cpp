#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>                                                                                                                
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#include "GatewayBG/Protocol/CmdType.h"
#include "GatewayBG/Operation/BaseInfo.h"
#include "GatewayBG/Socket/Pack.h"
#include "GatewayBG/Socket/PackMgr.h"
#include "Utils/Json/include/json.h"
using namespace std;
using namespace Json;

int GWOpr(int sockfd);
int LogMgr(int sockfd);
int DDFSMgr(int sockfd);
int StorageGW(int sockfd);
int GWBaseInfo(int sockfd);
int GWStatusInfo(int sockfd);
int NFSOpr(int sockfs);
int MultiPathOpr(int sockfs);
int SSDOpr(int sockfs);

/*
int main(int argc, char* argv[])
{
        int ret =0;

        if (argc != 2) {
                return 0;
        }
        int port = atoi(argv[1]);

        int sockfd;

        struct sockaddr_in serv_addr = {};

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
                printf("socket create error!\n");
                exit(-1);
        }
        printf("create socket success!\n");

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        //serv_addr.sin_addr.s_addr = inet_addr("192.168.0.7");

        ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (ret == -1) {
                printf("connect server fail!\n");
                exit(-1);
        }
        printf("connect server success!\n");

        //DDFSMgr(sockfd);
        //StorageGW(sockfd);
        //GWStatusInfo(sockfd); 
        //GWOpr(sockfd);
        NFSOpr(sockfd);
        //MultiPathOpr(sockfd);
        //SSDOpr(sockfd);


        close(sockfd);

        return 0;
}
*/

int DDFSMgr(int sockfd)
{
        int cnt = 0;

        Json::Value value;
        Pack pack;
        PackMgr pm(sockfd);
        //-------------------------

        /*
           pack.PkHead.Type = CMD_LOG_ERR;
           pack.JsonValue["aa"] = "";

           pm.Send(pack);

           if (pm.Recv(pack)) {
           if (pack.PkHead.Type == CMD_OK) {
           printf("初始化成功!\n");
           }
           if (pack.PkHead.Type == CMD_ERR) {
           printf("初始化失败!\n");
           }
           }

           pack.PkHead.Type = CMD_END;
           if (pm.Send(pack)) {
           printf("CMD_END!\n");
           }
         */

        ///*
        //::0. 初始化
        pack.PkHead.Type = CMD_MP_ADD;
        pack.JsonValue["path"]                  = "/home/wb/MountPoint/mp";
        pack.JsonValue["metaPath"]              = "/home/wb/MountPoint/db"; 
        pack.JsonValue["indexPath"]             = "/home/wb/MountPoint/db"; 
        pack.JsonValue["dataPath"]              = "/home/wb/MountPoint/db";
        pack.JsonValue["chunkSize"]             = "65535";
        pack.JsonValue["dedupThreadCount"]      = "5";
        pack.JsonValue["ioThreadCount"]         = "4";
        pack.JsonValue["flushThreadCount"]      = "1";
        pack.JsonValue["flushInterval"]         = "60";
        pack.JsonValue["bucketNum"]             = "60";
        pack.JsonValue["defragFreeRatio"]       = "0.3";
        pack.JsonValue["ioWaitRatio"]           = "0.2";

        char buf[100];
        sprintf(buf, "%ld", 1l<<20);
        pack.JsonValue["cacheSize"]             = buf;

        sprintf(buf, "%ld", 30l);
        pack.JsonValue["defragInterval"]        = buf;

        if (pm.Send(pack)) {
                printf("%d Send CMD_MP_ADD!\n", ++cnt);
        }

        if (pm.Recv(pack)) {
                if (pack.PkHead.Type == CMD_OK) {
                        printf("初始化成功!\n");
                }
                if (pack.PkHead.Type == CMD_ERR) {
                        printf("初始化失败!\n");
                        printf("%s\n", pack.JsonValue["info"].asString().c_str());
                }
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        //*/

        /*
        //::1 --挂载
        pack.PkHead.Type = CMD_MP_MOUNT;
        value["mp"] = "/home/wb/MountPoint/mp";  //"/home/wb";
        pack.JsonValue = value;
        if (pm.Send(pack)) {
        printf("%d Send CMD_MP_MOUNT!\n", ++cnt);
        }
        if (pm.Recv(pack)) {
        if (pack.PkHead.Type == CMD_ERR) {
        printf("errinfo is %s\n", pack.JsonValue["info"].asString().c_str());
        }
        if (pack.PkHead.Type == CMD_OK) {
        printf("Mount Success!\n");
        }
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
        printf("CMD_END!\n");
        }
        //////--------------------------------------

        //::2 --获取挂载点目录
        pack.PkHead.Type = CMD_MP_ALL_LIST;
        if (pm.Send(pack)) {
        printf("%d Send CMD_MP_ALL_LIST!\n", ++cnt);
        }

        pm.Recv(pack);
        Json::Value array = pack.JsonValue["mplist"]; 
        Json::Value key;

        for (unsigned int i=0; i<array.size(); ++i) {
        key = array[i];
        printf("array[%d]=%s\n", i, key["path"].asString().c_str());
        printf("array[%d]=%d\n", i, key["state"].asInt());
        }

        pm.Recv(pack);
        if (pack.PkHead.Type == CMD_OK) {
        printf("获取挂载点目录列表成功!\n");
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
        printf("CMD_END!\n");
        }
        ///-------------
        pack.PkHead.Type = CMD_MP_ONLINE_LIST;
        if (pm.Send(pack)) {
        printf("%d Send CMD_MP_ONLINE_LIST!\n", ++cnt);
        }

        pm.Recv(pack);
        array = pack.JsonValue["mplist"]; 

        for (unsigned int i=0; i<array.size(); ++i) {
        key = array[i];
        printf("array[%d]=%s\n", i, key["path"].asString().c_str());
        printf("array[%d]=%d\n", i, key["state"].asInt());
        }

        pm.Recv(pack);
        if (pack.PkHead.Type == CMD_OK) {
        printf("获取挂载点目录列表成功!\n");
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
        printf("CMD_END!\n");
        }

        //::3 --获取指定挂载点的使用情况
        pack.PkHead.Type = CMD_MP_RESOURCE;
        pack.JsonValue["path"] = "/home/wb/MountPoint/mp";
        if (pm.Send(pack)) {
                printf("%d Send CMD_RESOURCE_INFO!\n", ++cnt);
        }

        if (pm.Recv(pack)) {
                printf("cpunum=%d, totalmem=%d, usemem=%d, cpu=%s\n", pack.JsonValue["cpunum"].asInt(), 
                        pack.JsonValue["totalmem"].asInt(), 
                        pack.JsonValue["usemem"].asInt(), 
                        pack.JsonValue["cpu"].asString().c_str()
                      );
        }

        if (pm.Recv(pack)) {
                if (pack.PkHead.Type == CMD_OK) {
                        printf("获取指定挂载点情况 Success!\n");
                }
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //::4 --卸载
        pack.PkHead.Type = CMD_MP_UMOUNT;
        pack.JsonValue["mp"] = "/home/wb/MountPoint/mp";//"/home/wb";
        if (pm.Send(pack)) {
                printf("%d Send CMD_MP_UMOUNT!\n", ++cnt);
        }

        if (pm.Recv(pack)) {
                if (pack.PkHead.Type == CMD_OK) {
                        printf("UnMount Success!\n");
                }
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //::5--挂载点实际数据和消冗状态
        pack.PkHead.Type = CMD_MP_INFO;
        if (pm.Send(pack)) {
                printf("%d Send CMD_MP_INFO!\n", ++cnt);
        }

        if (pm.Recv(pack)) {
                printf("mp=%s, realSize=%s, rawSize=%s, dedup=%s\n", pack.JsonValue["mp"].asString().c_str(),
                        pack.JsonValue["orisize"].asString().c_str(), 
                        pack.JsonValue["realsize"].asString().c_str(), 
                        pack.JsonValue["ratio"].asString().c_str()
                      );
        }
        if (pm.Recv(pack)) {
                if (pack.PkHead.Type == CMD_OK) {
                        printf("获取挂载点消冗信息成功!\n");
                }
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END\n");
        }

        //::6. 删除挂载点数据
        pack.PkHead.Type = CMD_MP_DELETE;
        pack.JsonValue["path"] = "/home/wb/MountPoint/mp";
        if (pm.Send(pack)) {
                printf("%d Send CMD_MP_DELETE!\n", ++cnt);
        }

        pm.Recv(pack);
        if (pack.PkHead.Type == CMD_OK) {
                printf("删除挂载点数据成功!\n");
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("Send CMD_END\n");
        }
        */
                return 0;
}

int StorageGW(int sockfd)
{
        int cnt = 0;

        Json::Value value;
        Pack pack;
        PackMgr pm(sockfd);
        //--
        pack.PkHead.Type = CMD_DISK_DRIVER;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DISK_DRIVER!\n", ++cnt);
        }

        if (pm.Recv(pack)) {
                cout << pack.JsonValue.toStyledString();
                Json::Value array;
                Json::Value key;
                Json::Value driver;
                array = pack.JsonValue["diskdriver"];

                //cout << array.toStyledString() << endl;

                for (unsigned int i = 0; i<array.size(); ++i) {
                        cout << "driver=" << array[i]["driver"].asString()      << endl
                             << "commpany=" << array[i]["commpany"].asString()  << endl
                             //<< "totalSize=" << array[i]["size"].asDouble()      << "G" << endl;
                             << "totalSize=" << array[i]["size"].asString()     << endl;

                        key = array[i]["partition"];
                        //cout << array[i].size() << endl;

                        for (unsigned int j=0; j<key.size(); ++j) {
                                cout << "dev=" << key[j]["dev"].asString()       << endl
                                     << "fstype=" << key[j]["fstype"].asString() << endl
                                     //<< "devSize=" << key[j]["size"].asDouble()   << endl
                                     << "devSize=" << key[j]["size"].asString()  << endl
                                     << "mp=" << key[j]["mp"].asString()         << endl;
                        }
                        cout << endl;
                }
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        /*
        //--
        pack.PkHead.Type = CMD_NETCARD_INFO;
        if (pm.Send(pack)) {
                printf("%d Send CMD_NETCARD!\n", ++cnt);
        }
        
        Json::Value eth;
        Json::Value bound;
        if (pm.Recv(pack)) {
                if (pack.PkHead.Type == CMD_ERR) {
                        cout << pack.JsonValue["info"] << endl;
                }
                
                eth   = pack.JsonValue["eth"];
                bound = pack.JsonValue["bound"];

                for (unsigned int i=0; i<eth.size(); ++i) {
                        cout << eth[i]["ethx"].asString()              << endl
                             << eth[i]["company"].asString()           << endl
                             << eth[i]["chip"].asString()              << endl
                             << eth[i]["transmissionrate"].asString()  << endl
                             << eth[i]["mac"].asString()               << endl
                             << eth[i]["ip"].asString()                << endl
                             << eth[i]["netsmask"].asString()          << endl
                             << eth[i]["gateway"].asString()           << endl
                             << eth[i]["alias"].asString()             << endl;
                }

                for (unsigned int i=0; i<bound.size(); ++i) {
                        Json::Value e = bound[i];
                        Json::Value f = e["eth"];
                        cout << "-----bound-----" << endl;

                        cout << e["boundx"].asString()          << endl
                             << e["alias"].asString()           << endl;

                        for (unsigned int j =0; j<f.size(); ++j) {
                                cout << f[j]["ethx"].asString() << endl
                                     << f[j]["company"]
                                     << f[j]["chip"]  
                                     << f[j]["transmissionrate"] 
                                     << f[j]["mac"]     
                                     << f[j]["ip"]      
                                     << f[j]["netsmask"]
                                     << f[j]["gateway"]
                                     << f[j]["alias"];
                        }
                }
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        */

        /*
        //--
        pack.PkHead.Type = CMD_DEV_MOUNT;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DEV_INFO!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DEV_UMOUNT;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DEV_INFO!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DEV_DELETE;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DEV_INFO!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_FROMAT_DEV;
        if (pm.Send(pack)) {
                printf("%d Send CMD_FROMAT_DEV!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_CREATE_PARTITION_DEV;
        if (pm.Send(pack)) {
                printf("%d Send CMD_CREATE_PARTITION_DEV!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }


        //--
        pack.PkHead.Type = CMD_NETPORT_CONFIG;
        if (pm.Send(pack)) {
                printf("%d Send CMD_NETPORT!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_NETCARD_TRUNK;
        if (pm.Send(pack)) {
                printf("%d Send CMD_IP_TRUNK!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_NETCARD_UNTRUNK;
        if (pm.Send(pack)) {
                printf("%d Send CMD_IP_TRUNK!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DDFS_START;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DDFS_START!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DDFS_STOP;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DDFS_STOP!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DDFS_RESTART;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DDFS_RESTART!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DDFS_FORCE_STOP;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DDFS_FORCE_STOP!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_DDFS_FORCE_RESTART;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DDFS_FORCE_RESTART!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        */

        /*
        //--
        pack.PkHead.Type = CMD_GWENGINE_STOP;
        if (pm.Send(pack)) {
                printf("%d Send CMD_GWENGINE_STOP!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        */

        /*
        //--
        pack.PkHead.Type = CMD_GWENGINE_RESTART;
        if (pm.Send(pack)) {
                printf("%d Send CMD_GWENGINE_RESTART!\n", ++cnt);
        }
        */

        /*
        //--
        pack.PkHead.Type = CMD_DATA_ARCHIVE;
        if (pm.Send(pack)) {
                printf("%d Send CMD_DATA_ARCHIVE!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_MP_MODIFY;
        if (pm.Send(pack)) {
                printf("%d Send CMD_MP_CONFIG!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        */
        return 0;
}

int GWBaseInfo(int sockfd)
{
        int cnt = 0;

        Json::Value value;
        Pack pack;
        PackMgr pm(sockfd);
        //--
        pack.PkHead.Type = CMD_NETINFO_CONFIG;
        if (pm.Send(pack)) {
                printf("%d Send CMD_NETINFO_CONFIG!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        /*
        //--
        pack.PkHead.Type = CMD_LOG_INFO;
        if (pm.Send(pack)) {
                printf("%d Send CMD_LOG_INFO!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_LOG_WAR;
        if (pm.Send(pack)) {
                printf("%d Send CMD_LOG_WAR!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_LOG_ERR;
        if (pm.Send(pack)) {
                printf("%d Send CMD_LOG_ERR!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_EMAIL_CONFIG;
        if (pm.Send(pack)) {
                printf("%d Send EMAIL_CONFIG!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        */
        return 0;
}

int GWStatusInfo(int sockfd)
{
        int cnt = 0;

        Json::Value value;
        Pack pack;
        PackMgr pm(sockfd);
        //--
        pack.PkHead.Type = CMD_GATEWAY_RESOURCE;
        if (pm.Send(pack)) {
                printf("%d Send CMD_GATEWAY_INFO!\n", ++cnt);
        }

        if (pm.Recv(pack)) {
                if (pack.PkHead.Type == CMD_ERR) {
                        cout << pack.JsonValue["info"].asString() << endl;
                }
                cout << pack.JsonValue["cpunum"].asInt() << endl
                     << pack.JsonValue["totalmem"].asString() << endl
                     << pack.JsonValue["usemem"].asString() << endl
                     << pack.JsonValue["cpu"].asString() << endl;
        }

        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //----
        /*
        Pack mp;
        mp.PkHead.Type = CMD_MP_RESOURCE;
        mp.JsonValue["path"] = "/home/wb/MountPoint/mp";
        if (pm.Send(mp)) {
                printf("%d Send CMD_GATEWAY_INFO!\n", ++cnt);
        }

        if(pm.Recv(mp)) {
                if (pack.JsonValue["info"] == CMD_ERR) {
                        printf("%s\n", pack.JsonValue["info"].asString().c_str());
                }
        }

        mp.PkHead.Type = CMD_END;
        if (pm.Send(mp)) {
                printf("CMD_END!\n");
        }
        */

        /*
        //--
        pack.PkHead.Type = CMD_AUTORIZATION;
        if (pm.Send(pack)) {
                printf("%d Send CMD_AUTORIZATION!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        */
        return 0;
}

int LogMgr(int sockfd)
{
        int cnt = 0;

        Json::Value value;
        Pack pack;
        PackMgr pm(sockfd);
        //--
        pack.PkHead.Type = CMD_SYSTEM_WAR;
        if (pm.Send(pack)) {
                printf("%d Send CMD_SYSTEM_WAR!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }

        //--
        pack.PkHead.Type = CMD_SYSTEM_LOG;
        if (pm.Send(pack)) {
                printf("%d Send CMD_SYSTEM_LOG!\n", ++cnt);
        }
        pack.PkHead.Type = CMD_END;
        if (pm.Send(pack)) {
                printf("CMD_END!\n");
        }
        return 0;
}

int GWOpr(int sockfd)
{
        Json::Value value;
        Pack pack;
        PackMgr pm(sockfd);
        //--
        pack.PkHead.Type = CMD_GW_RESTART;
        if (pm.Send(pack)) {
                printf("Send GW_RESTART!\n");
        }
        return 0;
}

int NFSOpr (int sockfs)
{
        Pack share1;
        Pack share2;
        Pack pk;
        Pack pack;
        pk.PkHead.Type = CMD_NFS_LOOKUP;

        /*
        pk.PkHead.Type = CMD_NFS_SAVE;
        share1.JsonValue["shareddir"] = "/tmp/a/b";
        share1.JsonValue["sharedip"]   = "192.168.0.*";

        share2.JsonValue["shareddir"] = "/tmp/a/c";
        share2.JsonValue["sharedip"]   = "192.168.1.*";

        pk.JsonValue["NfsCfg"].append(share1.JsonValue);
        pk.JsonValue["NfsCfg"].append(share2.JsonValue);
        */

        PackMgr pm(sockfs);
        if (pm.Send(pk)) {
                printf("send CMD_NFS_SAVE\n");
        }

/*
        PackMgr pm(sockfs);
        if (pm.Send(pk)) {
                printf("send CMD_NFS_SAVE\n");
        }
*/
        if (pm.Recv(pack)) {
                cout << pack.JsonValue.toStyledString() << endl;
        }
        //for (size_t i = 0; i<array.size(); ++i) {

        Pack end;
        end.PkHead.Type = CMD_END;
        if (pm.Send(end)) {
                printf("send CMD_END!\n");
        }

        return 0;
}

int MultiPathOpr(int sockfs)
{
        Pack start;
        Pack stop;
        start.PkHead.Type = CMD_MULTIPATH_START;
        stop.PkHead.Type = CMD_MULTIPATH_STOP;

        PackMgr pm(sockfs);

/*
        if (pm.Send(start)) {
                printf("send CMD_MULTIPATH_START\n");
        }
*/
        if (pm.Send(stop)) {
                printf("send CMD_MULTIPATH_STOP\n");
        }
        Pack end;
        end.PkHead.Type = CMD_END;
        if (pm.Send(end)) {
                printf("send CMD_END!\n");
        }

        return 0;
}

int SSDOpr(int sockfs)
{
        Pack pk;
        Pack end;
        end.PkHead.Type = CMD_END;
        
        /*
        //create
        pk.PkHead.Type = CMD_SSDCACHE_CREATE;
        pk.JsonValue["SsdDev"] = "/dev/sdb1";
        pk.JsonValue["SasDev"] = "/dev/sdc1";
        */ 
        
        //mount
        /*
        pk.PkHead.Type = CMD_SSDCACHE_LOAD;
        pk.JsonValue["MountPoint"] = "/tmp/a";
        pk.JsonValue["CacheDev"]   = "/dev/mapper/cachedev";
        */
        

        /* 
        //umount
        pk.PkHead.Type = CMD_SSDCACHE_UNLOAD;
        pk.JsonValue["CacheDev"] = "/dev/mapper/cachedev";
        */

        //lookup
        pk.PkHead.Type = CMD_SSDCACHE_LOOKUP;
        
        PackMgr pm(sockfs);
        if (!pm.Send(pk)) {
                printf("send CMD_SSDCACHE_CREATE error\n"); 
        }

        pk.JsonValue.clear();
        if (!pm.Recv(pk)) {
                printf("recv CMD_SSDCACHE_CREATE error\n"); 
        }
        cout << pk.JsonValue.toStyledString() << endl;

        if (!pm.Send(end)) {
                printf("send CMD_END error\n"); 
        }
        return 0;
}

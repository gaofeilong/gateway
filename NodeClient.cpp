#include "Utils/Log/Log.h"
#include "Utils/CommonOpr/Lock.h"
#include "GatewayBG/Socket/GateWayMgr.h"

int main(int argc, char* argv[])
{
        int ret, port;

        /* 参数检查 add by gfl: 2012-12-1 */
        if (argc == 2 && ((port = atoi(argv[1])) >= 4096)) {
                /* 两个参数，并且后一个是有效端口号，daemon运行 */
                daemon(1, 0);
        } else if (argc == 3 && ((port = atoi(argv[1])) >= 4096) && 
                                        strcmp(argv[2], "-c") == 0) {
                /* 三个参数，最后一个是 -c 则不已daemon 运行 */
        } else {
                printf("argv[0]=%s\n", argv[0]);
                printf("argv[1]=端口号\n");
                return -1;
        }

        Lock lockFile;
        if (lockFile.LockFile("/var/run/gwengine_client.pid") == LOCK_FAIL) {
                LOG_INFO("GWEngine AlreadyRunning!");
                printf("GWEngine AlreadyRunning!\n");
                exit(1);
        }

        FILE* file = fopen("/var/run/NodeClient.pid", "w+");
        if (file == NULL) {
                return 1;
        }
        fprintf(file, "%d", getpid());
        fclose(file);

        GateWayMgr gate(port);
        ret = gate.Init();
        if ( ret != 0 ) {
                LOG_INFO("gate.Init error! ret=" << ret);
                return ret;
        }
        LOG_INFO("Listen beginning...");
        gate.Listen();
}

#include "RTSPServer.h"

#include <stdio.h>
#include <string.h>

void usage() {
    printf("KimchiMediaServer ip port ( -m mip mport )\n");
    printf("ip port : RTSP server ip and port\n");
    printf("-m mip mport : ip and port of master node\n");
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc < 3) {
        usage();
        return 0;
    }
    RTSPServer rtspServer(argv[1], atoi(argv[2]));

    if (argc == 6) {
        if (strcmp(argv[3], "-m") == 0) {
            rtspServer.registerMaster(argv[4], argv[5]);
        } else {
            usage();
            return 0;
        }
    }
    if (rtspServer.startListen())
        rtspServer.acceptLoop();
    return 0;
}

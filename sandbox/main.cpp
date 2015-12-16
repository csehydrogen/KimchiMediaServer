#include "RTSPServer.h"

#include <stdio.h>
#include <string.h>

void usage() {
    printf("KimchiMediaServer port ( -m mip mport )\n");
    printf("port : RTSP server port\n");
    printf("-m mip mport : ip and port of master node\n");
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc < 2) {
        usage();
        return 0;
    }
    RTSPServer rtspServer(atoi(argv[1]));

    if (argc == 5) {
        if (strcmp(argv[2], "-m") == 0) {
            rtspServer.registerMaster(argv[3], argv[4]);
        } else {
            usage();
            return 0;
        }
    }
    if (rtspServer.startListen())
        rtspServer.acceptLoop();
    return 0;
}

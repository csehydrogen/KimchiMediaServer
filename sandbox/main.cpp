#include "RTSPServer.h"

int main(int argc, char *argv[]) {
    RTSPServer rtspServer;
    if (rtspServer.startListen())
        rtspServer.acceptLoop();
    return 0;
}

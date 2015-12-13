#include "RTSPServer.h"

int main(int argc, char *argv[]) {
    srand(time(NULL));
    RTSPServer rtspServer;
    if (rtspServer.startListen())
        rtspServer.acceptLoop();
    return 0;
}

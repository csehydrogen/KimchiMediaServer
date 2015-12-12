#pragma once

class RTSPServer {
    int port, backlog, listenfd;
public:
    RTSPServer(int _port = 8554, int _backlog = 5);
    ~RTSPServer();
    bool startListen();
    void acceptLoop();
    static void* parseLoop(void *connfdp);
};

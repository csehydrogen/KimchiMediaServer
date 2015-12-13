#pragma once

class RTSPServer {
    int port, backlog, listenfd, nextRTPport;
public:
    RTSPServer(int _port = 8554, int _backlog = 5, int _nextRTPport = 8556);
    ~RTSPServer();
    void setNextRTPport(int _nextRTPport);
    int getNextRTPport();
    bool startListen();
    void acceptLoop();
    static void* parseLoop(void *connfdp);
};

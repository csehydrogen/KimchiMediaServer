#pragma once

#include "RTSPSession.h"

#include <map>

class RTSPServer {
    int port, backlog, listenfd, nextRTPport;
    std::map<std::string, RTSPSession*> sessions;
public:
    RTSPServer(int _port = 8554, int _backlog = 5, int _nextRTPport = 8556);
    ~RTSPServer();
    void setNextRTPport(int _nextRTPport);
    int getNextRTPport();
    void addSession(char const *key, RTSPSession *s);
    RTSPSession* getSession(std::string key);
    std::map<std::string, RTSPSession*>& getSessions();
    bool startListen();
    void acceptLoop();
    static void* parseLoop(void *arg);
    static void* sendLoop(void *arg);
};

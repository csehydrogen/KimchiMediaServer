#pragma once

#include "RTSPSession.h"
#include "Slave.h"

#include <map>

class RTSPServer {
    int port, backlog, listenfd, nextRTPport;
    std::map<std::string, RTSPSession*> sessions;
    int mfd, sfd;
    std::map<int, Slave*> slaves;
public:
    RTSPServer(int _port = 8554, int _backlog = 5);
    ~RTSPServer();
    void setNextRTPport(int _nextRTPport);
    int getNextRTPport();
    int getMfd();
    int getSfd();
    std::map<int, Slave*>& getSlaves();
    void addSession(char const *key, RTSPSession *s);
    void addSlave(int key, Slave *val);
    void delSlave(int key);
    RTSPSession* getSession(std::string key);
    std::map<std::string, RTSPSession*>& getSessions();
    bool startListen();
    void acceptLoop();
    static void* parseLoop(void *arg);
    static void* sendLoop(void *arg);
    static void* clientAcceptLoop(void *arg);
    static void* clientLoop(void *arg);
    static void* sendDataLoop(void *arg);
    void registerMaster(char const *ip, char const *port);
};

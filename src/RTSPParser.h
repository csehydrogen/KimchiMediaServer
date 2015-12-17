#pragma once

#include "RTSPRequest.h"
#include "BufferedReader.h"
#include "RTSPServer.h"

class RTSPParser {
    RTSPServer *server;
    int connfd;
    BufferedReader *br;
    char clientIP[16];
public:
    RTSPParser(RTSPServer *_server);
    ~RTSPParser();
    RTSPServer* getServer();
    char const * getClientIP();
    bool acceptClient(int listenfd);
    RTSPRequest* parse();
    void write(void const *str, size_t len);
};

#pragma once

#include "RTSPRequest.h"
#include "BufferedReader.h"
#include "RTSPServer.h"

class RTSPParser {
    RTSPServer *server;
    int connfd;
    BufferedReader *br;
public:
    RTSPParser(RTSPServer *_server);
    ~RTSPParser();
    RTSPServer* getServer();
    bool acceptClient(int listenfd);
    RTSPRequest* parse();
    void write(void const *str, size_t len);
};

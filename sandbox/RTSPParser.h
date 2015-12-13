#pragma once

#include "RTSPRequest.h"
#include "BufferedReader.h"

class RTSPParser {
    int connfd;
    BufferedReader *br;
public:
    RTSPParser();
    ~RTSPParser();
    bool acceptClient(int listenfd);
    RTSPRequest* parse();
};

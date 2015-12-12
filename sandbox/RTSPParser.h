#pragma once

#include "RTSPRequest.h"

#include <unistd.h>
#include <stdlib.h>

class RTSPParser {
    int connfd;
    static int const BUFSIZE = 4096;
    char buf[BUFSIZE], *bufnext;
    int bufremain;
public:
    RTSPParser();
    ~RTSPParser();
    bool acceptClient(int listenfd);
    ssize_t bufread(char *usrbuf, size_t n);
    ssize_t bufreadline(char *usrbuf, size_t maxlen);
    RTSPRequest* parse();
};

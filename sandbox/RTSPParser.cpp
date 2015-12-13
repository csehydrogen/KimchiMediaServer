#include "RTSPParser.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

RTSPParser::RTSPParser() : connfd(-1), br(NULL) {}

RTSPParser::~RTSPParser() {
    close(connfd);
    delete br;
}

bool RTSPParser::acceptClient(int listenfd) {
    sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    connfd = accept(listenfd, (sockaddr*)&cli_addr, &cli_addr_len);
    if (connfd < 0) {
        perror("ERROR on accept");
        return false;
    }

    br = new BufferedReader(connfd);

    return true;
}

RTSPRequest* RTSPParser::parse() {
    int n;
    char line[1024], *ptr, *saveptr;
    RTSPRequest *rtspRequest = new RTSPRequest();

    n = br->readline(line, sizeof(line));
    ptr = strtok_r(line, " ", &saveptr);
    rtspRequest->setMethod(ptr);
    ptr = strtok_r(NULL, " ", &saveptr);
    rtspRequest->setURL(ptr);
    ptr = strtok_r(NULL, "\r", &saveptr);
    rtspRequest->setVersion(ptr);

    while (true) {
        n = br->readline(line, sizeof(line));
        if (n <= 2) break; // TODO 주의 body가 존재할 수 있음
        char *key = strtok_r(line, ":", &saveptr);
        char *val = strtok_r(NULL, " \r", &saveptr);
        rtspRequest->addHeaders(key, val);
    }

    return rtspRequest;
}

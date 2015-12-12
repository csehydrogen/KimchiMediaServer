#include "RTSPParser.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

RTSPParser::RTSPParser() : connfd(-1), bufnext(buf), bufremain(0) {}

RTSPParser::~RTSPParser() {
    close(connfd);
}

bool RTSPParser::acceptClient(int listenfd) {
    sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    connfd = accept(listenfd, (sockaddr*)&cli_addr, &cli_addr_len);
    if (connfd < 0) {
        perror("ERROR on accept");
        return false;
    }

    return true;
}

ssize_t RTSPParser::bufread(char *usrbuf, size_t n) {
    while (bufremain <= 0) {
        bufremain = read(connfd, buf, sizeof(buf));
        if (bufremain < 0) {
            if (errno != EINTR)
                return -1;
        } else if (bufremain == 0) {
            return 0;
        } else {
            bufnext = buf;
        }
    }

    if (n > bufremain)
        n = bufremain;
    memcpy(usrbuf, bufnext, n);
    bufnext += n;
    bufremain -= n;
    return n;
}

ssize_t RTSPParser::bufreadline(char *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *cur = usrbuf;
    for (n = 1; n < maxlen; ++n) {
        if ((rc = bufread(&c, 1)) == 1) {
            *cur++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        } else {
            return -1;
        }
    }
    *cur = 0;
    return n;
}

RTSPRequest* RTSPParser::parse() {
    int n;
    char line[1024], *ptr, *saveptr;
    RTSPRequest *rtspRequest = new RTSPRequest();

    n = bufreadline(line, sizeof(line));
    ptr = strtok_r(line, " ", &saveptr);
    rtspRequest->setMethod(ptr);
    ptr = strtok_r(NULL, " ", &saveptr);
    rtspRequest->setURL(ptr);
    ptr = strtok_r(NULL, "\r", &saveptr);
    rtspRequest->setVersion(ptr);

    while (true) {
        n = bufreadline(line, sizeof(line));
        if (n <= 2) break; // TODO 주의 body가 존재할 수 있음
        char *key = strtok_r(line, ":", &saveptr);
        char *val = strtok_r(NULL, " \r", &saveptr);
        rtspRequest->addHeaders(key, val);
    }

    return rtspRequest;
}

#include "RTSPSession.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

RTSPSession::RTSPSession() : rtpfd(-1), rtcpfd(-1), port(0) {}

RTSPSession::~RTSPSession() {
    clear();
}

int RTSPSession::getPort() {
    return port;
}

char* RTSPSession::getKey() {
    return key;
}

void RTSPSession::setPort(int _port) {
    port = _port;
}

void RTSPSession::clear() {
    if (rtpfd != -1) {
        close(rtpfd);
        rtpfd = -1;
    }

    if (rtcpfd != -1) {
        close(rtcpfd);
        rtcpfd = -1;
    }

    port = 0;
}

bool RTSPSession::setup(int port) {
    if ((rtpfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("ERROR on RTP socket creation");
        clear();
        return false;
    }

    if ((rtcpfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("ERROR on RTCP socket creation");
        clear();
        return false;
    }

    while (true) {
        sockaddr_in rtpsa;
        rtpsa.sin_family = AF_INET;
        rtpsa.sin_addr.s_addr = INADDR_ANY;
        rtpsa.sin_port = htons(port);
        if (bind(rtpfd, (sockaddr*)&rtpsa, sizeof(rtpsa)) < 0) {
            if (errno != EADDRINUSE) {
                perror("ERROR on RTP socket bind");
                clear();
                return false;
            }
            port += 2;
            continue;
        }

        sockaddr_in rtcpsa;
        rtcpsa.sin_family = AF_INET;
        rtcpsa.sin_addr.s_addr = INADDR_ANY;
        rtcpsa.sin_port = htons(port + 1);
        if (bind(rtcpfd, (sockaddr*)&rtcpsa, sizeof(rtcpsa)) < 0) {
            if (errno != EADDRINUSE) {
                perror("ERROR on RTCP socket bind");
                clear();
                return false;
            }
            port += 2;
            continue;
        }

        break;
    }
    this->port = port;
    return true;
}

void RTSPSession::generateKey() {
    srand(time(NULL));
    for (int i = 0; i < KEYLEN; ++i)
        key[i] = 'A' + rand() % 26;
    key[KEYLEN] = '\0';
}

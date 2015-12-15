#include "RTSPSession.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

RTSPSession::RTSPSession() : rtpfd(-1), rtcpfd(-1), port(0), seqnum(0), timestamp(0), isPlaying(false) {}

RTSPSession::~RTSPSession() {
    clear();
}

int RTSPSession::getPort() {
    return port;
}

char* RTSPSession::getKey() {
    return key;
}

int RTSPSession::getSeqnum() {
    return seqnum;
}

int RTSPSession::getTimestamp() {
    return timestamp;
}

void RTSPSession::setPort(int _port) {
    port = _port;
}

void RTSPSession::setClientAddr(char const *ip, int port) {
    sa_cli.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &sa_cli.sin_addr);
    sa_cli.sin_port = htons(port);
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
    for (int i = 0; i < KEYLEN; ++i)
        key[i] = 'A' + rand() % 26;
    key[KEYLEN] = '\0';
    ssrc = rand();
}

bool RTSPSession::setTS(char const *fp) {
    if (ts.open(fp)) {
        ts.parsetsx();
        return true;
    }
    return false;
}

void RTSPSession::seek(double startNptTime) {
    ts.seekByNpt(startNptTime);
}

void RTSPSession::setPlay() {
    isPlaying = true;
}

void RTSPSession::setPause() {
    isPlaying = false;
}

void RTSPSession::play() {
    if (!isPlaying) return;
    unsigned char p[12 + 188 * 7];
    p[0] = 0x80;
    p[1] = 0x21;
    *(unsigned short*)(p + 2) = htons(seqnum);
    *(unsigned int*)(p + 4) = htonl(timestamp);
    *(unsigned int*)(p + 8) = htonl(ssrc);
    int i;
    for (i = 0; i < 7; ++i) {
        int res = ts.getFrame(p + 12 + 188 * i);
        if (res == -1) {
            isPlaying = false;
            break;
        } else if (res == -2) {
            break;
        } else {
            timestamp += res;
        }
    }
    if (i > 0) {
        if (sendto(rtpfd, p, 12 + 188 * i, 0, (sockaddr*)&sa_cli, sizeof(sa_cli)) == -1) {
            perror("ERROR on sendto");
            return;
        }
        ++seqnum;
    }
}

double RTSPSession::getNpt() {
    return ts.getNpt();
}

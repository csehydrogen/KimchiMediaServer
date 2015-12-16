#pragma once

#include "MPEG2TS.h"

#include <netinet/in.h>

class RTSPSession {
    int rtpfd, rtcpfd, port, seqnum, timestamp, ssrc;
    bool isPlaying, isTeardown;
    static int const KEYLEN = 8;
    char key[KEYLEN + 1];
    MPEG2TS ts;
    sockaddr_in sa_cli;
public:
    RTSPSession();
    ~RTSPSession();
    int getPort();
    char* getKey();
    int getSeqnum();
    int getTimestamp();
    void setPort(int _port);
    void setClientAddr(char const *ip, int port);
    void clear();
    bool setup(int nextRTPport);
    void generateKey();
    bool setTS(char const *fp);
    void seek(double startNptTime);
    void setPlay();
    void setPause();
    void setTeardown();
    bool getTeardown();
    void play();
    double getNpt();
};

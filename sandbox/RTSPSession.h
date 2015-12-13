#pragma once

class RTSPSession {
    int rtpfd, rtcpfd, port;
    static int const KEYLEN = 8;
    char key[KEYLEN + 1];
public:
    RTSPSession();
    ~RTSPSession();
    int getPort();
    char* getKey();
    void setPort(int _port);
    void clear();
    bool setup(int nextRTPport);
    void generateKey();
};

#pragma once

#include "BufferedReader.h"

#include <algorithm>

class Slave {
    int sfd;
    BufferedReader br;
    double a, b, c, d;
    double BW, RAM, CPU;
    char ip[16]; int port; char RTSPip[16]; int RTSPport;
    bool disconnected;
public:
    Slave(int _sfd);
    ~Slave();
    int getSfd();
    char const *getIP();
    char const *getRTSPip();
    int getPort();
    int getRTSPport();
    bool isDisconnected();
    void disconnect();
    void setIPandPort(char const *_ip, int _port);
    BufferedReader& getBr();
    double compute(bool allocated = true, double size = 0.0, double length = 0.0);
    bool readAndUpdate(bool init);
};

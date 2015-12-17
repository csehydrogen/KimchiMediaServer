#include "Slave.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

Slave::Slave(int _sfd) : sfd(_sfd), br(_sfd), a(1), b(1), c(1), d(1),
    disconnected(false) {}

Slave::~Slave() {
    close(sfd);
}

int Slave::getSfd() {
    return sfd;
}

char const * Slave::getIP() {
    return ip;
}

char const * Slave::getRTSPip() {
    return RTSPip;
}

int Slave::getPort() {
    return port;
}

int Slave::getRTSPport() {
    return RTSPport;
}

BufferedReader& Slave::getBr() {
    return br;
}

bool Slave::isDisconnected() {
    return disconnected;
}

void Slave::disconnect() {
    disconnected = true;
}

void Slave::setIPandPort(char const *_ip, int _port) {
    memcpy(ip, _ip, sizeof(ip));
    port = _port;
}

double Slave::compute(bool allocated, double size, double length) {
    double BW2 = 1 - BW;
    double RAM2 = 1 - RAM;
    double CPU2 = 1 - CPU;
    if (allocated) {
        BW2 -= size / length / 1000000000;
        RAM2 -= size / totalMem / 1000;
        CPU2 -= a * CPU + b * RAM + c * size / length / 1000000000 + d;
    }
    BW2 *= BW2;
    RAM2 *= RAM2;
    CPU2 *= CPU2;
    return std::max(BW2, std::max(RAM2, CPU2));
}

bool Slave::readAndUpdate(bool init) {
    int type;
    if (br.readn((char*)&type, sizeof(type)) == 0) {
        return false;
    }
    if (type == 0) {
        char buf[24];
        br.readn(buf, sizeof(buf));
        double BWn = *(double*)(buf + 0);
        double RAMn = *(double*)(buf + 8);
        double CPUn = *(double*)(buf + 16);

        if (!init) {
            static double eta = 0.01;
            double f = CPUn;
            double x = CPU;
            double y = RAM;
            double z = BWn - BW;
            double da = eta * (f - a * x - b * y - c * z - d) * x;
            double db = eta * (f - a * x - b * y - c * z - d) * y;
            double dc = eta * (f - a * x - b * y - c * z - d) * z;
            double dd = eta * (f - a * x - b * y - c * z - d) * 1;
            a += da;
            b += db;
            c += dc;
            d += dd;
        }

        BW = BWn;
        RAM = RAMn;
        CPU = CPUn;
    } else if (type == 1) {
        int iip;
        br.readn((char*)&iip, sizeof(iip));
        inet_ntop(AF_INET, &iip, RTSPip, sizeof(RTSPip));
        br.readn((char*)&RTSPport, sizeof(RTSPport));
        br.readn((char*)&totalMem, sizeof(totalMem));
        printf("from %s:%d, RTSP %s:%d\n", ip, port, RTSPip, RTSPport);
    } else {
        printf("from %s:%d, Unknown type %d\n", ip, port, type);
    }

    return true;
}

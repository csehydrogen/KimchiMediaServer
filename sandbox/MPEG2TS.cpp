#include "MPEG2TS.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

MPEG2TS::MPEG2TS() : tsfd(-1), tsxfd(-1), tsbr(NULL), tsxbr(NULL), duration(0.0),
    iframe(), curtpn(0) {}

MPEG2TS::~MPEG2TS() {
    delete tsbr;
    delete tsxbr;
    close(tsfd);
    close(tsxfd);
}

double MPEG2TS::getDuration() {
    return duration;
}

bool MPEG2TS::open(char const *fp) {
    char buf[256];
    strcpy(buf, fp);
    if((tsfd = ::open(buf, O_RDONLY)) < 0) {
        perror("ERROR on opening ts");
        return false;
    }

    int n = strlen(buf);
    buf[n] = 'x';
    buf[n + 1] = '\0';
    if((tsxfd = ::open(buf, O_RDONLY)) < 0) {
        perror("ERROR on opening tsx");
        close(tsfd);
        return false;
    }

    tsbr = new BufferedReader(tsfd);
    tsxbr = new BufferedReader(tsxfd);

    return true;
}

void MPEG2TS::parsetsx() {
    unsigned char buf[11];
    double pcr;

    pcrs.clear();
    while (true) {
        if (tsxbr->readn((char*)buf, 11) == 0) break;
        // 9 : H264_IFRAME
        // F : H265_IFRAME
        unsigned char rt = buf[0];
        unsigned pcr_int
            = (unsigned)buf[3] + ((unsigned)buf[4] << 8) + ((unsigned)buf[5] << 16);
        unsigned pcr_frac = buf[6];
        pcr = pcr_int + pcr_frac / 256.0;
        unsigned tpn = *(unsigned*)(&buf[7]);
        if (rt == 0x89 || rt == 0x8F) {
            iframe[pcr] = tpn;
        }
        while (pcrs.size() <= tpn) {
            pcrs.push_back(pcr);
        }
    }
    iframe[0.0] = 0;
    duration = pcr;
}

bool MPEG2TS::seekByNpt(double npt) {
    if (npt < 0) {
        if (curtpn >= pcrs.size())
            return false;
        npt = pcrs[curtpn];
    } else {
        auto it = iframe.lower_bound(npt);
        if (it != iframe.begin())
            --it;
        npt = it->first;
        curtpn = it->second;
        int res = tsbr->seek(188 * curtpn, SEEK_SET);
    }

    gettimeofday(&startTime, NULL);
    startTime.tv_sec -= (int)npt;
    startTime.tv_usec -= (npt - (int)npt) * 1000000;
    if (startTime.tv_usec < 0) {
        startTime.tv_usec += 1000000;
        startTime.tv_sec -= 1;
    }

    return true;
}

int MPEG2TS::getFrame(unsigned char *buf) {
    if (pcrs.size() <= curtpn) {
        curtpn = 0;
        return -1;
    }
    double npt = pcrs[curtpn];
    timeval playTime;
    playTime.tv_sec = startTime.tv_sec + (int)npt;
    playTime.tv_usec = startTime.tv_usec + (npt - (int)npt) * 1000000;
    if (playTime.tv_usec >= 1000000) {
        playTime.tv_usec -= 1000000;
        playTime.tv_sec += 1;
    }

    timeval curTime;
    gettimeofday(&curTime, NULL);
    if (curTime.tv_sec < playTime.tv_sec
        || (curTime.tv_sec == playTime.tv_sec && curTime.tv_usec < playTime.tv_usec)) {
        return -2;
    }

    ++curtpn;
    int res = tsbr->readn((char*)buf, 188);
    if (res != 188) {
        printf("wrong!!!\n");
    }

    int ret;
    if (pcrs.size() <= curtpn) {
        ret = 0;
    } else {
        ret = (pcrs[curtpn] - pcrs[curtpn - 1]) * 100000;
    }
    return ret;
}

double MPEG2TS::getNpt() {
    return pcrs.size() > curtpn ? pcrs[curtpn] : 0.0;
}

#include "MPEG2TS.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

MPEG2TS::MPEG2TS() : tsfd(-1), tsxfd(-1), tsbr(NULL), tsxbr(NULL), duration(0.0),
    iframe() {}

MPEG2TS::~MPEG2TS() {
    delete tsbr;
    delete tsxbr;
    close(tsfd);
    close(tsxfd);
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
    }
    duration = pcr;
}

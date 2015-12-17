#pragma once

#include "BufferedReader.h"

#include <map>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

class MPEG2TS {
    int tsfd, tsxfd;
    BufferedReader *tsbr, *tsxbr;
    double duration;
    std::map<double, unsigned> iframe;
    std::vector<double> pcrs;
    int curtpn;
    timeval startTime;
public:
    MPEG2TS();
    ~MPEG2TS();
    double getDuration();
    double getSize(); // byte
    bool open(char const *fp);
    void parsetsx();
    bool seekByNpt(double npt);
    int getFrame(unsigned char *buf);
    double getNpt();
};

#pragma once

#include "BufferedReader.h"

#include <map>

class MPEG2TS {
    int tsfd, tsxfd;
    BufferedReader *tsbr, *tsxbr;
    double duration;
    std::map<double, unsigned> iframe;
public:
    MPEG2TS();
    ~MPEG2TS();
    double getDuration();
    bool open(char const *fp);
    void parsetsx();
    void seekByNpt(double npt);
    int getFrame(unsigned char *buf);
};

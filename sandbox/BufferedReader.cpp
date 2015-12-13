#include "BufferedReader.h"

#include <errno.h>
#include <string.h>

BufferedReader::BufferedReader(int _fd, int _sz) : fd(_fd), sz(_sz), remain(0) {
    buf = next = new char[sz];
}

BufferedReader::~BufferedReader() {
    delete[] buf;
}

ssize_t BufferedReader::read(char *usrbuf, size_t n) {
    while (remain <= 0) {
        remain = ::read(fd, buf, sizeof(buf));
        if (remain < 0) {
            if (errno != EINTR)
                return -1;
        } else if (remain == 0) {
            return 0;
        } else {
            next = buf;
        }
    }

    if (n > remain)
        n = remain;
    memcpy(usrbuf, next, n);
    next += n;
    remain -= n;
    return n;
}

ssize_t BufferedReader::readline(char *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *cur = usrbuf;
    for (n = 1; n < maxlen; ++n) {
        if ((rc = read(&c, 1)) == 1) {
            *cur++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        } else {
            return -1;
        }
    }
    *cur = 0;
    return n;
}

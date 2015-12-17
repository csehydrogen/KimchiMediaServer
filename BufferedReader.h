#pragma once

#include <unistd.h>
#include <stdlib.h>

class BufferedReader {
    int sz, remain, fd;
    char *buf, *next;
public:
    BufferedReader(int _fd, int _sz = 4096);
    ~BufferedReader();
    ssize_t read(char *usrbuf, size_t n);
    ssize_t readn(char *usrbuf, size_t n);
    ssize_t readline(char *usrbuf, size_t maxlen);
    off_t seek(off_t offset, int whence);
};

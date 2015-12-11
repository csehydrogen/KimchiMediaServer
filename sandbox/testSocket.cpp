#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

class RTSPParser {
    int connfd;
    static int const BUFSIZE = 4096;
    char buf[BUFSIZE], *bufnext;
    int bufremain;
public:
    RTSPParser();
    ~RTSPParser();
    bool acceptClient(int listenfd);
    ssize_t bufread(char *usrbuf, size_t n);
    ssize_t bufreadline(char *usrbuf, size_t maxlen);
};

RTSPParser::RTSPParser() : connfd(-1), bufnext(buf), bufremain(0) {}

RTSPParser::~RTSPParser() {
    close(connfd);
}

bool RTSPParser::acceptClient(int listenfd) {
    sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    connfd = accept(listenfd, (sockaddr*)&cli_addr, &cli_addr_len);
    if (connfd < 0) {
        perror("ERROR on accept");
        return false;
    }

    return true;
}

ssize_t RTSPParser::bufread(char *usrbuf, size_t n) {
    while (bufremain <= 0) {
        bufremain = read(connfd, buf, sizeof(buf));
        if (bufremain < 0) {
            if (errno != EINTR)
                return -1;
        } else if (bufremain == 0) {
            return 0;
        } else {
            bufnext = buf;
        }
    }

    if (n > bufremain)
        n = bufremain;
    memcpy(usrbuf, bufnext, n);
    bufnext += n;
    bufremain -= n;
    return n;
}

ssize_t RTSPParser::bufreadline(char *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *cur = usrbuf;
    for (n = 1; n < maxlen; ++n) {
        if ((rc = bufread(&c, 1)) == 1) {
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

class RTSPServer {
    int port, backlog, listenfd;
public:
    RTSPServer(int _port = 8554, int _backlog = 5);
    ~RTSPServer();
    bool startListen();
    void acceptLoop();
    static void* parseLoop(void *connfdp);
};

RTSPServer::RTSPServer(int _port, int _backlog)
    : port(_port), backlog(_backlog), listenfd(-1) {}

RTSPServer::~RTSPServer() {
    close(listenfd);
}

bool RTSPServer::startListen() {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("ERROR opening socket");
        return false;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(listenfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on bind");
        close(listenfd);
        return false;
    }

    if (listen(listenfd, backlog)) {
        perror("ERROR on listen");
        close(listenfd);
        return false;
    }

    return true;
}

void RTSPServer::acceptLoop() {
    while (true) {
        RTSPParser *rtspParser = new RTSPParser();
        if (!rtspParser->acceptClient(listenfd))
            return;

        pthread_t thread;
        if (pthread_create(&thread, NULL, parseLoop, rtspParser)) {
            perror("ERROR on pthread_create");
            delete rtspParser;
            return;
        }
    }
}

void* RTSPServer::parseLoop(void *arg) {
    RTSPParser *rtspParser = (RTSPParser*)arg;

    if (pthread_detach(pthread_self())) {
        perror("ERROR on pthread_detach");
        delete rtspParser;
        return NULL;
    }

    // TODO parsing
    char line[4096];
    int n = rtspParser->bufreadline(line, sizeof(line));
    printf("Thread %d\n", pthread_self());
    write(STDOUT_FILENO, line, n);
    printf("\n");

    delete rtspParser;
    return NULL;
}

int main(int argc, char *argv[]) {
    RTSPServer rtspServer;
    if (rtspServer.startListen())
        rtspServer.acceptLoop();
    return 0;
}

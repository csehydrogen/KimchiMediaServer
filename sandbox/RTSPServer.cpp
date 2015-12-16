#include "RTSPServer.h"
#include "RTSPParser.h"
#include "RTSPRequest.h"
#include "RTSPResponse.h"
#include "MPEG2TS.h"

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

RTSPServer::RTSPServer(char const *_ip, int _port, int _backlog)
    : port(_port), backlog(_backlog), listenfd(-1), nextRTPport(_port + 2), mfd(-1) {
        inet_pton(AF_INET, _ip, &ip);
}

RTSPServer::~RTSPServer() {
    close(listenfd);
}

void RTSPServer::setNextRTPport(int _nextRTPport) {
    nextRTPport = _nextRTPport;
}

int RTSPServer::getNextRTPport() {
    return nextRTPport;
}

int RTSPServer::getMfd() {
    return mfd;
}

int RTSPServer::getSfd() {
    return sfd;
}

std::map<int, Slave*>& RTSPServer::getSlaves() {
    return slaves;
}

void RTSPServer::addSession(char const *key, RTSPSession *s) {
    sessions[key] = s;
}

void RTSPServer::addSlave(int key, Slave *val) {
    slaves[key] = val;
}

void RTSPServer::delSlave(int key) {
    slaves.erase(key);
}

RTSPSession* RTSPServer::getSession(std::string key) {
    return sessions[key];
}

std::map<std::string, RTSPSession*>& RTSPServer::getSessions() {
    return sessions;
}

bool RTSPServer::startListen() {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("ERROR opening socket");
        return false;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ip;
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

    char clientIP[16];
    inet_ntop(AF_INET, &serv_addr.sin_addr.s_addr, clientIP, sizeof(clientIP));
    printf("RTSP Server listening at %s:%d\n", clientIP, port);

    if (mfd >= 0) {
        char buf[12];
        *(int*)(buf + 0) = 1;
        *(int*)(buf + 4) = serv_addr.sin_addr.s_addr;
        *(int*)(buf + 8) = port;
        write(mfd, buf, sizeof(buf));
    }

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("ERROR opening socket to slave");
        close(listenfd);
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port + 1);
    if (bind(sfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding to slave socket");
        close(listenfd);
        close(sfd);
        return false;
    }

    if (listen(sfd, backlog)) {
        perror("ERROR on listen on slave socket");
        close(listenfd);
        close(sfd);
        return false;
    }

    printf("Master Server listening at port %d\n", port + 1);

    return true;
}

void RTSPServer::acceptLoop() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, sendLoop, this)) {
        perror("ERROR on pthread_create sendLoop");
        return;
    }
    if (pthread_create(&thread, NULL, clientAcceptLoop, this)) {
        perror("ERROR on pthread_create clientAcceptLoop");
        return;
    }
    while (true) {
        RTSPParser *rtspParser = new RTSPParser(this);
        if (!rtspParser->acceptClient(listenfd))
            return;

        if (pthread_create(&thread, NULL, parseLoop, rtspParser)) {
            perror("ERROR on pthread_create parseLoop");
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

    RTSPServer *rtspServer = rtspParser->getServer();

    while(true) {
        RTSPRequest *rtspRequest = rtspParser->parse();
        if (rtspRequest == NULL)
            break;
        RTSPResponse rtspResponse(rtspRequest);

        std::string method = rtspRequest->getMethod();
        if (method == "OPTIONS") {
            std::string res = rtspResponse.getOPTIONS();
            rtspParser->write(res.c_str(), res.length());
        } else if (method == "DESCRIBE") {
            MPEG2TS v;
            if (v.open(rtspRequest->getFilepath().c_str())) {
                v.parsetsx();
                std::string res = rtspResponse.getDESCRIBE(v.getDuration());
                rtspParser->write(res.c_str(), res.length());
            }
        } else if (method == "SETUP") {
            auto &slaves = rtspServer->getSlaves();
            while (true) {
                bool flag = true;
                for (auto it = slaves.begin(); it != slaves.end(); ++it) {
                    if (it->second->isDisconnected()) {
                        slaves.erase(it);
                        flag = false;
                        break;
                    }
                }
                if (flag) break;
            }

            if (slaves.size() > 0) {
                MPEG2TS v;
                if (v.open(rtspRequest->getFilepath().c_str())) {
                    v.parsetsx();
                    double size = v.getSize();
                    double length = v.getDuration();
                    double ms = 1e300;
                    std::map<int, Slave*>::iterator mi;
                    for (auto i = slaves.begin(); i != slaves.end(); ++i) {
                        double s = 0;
                        for (auto j = slaves.begin(); j != slaves.end(); ++j) {
                            s += j->second->compute(i == j, size, length);
                        }
                        if (ms > s) {
                            ms = s;
                            mi = i;
                        }
                    }

                    std::string res = rtspResponse.getREDIRECT(mi->second->getRTSPip(), mi->second->getRTSPport(), rtspRequest->getFilepath().c_str());
                    rtspParser->write(res.c_str(), res.length());
                    printf("REDIRECT to %s:%d\n", mi->second->getRTSPip(), mi->second->getRTSPport());
                }
            } else {
                RTSPSession *s = new RTSPSession();
                if (s->setup(rtspServer->getNextRTPport())) {
                    rtspServer->setNextRTPport(s->getPort() + 2);
                    s->generateKey();
                    s->setTS(rtspRequest->getFilepath().c_str());
                    s->setClientAddr(rtspParser->getClientIP(), rtspRequest->getClientRTPPort());
                    rtspServer->addSession(s->getKey(), s);

                    std::string res = rtspResponse.getSETUP(
                        rtspParser->getClientIP(),
                        s->getPort(),
                        s->getPort() + 1,
                        s->getKey()
                    );
                    rtspParser->write(res.c_str(), res.length());
                }
            }
        } else if (method == "PLAY") {
            pthread_t thread;
            if (pthread_create(&thread, NULL, sendDataLoop, rtspServer)) {
                perror("ERROR on pthread_create sendDataLoop");
            }

            RTSPSession *s = rtspServer->getSession(rtspRequest->getHeader("Session"));
            double startNptTime = rtspRequest->getStartNptTime();
            s->seek(startNptTime);

            std::string res = rtspResponse.getPLAY(s->getNpt(), s->getSeqnum(), s->getTimestamp());
            rtspParser->write(res.c_str(), res.length());

            s->setPlay();
        } else if (method == "PAUSE") {
            RTSPSession *s = rtspServer->getSession(rtspRequest->getHeader("Session"));

            std::string res = rtspResponse.getPAUSE();
            rtspParser->write(res.c_str(), res.length());

            s->setPause();
        } else if (method == "TEARDOWN") {
            RTSPSession *s = rtspServer->getSession(rtspRequest->getHeader("Session"));

            std::string res = rtspResponse.getTEARDOWN();
            rtspParser->write(res.c_str(), res.length());

            s->setTeardown();
        }

        delete rtspRequest;
    }

    delete rtspParser;

    return NULL;
}

void* RTSPServer::sendLoop(void *arg) {
    if (pthread_detach(pthread_self())) {
        perror("ERROR on pthread_detach");
        return NULL;
    }

    RTSPServer *server = (RTSPServer*)arg;
    auto &sessions = server->getSessions();
    while (true) {
        usleep(1000);
        for (auto it = sessions.begin(); it != sessions.end(); ++it) {
            if (it->second->getTeardown()) {
                delete it->second;
                sessions.erase(it);
                break;
            } else {
                it->second->play();
            }
        }
    }
    return NULL;
}

void* RTSPServer::clientAcceptLoop(void *arg) {
    if (pthread_detach(pthread_self())) {
        perror("ERROR on pthread_detach");
        return NULL;
    }

    RTSPServer *server = (RTSPServer*)arg;
    while (true) {
        sockaddr_in sa_cli;
        socklen_t cli_addr_len = sizeof(sa_cli);
        int connfd = accept(server->sfd, (sockaddr*)&sa_cli, &cli_addr_len);
        if (connfd < 0) {
            perror("ERROR on slave accept");
            return NULL;
        }

        char clientIP[16];
        inet_ntop(AF_INET, &sa_cli.sin_addr.s_addr, clientIP, sizeof(clientIP));
        printf("slave %s:%d connected\n", clientIP, ntohs(sa_cli.sin_port));

        Slave *slave = new Slave(connfd);
        slave->setIPandPort(clientIP, ntohs(sa_cli.sin_port));
        server->addSlave(connfd, slave);

        pthread_t thread;
        if (pthread_create(&thread, NULL, clientLoop, slave)) {
            perror("ERROR on pthread_create clientLoop");
            delete slave;
            server->delSlave(connfd);
            return NULL;
        }
    }

    return NULL;
}

void* RTSPServer::clientLoop(void *arg) {
    if (pthread_detach(pthread_self())) {
        perror("ERROR on pthread_detach");
        return NULL;
    }

    Slave *slave = (Slave*)arg;
    slave->readAndUpdate(true);
    while (slave->readAndUpdate(false));
    slave->disconnect();

    return NULL;
}

void* RTSPServer::sendDataLoop(void *arg) {
    if (pthread_detach(pthread_self())) {
        perror("ERROR on pthread_detach");
        return NULL;
    }

    usleep(1000000);

    RTSPServer *server = (RTSPServer*)arg;
    int mfd = server->getMfd();
    // TODO substitute with real values
    char buf[28];
    *(int*)(buf + 0) = 0;
    *(double*)(buf + 4) = 1000.0;
    *(double*)(buf + 12) = 0.5;
    *(double*)(buf + 20) = 0.5;
    write(mfd, buf, sizeof(buf));

    return NULL;
}

void RTSPServer::registerMaster(char const *ip, char const *port) {
    mfd = socket(AF_INET, SOCK_STREAM, 0);
    if (mfd < 0) {
        perror("ERROR creating socket to master");
        return;
    }

    hostent *he_serv = gethostbyname(ip);
    if (he_serv == NULL) {
        fprintf(stderr, "ERROR no such host : %s\n", ip);
        close(mfd);
        mfd = -1;
        return;
    }

    sockaddr_in sa_serv;
    sa_serv.sin_family = AF_INET;
    memcpy((char*)&sa_serv.sin_addr.s_addr, he_serv->h_addr, he_serv->h_length);
    sa_serv.sin_port = htons(atoi(port));

    if (connect(mfd, (sockaddr*)&sa_serv, sizeof(sa_serv)) < 0) {
        perror("ERROR connecting to master");
        close(mfd);
        mfd = -1;
        return;
    }

    printf("Connected to master %s:%s\n", ip, port);

    // TODO substitute with real values
    char buf[28];
    *(int*)(buf + 0) = 0;
    *(double*)(buf + 4) = 1000.0;
    *(double*)(buf + 12) = 0.5;
    *(double*)(buf + 20) = 0.5;
    write(mfd, buf, sizeof(buf));
}
